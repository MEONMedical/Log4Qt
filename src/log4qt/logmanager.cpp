/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#include "logmanager.h"

#include "consoleappender.h"
#include "helpers/datetime.h"
#include "helpers/initialisationhelper.h"
#include "helpers/optionconverter.h"
#include "hierarchy.h"
#include "jsonconfigurator.h"
#include "propertyconfigurator.h"
#include "xmlconfigurator.h"
#include "ttcclayout.h"
#include "varia/denyallfilter.h"
#include "varia/levelrangefilter.h"
#include "helpers/configuratorhelper.h"

#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QSettings>
#include <QStringList>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStringBuilder>

#include <cstdlib>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

static void qt_message_fatal(QtMsgType, const QMessageLogContext &context, const QString &message);
static bool isFatal(QtMsgType msgType);

LOG4QT_DECLARE_STATIC_LOGGER(static_logger, Log4Qt::LogManager)
Q_GLOBAL_STATIC(QMutex, singleton_guard)

LogManager::LogManager() :
    mLoggerRepository(new Hierarchy()),
    mHandleQtMessages(false),
    mWatchThisFile(false),
    mQtMsgHandler(nullptr)
{
}

LogManager::~LogManager()
{
    static_logger()->warn(u"Unexpected destruction of LogManager"_s);
}


Logger *LogManager::rootLogger()
{
    return instance()->mLoggerRepository->rootLogger();
}


QList<Logger *> LogManager::loggers()
{
    return instance()->mLoggerRepository->loggers();
}


Level LogManager::threshold()
{
    return instance()->mLoggerRepository->threshold();
}


void LogManager::setThreshold(Level level)
{
    instance()->mLoggerRepository->setThreshold(level);
}


bool LogManager::exists(const char *pName)
{
    return instance()->mLoggerRepository->exists(QLatin1String(pName));
}


LogManager *LogManager::instance()
{
    // Do not use Q_GLOBAL_STATIC. The LogManager is rather expensive
    // to construct, an exit handler must be set and doStartup must be
    // called.

    LogManager *p = mInstance.load(std::memory_order_acquire);
    if (p == nullptr)
    {
        QMutexLocker locker(singleton_guard());
        p = mInstance.load(std::memory_order_relaxed);
        if (p == nullptr)
        {
            p = new LogManager;
            // Publish before running setup so recursive calls into instance()
            // observe a fully-constructed object (the setup steps below may
            // call back into instance() via the logging system).
            mInstance.store(p, std::memory_order_release);
            atexit(shutdown);
            p->doConfigureLogLogger();
            p->welcome();
            p->doStartup();
        }
    }
    return p;
}


Logger *LogManager::logger(const QString &name)
{
    return instance()->mLoggerRepository->logger(name);
}


void LogManager::resetConfiguration()
{
    setHandleQtMessages(false);
    instance()->mLoggerRepository->resetConfiguration();
    configureLogLogger();
}


const char *LogManager::version()
{
    return LOG4QT_VERSION_STR;
}

QVersionNumber LogManager::versionNumber()
{
    return QVersionNumber(LOG4QT_VERSION_MAJOR, LOG4QT_VERSION_MINOR, LOG4QT_VERSION_PATCH);
}

void LogManager::shutdown()
{
    instance()->mLoggerRepository->shutdown();
}


QString LogManager::filterRules()
{
    LogManager *self = instance();
    QMutexLocker locker(&self->mObjectGuard);
    return self->mFilterRules;
}

QString LogManager::messagePattern()
{
    LogManager *self = instance();
    QMutexLocker locker(&self->mObjectGuard);
    return self->mMessagePattern;
}

void LogManager::doSetHandleQtMessages(bool handleQtMessages)
{
    QMutexLocker locker(&mObjectGuard);

    if (instance()->mHandleQtMessages == handleQtMessages)
        return;

    instance()->mHandleQtMessages = handleQtMessages;
    if (instance()->mHandleQtMessages)
    {
        static_logger()->trace(u"Activate Qt message handling"_s);
        instance()->mQtMsgHandler = qInstallMessageHandler(qtMessageHandler);
    }
    else
    {
        static_logger()->trace(u"Deactivate Qt message handling"_s);
        qInstallMessageHandler(instance()->mQtMsgHandler);
    }
}

void LogManager::doSetWatchThisFile(bool watchThisFile)
{
    QMutexLocker locker(&mObjectGuard);

    if (instance()->mWatchThisFile == watchThisFile)
        return;

    instance()->mWatchThisFile = watchThisFile;
    static_logger()->trace(u"%1able watching the current properties file"_s, watchThisFile ? "En" : "Dis");
}

