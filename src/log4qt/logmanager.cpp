/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logmanager.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:     Sep 2008, Martin Heinrich:
 *              - Resolved compilation problem with Microsoft Visual Studio 2005
 *              Feb 2009, Martin Heinrich
 *              - Fixed VS 2008 unreferenced formal parameter warning by using
 *                Q_UNUSED in operator<<.
 *
 *
 * Copyright 2007 - 2009 Martin Heinrich
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
#include "propertyconfigurator.h"
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


namespace Log4Qt
{

static void qt_message_fatal(QtMsgType, const QMessageLogContext &context, const QString &message);
static bool isFatal(QtMsgType msgType);

LOG4QT_DECLARE_STATIC_LOGGER(static_logger, Log4Qt::LogManager)
Q_GLOBAL_STATIC(QMutex, singleton_guard)

LogManager::LogManager() :
    mObjectGuard(QMutex::Recursive), // Recursive for doStartup() to call doConfigureLogLogger()
    mpLoggerRepository(new Hierarchy()),
    mHandleQtMessages(false),
    mWatchThisFile(false),
    mQtMsgHandler(Q_NULLPTR)
{
}

LogManager::~LogManager()
{
    static_logger()->warn("Unexpected destruction of LogManager");

    // doSetConfigureHandleQtMessages(false);
    // delete mpLoggerRepository;
}


Logger *LogManager::rootLogger()
{
    return instance()->mpLoggerRepository->rootLogger();
}


QList<Logger *> LogManager::loggers()
{
    return instance()->mpLoggerRepository->loggers();
}


Level LogManager::threshold()
{
    return instance()->mpLoggerRepository->threshold();
}


void LogManager::setThreshold(Level level)
{
    instance()->mpLoggerRepository->setThreshold(level);
}


bool LogManager::exists(const char *pName)
{
    return instance()->mpLoggerRepository->exists(QLatin1String(pName));
}


LogManager *LogManager::instance()
{
    // Do not use Q_GLOBAL_STATIC. The LogManager is rather expensive
    // to construct, an exit handler must be set and doStartup must be
    // called.

    if (!mspInstance)
    {
        QMutexLocker locker(singleton_guard());
        if (!mspInstance)
        {
            mspInstance = new LogManager;
            // qAddPostRoutine(shutdown);
            atexit(shutdown);
            mspInstance->doConfigureLogLogger();
            mspInstance->welcome();
            mspInstance->doStartup();
        }
    }
    return mspInstance;
}


Logger *LogManager::logger(const QString &rName)
{
    return instance()->mpLoggerRepository->logger(rName);
}


void LogManager::resetConfiguration()
{
    setHandleQtMessages(false);
    instance()->mpLoggerRepository->resetConfiguration();
    configureLogLogger();
}


const char *LogManager::version()
{
    return LOG4QT_VERSION_STR;
}


void LogManager::shutdown()
{
    instance()->mpLoggerRepository->shutdown();
}


void LogManager::doSetHandleQtMessages(bool handleQtMessages)
{
    QMutexLocker locker(&mObjectGuard);

    if (instance()->mHandleQtMessages == handleQtMessages)
        return;

    instance()->mHandleQtMessages = handleQtMessages;
    if (instance()->mHandleQtMessages)
    {
        static_logger()->trace("Activate Qt message handling");
        instance()->mQtMsgHandler = qInstallMessageHandler(qtMessageHandler);
    }
    else
    {
        static_logger()->trace("Deactivate Qt message handling");
        qInstallMessageHandler(instance()->mQtMsgHandler);
    }
}

void LogManager::doSetWatchThisFile(bool watchThisFile)
{
    QMutexLocker locker(&mObjectGuard);

    if (instance()->mWatchThisFile == watchThisFile)
        return;

    instance()->mWatchThisFile = watchThisFile;
    static_logger()->trace("%1able watching the current properties file", watchThisFile ? "En" : "Dis");
}


void LogManager::doConfigureLogLogger()
{
    QMutexLocker locker(&instance()->mObjectGuard);

    // Level
    QString value = InitialisationHelper::setting(QLatin1String("Debug"),
                    QLatin1String("ERROR"));
    logLogger()->setLevel(OptionConverter::toLevel(value, Level::DEBUG_INT));

    // Common layout
    LayoutSharedPtr p_layout(new TTCCLayout());
    p_layout->setName(QLatin1String("LogLog TTCC"));
    static_cast<TTCCLayout *>(p_layout.data())->setContextPrinting(false);
    p_layout->activateOptions();

    // Common deny all filter
    FilterSharedPtr p_denyall(new DenyAllFilter());
    p_denyall->activateOptions();

    // ConsoleAppender on stdout for all events <= INFO
    ConsoleAppender *p_appender;
    p_appender = new ConsoleAppender(p_layout, ConsoleAppender::STDOUT_TARGET);
    LevelRangeFilter *pFilterStdout = new LevelRangeFilter();
    pFilterStdout->setNext(p_denyall);
    pFilterStdout->setLevelMin(Level::NULL_INT);
    pFilterStdout->setLevelMax(Level::INFO_INT);
    pFilterStdout->activateOptions();
    p_appender->setName(QLatin1String("LogLog stdout"));
    p_appender->addFilter(FilterSharedPtr(pFilterStdout));
    p_appender->activateOptions();
    logLogger()->addAppender(AppenderSharedPtr(p_appender));

    // ConsoleAppender on stderr for all events >= WARN
    p_appender = new ConsoleAppender(p_layout, ConsoleAppender::STDERR_TARGET);
    LevelRangeFilter *pFilterStderr = new LevelRangeFilter();
    pFilterStderr->setNext(p_denyall);
    pFilterStderr->setLevelMin(Level::WARN_INT);
    pFilterStderr->setLevelMax(Level::OFF_INT);
    pFilterStderr->activateOptions();
    p_appender->setName(QLatin1String("LogLog stderr"));
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
    QString default_value = QLatin1String("false");
    QString value = InitialisationHelper::setting(QLatin1String("DefaultInitOverride"),
                    default_value);
    if (value != default_value)
    {
        static_logger()->debug("DefaultInitOverride is set. Aborting default initialisation");
        return;
    }

    // Configuration using setting Configuration
    value = InitialisationHelper::setting(QLatin1String("Configuration"));
    if (QFile::exists(value))
    {
        static_logger()->debug("Default initialisation configures from file '%1' specified by Configure", value);
        PropertyConfigurator::configure(value);
        return;
    }

    const QString default_file(QLatin1String("log4qt.properties"));
    QStringList filesToCheck;

    // Configuration using setting
    if (auto app = QCoreApplication::instance())
    {
        const QLatin1String log4qt_group("Log4Qt");
        const QLatin1String properties_group("Properties");
        QSettings s;
        s.beginGroup(log4qt_group);
        if (s.childGroups().contains(properties_group))
        {
            static_logger()->debug("Default initialisation configures from setting '%1/%2'", log4qt_group, properties_group);
            s.beginGroup(properties_group);
            PropertyConfigurator::configure(s);
            return;
        }

        // Configuration using executable file name + .log4qt.properties
        QString binConfigFile = app->applicationFilePath() + QLatin1Char('.') + default_file;

        filesToCheck << binConfigFile;
        if (binConfigFile.contains(".exe.", Qt::CaseInsensitive))
        {
            binConfigFile.replace(".exe.", ".", Qt::CaseInsensitive);
            filesToCheck << binConfigFile;
        }

        filesToCheck << QFileInfo(app->applicationFilePath()).path() + QLatin1Char('/') + default_file;
    }

    filesToCheck << default_file;

    for (const auto &rConfigFileName: filesToCheck)
    {
        // Configuration using default file
        if (QFile::exists(rConfigFileName))
        {
            static_logger()->debug("Default initialisation configures from default file '%1'", rConfigFileName);
            PropertyConfigurator::configure(rConfigFileName);
            if (mWatchThisFile)
               ConfiguratorHelper::setConfigurationFile(rConfigFileName, PropertyConfigurator::configure);
            return;
        }
    }

    static_logger()->debug("Default initialisation leaves package unconfigured");
}


void LogManager::welcome()
{
    static_logger()->info("Initialising Log4Qt %1",
                          QLatin1String(LOG4QT_VERSION_STR));

    // Debug: Info
    if (static_logger()->isDebugEnabled())
    {
        // Create a nice timestamp with UTC offset
        DateTime start_time = QDateTime::fromMSecsSinceEpoch(InitialisationHelper::startTime());
        QString offset;
        {
            QDateTime utc = start_time.toUTC();
            QDateTime local = start_time.toLocalTime();
            QDateTime local_as_utc = QDateTime(local.date(), local.time(), Qt::UTC);
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
        static_logger()->debug("Program startup time is %1 (UTC%2)",
                               start_time.toString(QLatin1String("ISO8601")),
                               offset);
        static_logger()->debug("Internal logging uses the level %1",
                               logLogger()->level().toString());
    }

    // Trace: Dump settings
    if (static_logger()->isTraceEnabled())
    {
        static_logger()->trace("Settings from the system environment:");
        for (const auto &entry : InitialisationHelper::environmentSettings().keys())
            static_logger()->trace("    %1: '%2'",
                                   entry,
                                   InitialisationHelper::environmentSettings().value(entry));

        static_logger()->trace("Settings from the application settings:");
        if (QCoreApplication::instance())
        {
            const QLatin1String log4qt_group("Log4Qt");
            const QLatin1String properties_group("Properties");
            static_logger()->trace("    %1:", log4qt_group);
            QSettings s;
            s.beginGroup(log4qt_group);
            for (const auto &entry : s.childKeys())
                static_logger()->trace("        %1: '%2'",
                                       entry,
                                       s.value(entry).toString());
            static_logger()->trace("    %1/%2:", log4qt_group, properties_group);
            s.beginGroup(properties_group);
            for (const auto &entry : s.childKeys())
                static_logger()->trace("        %1: '%2'",
                                       entry,
                                       s.value(entry).toString());
        }
        else
            static_logger()->trace("    QCoreApplication::instance() is not available");
    }
}

void LogManager::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &rMessage)
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg:
        level = Level::INFO_INT;
        break;
#endif
    default:
        level = Level::TRACE_INT;
    }
    LoggingEvent loggingEvent = LoggingEvent(instance()->qtLogger(),
                                             level,
                                             rMessage,
                                             context.file,
                                             context.function,
                                             context.line,
                                             QStringLiteral("Qt ") % context.category);

    instance()->qtLogger()->log(loggingEvent);


    // Qt fatal behaviour copied from global.cpp qt_message_output()
    // begin {

    if (isFatal(type))
        qt_message_fatal(type, context, rMessage);

    // } end
}

static void qt_message_fatal(QtMsgType, const QMessageLogContext &context, const QString &message)
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
    Q_UNUSED(context);
    Q_UNUSED(message);
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

    if (msgType == QtWarningMsg || msgType == QtCriticalMsg) {
        static bool fatalWarnings = !qEnvironmentVariableIsEmpty("QT_FATAL_WARNINGS");
        return fatalWarnings;
    }

    return false;
}

LogManager *LogManager::mspInstance = 0;

}  // namespace Log4Qt