void LogManager::doSetFilterRules(const QString &filterRules)
{
    QMutexLocker locker(&mObjectGuard);

    if (instance()->mFilterRules == filterRules)
        return;

    instance()->mFilterRules = filterRules;
    QLoggingCategory::setFilterRules(filterRules);
    static_logger()->trace(u"Setting filter rules to: %1"_s, filterRules);
}

void LogManager::doSetMessagePattern(const QString &messagePattern)
{
    QMutexLocker locker(&mObjectGuard);

    if (instance()->mMessagePattern == messagePattern)
        return;

    instance()->mMessagePattern = messagePattern;
    qSetMessagePattern(messagePattern);
    static_logger()->trace(u"Setting message pattern to: %1"_s, messagePattern);
}

void LogManager::doConfigureLogLogger()
{
    QMutexLocker locker(&instance()->mObjectGuard);

    // Level
    QString value = InitialisationHelper::setting(u"Debug"_s,
                    u"ERROR"_s);
    logLogger()->setLevel(OptionConverter::toLevel(value, Level::DEBUG_INT));

    // Common layout
    LayoutSharedPtr p_layout(new TTCCLayout());
    p_layout->setName(u"LogLog TTCC"_s);
    static_cast<TTCCLayout *>(p_layout.data())->setContextPrinting(false);
    p_layout->activateOptions();

    // Common deny all filter
    FilterSharedPtr p_denyall(new DenyAllFilter());
    p_denyall->activateOptions();

    // ConsoleAppender on stdout for all events <= INFO
    ConsoleAppender *p_appender;
    p_appender = new ConsoleAppender(p_layout, ConsoleAppender::StdOut);
    auto *pFilterStdout = new LevelRangeFilter();
    pFilterStdout->setNext(p_denyall);
    pFilterStdout->setLevelMin(Level::NULL_INT);
    pFilterStdout->setLevelMax(Level::INFO_INT);
    pFilterStdout->activateOptions();
    p_appender->setName(u"LogLog stdout"_s);
    p_appender->addFilter(FilterSharedPtr(pFilterStdout));
    p_appender->activateOptions();
    logLogger()->addAppender(AppenderSharedPtr(p_appender));

    // ConsoleAppender on stderr for all events >= WARN
    p_appender = new ConsoleAppender(p_layout, ConsoleAppender::StdErr);
    auto *pFilterStderr = new LevelRangeFilter();
    pFilterStderr->setNext(p_denyall);
    pFilterStderr->setLevelMin(Level::WARN_INT);
    pFilterStderr->setLevelMax(Level::OFF_INT);
    pFilterStderr->activateOptions();
    p_appender->setName(u"LogLog stderr"_s);
    p_appender->addFilter(FilterSharedPtr(pFilterStderr));
    p_appender->activateOptions();
    logLogger()->addAppender(AppenderSharedPtr(p_appender));
}

/*!
 * \brief LogManager::doStartup
 *
 * 1. If "DefaultInitOverride" or LOG4QT_DEFAULTINITOVERRIDE is not "false" then the initialization is skipped.
 * 2. If the file from "Configuration" or from LOG4QT_CONFIGURATION exists this file is used
 * 3. Check Settings from [Log4Qt/Properties] exists the configdata from there is used
 * 4. Check if <application binaryname>.log4qt.properties exists this file is used
 * 5. Check if <application binaryname.exe.log4qt.properties exists this file is used
 * 6. Check if "log4qt.properties" exists in the executables path
 * 7. Check if "log4qt.properties" exists in the current working directory
 */
void LogManager::doStartup()
{
    QMutexLocker locker(&instance()->mObjectGuard);

    // Override
    QString default_value = u"false"_s;
    QString value = InitialisationHelper::setting(u"DefaultInitOverride"_s,
                    default_value);
    if (value != default_value)
    {
        static_logger()->debug(u"DefaultInitOverride is set. Aborting default initialisation"_s);
        return;
    }

    // Configuration using setting Configuration
    value = InitialisationHelper::setting(u"Configuration"_s);
    if (!value.isEmpty() && QFile::exists(value))
    {
        static_logger()->debug(u"Default initialisation configures from file '%1' specified by Configure"_s, value);
        if (value.endsWith(u".json"_s, Qt::CaseInsensitive))
            JsonConfigurator::configure(value);
        else if (value.endsWith(u".xml"_s, Qt::CaseInsensitive))
            XmlConfigurator::configure(value);
        else
            PropertyConfigurator::configure(value);
        return;
    }

    const QString default_properties(u"log4qt.properties"_s);
    const QString default_json(u"log4qt.json"_s);
    const QString default_xml(u"log4qt.xml"_s);
    QStringList filesToCheck;

    // Configuration using setting
    if ([[maybe_unused]] auto app = QCoreApplication::instance())
    {
        const QLatin1String log4qt_group("Log4Qt");
        const QLatin1String properties_group("Properties");
        QSettings s;
        s.beginGroup(log4qt_group);
        if (s.childGroups().contains(properties_group))
        {
            static_logger()->debug(u"Default initialisation configures from setting '%1/%2'"_s, log4qt_group, properties_group);
            s.beginGroup(properties_group);
            PropertyConfigurator::configure(s);
            return;
        }

        // Configuration using executable file name + .log4qt.properties / .log4qt.json / .log4qt.xml
        QString binPropsFile = QCoreApplication::applicationFilePath() + QLatin1Char('.') + default_properties;
        QString binJsonFile = QCoreApplication::applicationFilePath() + QLatin1Char('.') + default_json;
        QString binXmlFile = QCoreApplication::applicationFilePath() + QLatin1Char('.') + default_xml;

        filesToCheck << binPropsFile << binJsonFile << binXmlFile;
        if (binPropsFile.contains(QLatin1String(".exe."), Qt::CaseInsensitive))
        {
            binPropsFile.replace(QLatin1String(".exe."), QLatin1String("."), Qt::CaseInsensitive);
            filesToCheck << binPropsFile;
            binJsonFile.replace(QLatin1String(".exe."), QLatin1String("."), Qt::CaseInsensitive);
            filesToCheck << binJsonFile;
            binXmlFile.replace(QLatin1String(".exe."), QLatin1String("."), Qt::CaseInsensitive);
            filesToCheck << binXmlFile;
        }

        const QString appDir = QFileInfo(QCoreApplication::applicationFilePath()).path() + QLatin1Char('/');
        filesToCheck << appDir + default_properties << appDir + default_json << appDir + default_xml;
    }

    filesToCheck << default_properties << default_json << default_xml;

    for (const auto &configFileName : filesToCheck)
    {
        // Configuration using default file
        if (QFile::exists(configFileName))
        {
            const bool isJson = configFileName.endsWith(u".json"_s, Qt::CaseInsensitive);
            const bool isXml = configFileName.endsWith(u".xml"_s, Qt::CaseInsensitive);
            static_logger()->debug(u"Default initialisation configures from default file '%1'"_s, configFileName);
            if (isJson)
            {
                JsonConfigurator::configure(configFileName);
                if (mWatchThisFile)
                    ConfiguratorHelper::setConfigurationFile(configFileName, JsonConfigurator::configure);
            }
            else if (isXml)
            {
                XmlConfigurator::configure(configFileName);
                if (mWatchThisFile)
                    ConfiguratorHelper::setConfigurationFile(configFileName, XmlConfigurator::configure);
            }
            else
            {
                PropertyConfigurator::configure(configFileName);
                if (mWatchThisFile)
                    ConfiguratorHelper::setConfigurationFile(configFileName, PropertyConfigurator::configure);
            }
            return;
        }
    }

    static_logger()->debug(u"Default initialisation leaves package unconfigured"_s);
}


void LogManager::welcome()
{
    static_logger()->info(u"Initialising Log4Qt %1"_s,
                          u"" LOG4QT_VERSION_STR ""_s);

    // Debug: Info
    if (static_logger()->isDebugEnabled())
    {
        // Create a nice timestamp with UTC offset
        DateTime start_time = QDateTime::fromMSecsSinceEpoch(InitialisationHelper::startTime());
        QString offset;
        {
            QDateTime utc = start_time.toUTC();
            QDateTime local = start_time.toLocalTime();
            QDateTime local_as_utc = QDateTime(local.date(), local.time(), QTimeZone::utc());
            int min = utc.secsTo(local_as_utc) / 60;
            if (min < 0)
                offset += QLatin1Char('-');
            else
                offset += QLatin1Char('+');
            min = abs(min);
            offset += QString::number(min / 60).rightJustified(2, QLatin1Char('0'));
            offset += QLatin1Char(':');
            offset += QString::number(min % 60).rightJustified(2, QLatin1Char('0'));
        }
        static_logger()->debug(u"Program startup time is %1 (UTC%2)"_s,
                               start_time.toString(u"ISO8601"_s),
                               offset);
        static_logger()->debug(u"Internal logging uses the level %1"_s,
                               logLogger()->level().toString());
    }

    // Trace: Dump settings
    if (static_logger()->isTraceEnabled())
    {
        static_logger()->trace(u"Settings from the system environment:"_s);
        auto settings = InitialisationHelper::environmentSettings();
        for (const auto &[key, value] : settings.asKeyValueRange())
            static_logger()->trace(u"    %1: '%2'"_s, key, value);

        static_logger()->trace(u"Settings from the application settings:"_s);
        if (QCoreApplication::instance())
        {
            const QLatin1String log4qt_group("Log4Qt");
            const QLatin1String properties_group("Properties");
            static_logger()->trace(u"    %1:"_s, log4qt_group);
            QSettings s;
            s.beginGroup(log4qt_group);
            for (const auto &entry : s.childKeys())
                static_logger()->trace(u"        %1: '%2'"_s,
                                       entry,
                                       s.value(entry).toString());
            static_logger()->trace(u"    %1/%2:"_s, log4qt_group, properties_group);
            s.beginGroup(properties_group);
            for (const auto &entry : s.childKeys())
                static_logger()->trace(u"        %1: '%2'"_s,
                                       entry,
                                       s.value(entry).toString());
        }
        else
            static_logger()->trace(u"    QCoreApplication::instance() is not available"_s);
    }
}

void LogManager::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Level level;
    switch (type)
    {
    case QtDebugMsg:
        level = Level::DEBUG_INT;
        break;
    case QtWarningMsg:
        level = Level::WARN_INT;
        break;
    case QtCriticalMsg:
        level = Level::ERROR_INT;
        break;
    case QtFatalMsg:
        level = Level::FATAL_INT;
        break;
    case QtInfoMsg:
        level = Level::INFO_INT;
        break;
    default:
        level = Level::TRACE_INT;
    }
    LoggingEvent loggingEvent = LoggingEvent(instance()->qtLogger(),
                                             level,
                                             message,
                                             MessageContext(context.file, context.line, context.function),
                                             u"Qt "_s % context.category);

    instance()->qtLogger()->log(loggingEvent);


    // Qt fatal behaviour copied from global.cpp qt_message_output()
    // begin {

    if (isFatal(type))
        qt_message_fatal(type, context, message);

    // } end
}

#ifdef Q_OS_WIN
static inline void convert_to_wchar_t_elided(wchar_t *d, size_t space, const char *s) Q_DECL_NOEXCEPT
{
    size_t len = qstrlen(s);
    if (len + 1 > space) {
        const size_t skip = len - space + 4; // 4 for "..." + '\0'
        s += skip;
        len -= skip;
        for (int i = 0; i < 3; ++i)
          *d++ = L'.';
    }
    while (len--)
        *d++ = *s++;
    *d++ = 0;
}
#endif

static void qt_message_fatal([[maybe_unused]] QtMsgType, [[maybe_unused]] const QMessageLogContext &context, [[maybe_unused]] const QString &message)
{
#if defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    wchar_t contextFileL[256];
    // we probably should let the compiler do this for us, by declaring QMessageLogContext::file to
    // be const wchar_t * in the first place, but the #ifdefery above is very complex  and we
    // wouldn't be able to change it later on...
    convert_to_wchar_t_elided(contextFileL, sizeof contextFileL / sizeof *contextFileL,
                              context.file);
    // get the current report mode
    int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
    _CrtSetReportMode(_CRT_ERROR, reportMode);

    int ret = _CrtDbgReportW(_CRT_ERROR, contextFileL, context.line, _CRT_WIDE(QT_VERSION_STR),
                             reinterpret_cast<const wchar_t *>(message.utf16()));
    if ((ret == 0) && (reportMode & _CRTDBG_MODE_WNDW))
        return; // ignore
    else if (ret == 1)
        _CrtDbgBreak();
#else
#endif

    std::abort();
}

static bool isFatal(QtMsgType msgType)
{
    if (msgType == QtFatalMsg)
        return true;

    if (msgType == QtCriticalMsg) {
        static bool fatalCriticals = !qEnvironmentVariableIsEmpty("QT_FATAL_CRITICALS");
        return fatalCriticals;
    }

    if (msgType == QtWarningMsg) {
        static bool fatalWarnings = !qEnvironmentVariableIsEmpty("QT_FATAL_WARNINGS");
        return fatalWarnings;
    }

    return false;
}

std::atomic<LogManager *> LogManager::mInstance{nullptr};

}  // namespace Log4Qt
