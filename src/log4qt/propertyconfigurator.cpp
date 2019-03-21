/******************************************************************************
 *
 * package:     Logging
 * file:        propertyconfigurator.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes      Feb 2009, Martin Heinrich
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

#include "propertyconfigurator.h"

#include "helpers/configuratorhelper.h"
#include "helpers/factory.h"
#include "helpers/optionconverter.h"
#include "helpers/properties.h"
#include "appender.h"
#include "layout.h"
#include "logger.h"
#include "logmanager.h"
#include "loggerrepository.h"
#include "varia/listappender.h"

#include <QFile>

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(staticLogger, Log4Qt::PropertyConfigurator)

bool PropertyConfigurator::doConfigure(const Properties &properties,
                                       LoggerRepository *loggerRepository)
{
    startCaptureErrors();
    configureFromProperties(properties, loggerRepository);
    return stopCaptureErrors();
}


bool PropertyConfigurator::doConfigure(const QString &configFileName,
                                       LoggerRepository *loggerRepository)
{
    startCaptureErrors();
    configureFromFile(configFileName, loggerRepository);
    return stopCaptureErrors();
}


bool PropertyConfigurator::doConfigure(const QSettings &settings,
                                       LoggerRepository *loggerRepository)
{
    startCaptureErrors();
    configureFromSettings(settings, loggerRepository);
    return stopCaptureErrors();
}


bool PropertyConfigurator::configure(const Properties &properties)
{
    PropertyConfigurator configurator;
    return configurator.doConfigure(properties);
}


bool PropertyConfigurator::configure(const QString &configFilename)
{
    PropertyConfigurator configurator;
    return configurator.doConfigure(configFilename);
}


bool PropertyConfigurator::configure(const QSettings &settings)
{
    PropertyConfigurator configurator;
    return configurator.doConfigure(settings);
}


bool PropertyConfigurator::configureAndWatch(const QString &configFilename)
{
    // Stop an existing watch to avoid a possible concurrent configuration
    ConfiguratorHelper::setConfigurationFile();
    if (configFilename.isEmpty())
        return true;

    PropertyConfigurator configurator;
    bool result = configurator.doConfigure(configFilename);
    ConfiguratorHelper::setConfigurationFile(configFilename, configure);
    return result;
}


void PropertyConfigurator::configureFromFile(const QString &configFileName,
        LoggerRepository *loggerRepository)
{
    QFile file(configFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to open property file '%1'"),
                                  CONFIGURATOR_OPENING_FILE_ERROR,
                                  "Log4Qt::PropertyConfigurator");
        e << configFileName;
        e.addCausingError(LogError(file.errorString(), file.error()));
        staticLogger()->error(e);
        return;
    }
    Properties properties;
    properties.load(&file);
    if (file.error())
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to read property file '%1'"),
                                  CONFIGURATOR_READING_FILE_ERROR,
                                  "Log4Qt::PropertyConfigurator");
        e << configFileName;
        e.addCausingError(LogError(file.errorString(), file.error()));
        staticLogger()->error(e);
        return;
    }
    configureFromProperties(properties, loggerRepository);
}


void PropertyConfigurator::configureFromProperties(const Properties &properties,
        LoggerRepository *loggerRepository)
{
    if (!loggerRepository)
        loggerRepository = LogManager::loggerRepository();

    configureGlobalSettings(properties, loggerRepository);
    configureRootLogger(properties, loggerRepository);
    configureNonRootElements(properties, loggerRepository);
    mAppenderRegistry.clear();
}


void PropertyConfigurator::configureFromSettings(const QSettings &settings,
        LoggerRepository *loggerRepository)
{
    Properties properties;
    properties.load(settings);
    configureFromProperties(properties, loggerRepository);
}


void PropertyConfigurator::configureGlobalSettings(const Properties &properties,
        LoggerRepository *loggerRepository) const
{
    Q_ASSERT_X(loggerRepository, "PropertyConfigurator::configureGlobalSettings()", "loggerRepository must not be null.");

    const QLatin1String key_reset("log4j.reset");
    const QLatin1String key_debug("log4j.Debug");
    const QLatin1String key_config_debug("log4j.configDebug");
    const QLatin1String key_threshold("log4j.threshold");
    const QLatin1String key_handle_qt_messages("log4j.handleQtMessages");
    const QLatin1String key_watch_this_file("log4j.watchThisFile");
    const QLatin1String key_filterRules("log4j.qtLogging.filterRules");
    const QLatin1String key_messagePattern("log4j.qtLogging.messagePattern");

    // Test each global setting and set it
    // - Reset: log4j.reset
    // - Debug: log4j.Debug, log4j.configDebug
    // - Threshold: log4j.threshold
    // - Handle Qt Messages: log4j.handleQtMessages
    // - Watch the properties file
    // - filterRules: QLoggingCategory::setFilterRules
    // - messagePattern: qSetMessagePattern

    // Reset
    QString value = properties.property(key_reset);
    if (!value.isEmpty() && OptionConverter::toBoolean(value, false))
    {
        // Use LogManager and not loggerRepository to reset internal
        // logging.
        LogManager::resetConfiguration();
        staticLogger()->debug(QStringLiteral("Reset configuration"));
    }

    // Debug
    value = properties.property(key_debug);
    if (value.isNull())
    {
        value = properties.property(key_config_debug);
        if (!value.isNull())
            staticLogger()->warn(QStringLiteral("[%1] is deprecated. Use [%2] instead."), key_config_debug, key_debug);
    }
    if (!value.isNull())
    {
        // Don't use OptionConverter::toLevel(). Invalid level string is a valid setting
        bool ok;
        Level level = Level::fromString(value, &ok);
        if (!ok)
            level = Level::DEBUG_INT;
        LogManager::logLogger()->setLevel(level);
        staticLogger()->debug(QStringLiteral("Set level for Log4Qt logging to %1"),
                        LogManager::logLogger()->level().toString());
    }

    // Threshold
    value = properties.property(key_threshold);
    if (!value.isNull())
    {
        loggerRepository->setThreshold(OptionConverter::toLevel(value, Level::ALL_INT));
        staticLogger()->debug(QStringLiteral("Set threshold for LoggerRepository to %1"),
                        loggerRepository->threshold().toString());
    }

    // Handle Qt messages
    value = properties.property(key_handle_qt_messages);
    if (!value.isNull())
    {
        LogManager::setHandleQtMessages(OptionConverter::toBoolean(value, false));
        staticLogger()->debug(QStringLiteral("Set handling of Qt messages LoggerRepository to %1"),
                        QVariant(LogManager::handleQtMessages()).toString());
    }

    // Watch this file
    value = properties.property(key_watch_this_file);
    if (!value.isNull())
    {
        LogManager::setWatchThisFile(OptionConverter::toBoolean(value, false));
        staticLogger()->debug(QStringLiteral("Set watching the properties file to %1"),
                        QVariant(LogManager::watchThisFile()).toString());
    }

    value = properties.property(key_filterRules);
    if (!value.isNull())
    {
        LogManager::setFilterRules(value);
        staticLogger()->debug(QStringLiteral("Set filter rules to %1"), LogManager::filterRules());
    }

    value = properties.property(key_messagePattern);
    if (!value.isNull())
    {
        LogManager::setMessagePattern(value);
        staticLogger()->debug(QStringLiteral("Set message pattern to %1"), LogManager::messagePattern());
    }
}


void PropertyConfigurator::configureNonRootElements(const Properties &properties,
        LoggerRepository *loggerRepository)
{
    Q_ASSERT_X(loggerRepository, "PropertyConfigurator::configureNonRootElements()", "loggerRepository must not be null.");

    const QString logger_prefix = QStringLiteral("log4j.logger.");
    const QString category_prefix = QStringLiteral("log4j.category.");

    // Iterate through all entries:
    // - Test for the logger/category prefix
    // - Convert JAVA class names to C++ ones
    // - Parse logger data (Level, Appender)
    // - Parse logger additivity

    QStringList keys = properties.propertyNames();
    for (const auto &key : qAsConst(keys))
    {
        QString java_name;
        if (key.startsWith(logger_prefix))
            java_name = key.mid(logger_prefix.length());
        else if (key.startsWith(category_prefix))
            java_name = key.mid(category_prefix.length());
        QString cpp_name = OptionConverter::classNameJavaToCpp(java_name);
        if (!java_name.isEmpty())
        {
            Logger *p_logger = loggerRepository->logger(cpp_name);
            QString value = OptionConverter::findAndSubst(properties, key);
            parseLogger(properties, p_logger, key, value);
            parseAdditivityForLogger(properties, p_logger, java_name);
        }
    }
}


void PropertyConfigurator::configureRootLogger(const Properties &properties,
        LoggerRepository *loggerRepository)
{
    Q_ASSERT_X(loggerRepository, "PropertyConfigurator::configureRootLogger()", "loggerRepository must not be null.");

    const QLatin1String key_root_logger("log4j.rootLogger");
    const QLatin1String key_root_category("log4j.rootCategory");

    // - Test for the logger/category prefix
    // - Parse logger data for root logger

    QString key = key_root_logger;
    QString value = OptionConverter::findAndSubst(properties, key);
    if (value.isNull())
    {
        key = key_root_category;
        value = OptionConverter::findAndSubst(properties, key);
        if (!value.isNull())
            staticLogger()->warn(QStringLiteral("[%1] is deprecated. Use [%2] instead."), key_root_category, key_root_logger);
    }

    if (value.isNull())
        staticLogger()->debug(QStringLiteral("Could not find root logger information. Is this correct?"));
    else
        parseLogger(properties, loggerRepository->rootLogger(), key, value);
}


void PropertyConfigurator::parseAdditivityForLogger(const Properties &properties,
        Logger *logger,
        const QString &log4jName) const
{
    Q_ASSERT_X(logger, "parseAdditivityForLogger()", "pLogger must not be null.");

    const QLatin1String additivity_prefix("log4j.additivity.");

    // - Lookup additivity key for logger
    // - Set additivity, if specified

    QString key = additivity_prefix + log4jName;
    QString value = OptionConverter::findAndSubst(properties, key);
    staticLogger()->debug(QStringLiteral("Parsing additivity for logger: key '%1', value '%2'"), key, value);
    if (!value.isEmpty())
    {
        bool additivity = OptionConverter::toBoolean(value, true);
        staticLogger()->debug(QStringLiteral("Setting additivity for logger '%1' to '%2'"), logger->name(), QVariant(value).toString());
        logger->setAdditivity(additivity);
    }
}


AppenderSharedPtr PropertyConfigurator::parseAppender(const Properties &properties,
        const QString &name)
{
    // - Test if appender has been parsed before
    // - Find appender key
    // - Create appender object
    // - Set layout, if required by appender
    // - Set properties
    // - Activate options
    // - Add appender to registry

    const QLatin1String appender_prefix("log4j.appender.");

    staticLogger()->debug(QStringLiteral("Parsing appender named '%1'"), name);

    if (mAppenderRegistry.contains(name))
    {
        staticLogger()->debug(QStringLiteral("Appender '%1' was already parsed."), name);
        return mAppenderRegistry.value(name);
    }

    QString key = appender_prefix + name;
    QString value = OptionConverter::findAndSubst(properties, key);
    if (value.isNull())
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Missing appender definition for appender named '%1'"),
                                  CONFIGURATOR_MISSING_APPENDER_ERROR,
                                  "Log4Qt::PropertyConfigurator");
        e << name;
        staticLogger()->error(e);
        return nullptr;
    }
    AppenderSharedPtr p_appender(Factory::createAppender(value));
    if (!p_appender)
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to create appender of class '%1' named '%2'"),
                                  CONFIGURATOR_UNKNOWN_APPENDER_CLASS_ERROR,
                                  "Log4Qt::PropertyConfigurator");
        e << value << name;
        staticLogger()->error(e);
        return nullptr;
    }
    p_appender->setName(name);

    if (p_appender->requiresLayout())
    {
        LayoutSharedPtr p_layout = parseLayout(properties, key);
        if (p_layout)
            p_appender->setLayout(p_layout);
        else
            return nullptr;
    }

    QStringList exclusions;
    exclusions << QStringLiteral("layout");
    setProperties(properties, key + QStringLiteral("."), exclusions, p_appender.data());
    auto *p_appenderskeleton = qobject_cast<AppenderSkeleton *>(p_appender.data());
    if (p_appenderskeleton)
        p_appenderskeleton->activateOptions();

    mAppenderRegistry.insert(name, p_appender);
    return p_appender;
}


LayoutSharedPtr PropertyConfigurator::parseLayout(const Properties &properties,
        const QString &appendename)
{
    Q_ASSERT_X(!appendename.isEmpty(), "PropertyConfigurator::parseLayout()", "rAppenderKey must not be empty.");

    // - Find layout key
    // - Create layput object
    // - Set properties
    // - Activate options

    const QLatin1String layout_suffix(".layout");

    staticLogger()->debug(QStringLiteral("Parsing layout for appender named '%1'"), appendename);

    QString key = appendename + layout_suffix;
    QString value = OptionConverter::findAndSubst(properties, key);
    LayoutSharedPtr p_layout;
    if (value.isNull())
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Missing layout definition for appender '%1'"),
                                  CONFIGURATOR_MISSING_LAYOUT_ERROR,
                                  "Log4Qt::PropertyConfigurator");
        e << appendename;
        staticLogger()->error(e);
        return p_layout;
    }
    p_layout.reset(Factory::createLayout(value));
    if (!p_layout)
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to create layoput of class '%1' requested by appender '%2'"),
                                  CONFIGURATOR_UNKNOWN_LAYOUT_CLASS_ERROR,
                                  "Log4Qt::PropertyConfigurator");
        e << value << appendename;
        staticLogger()->error(e);
        return p_layout;
    }

    setProperties(properties, key + QStringLiteral("."), QStringList(), p_layout.data());
    p_layout->activateOptions();

    return p_layout;
}


void PropertyConfigurator::parseLogger(const Properties &properties,
                                       Logger *logger,
                                       const QString &key,
                                       const QString &value)
{
    Q_ASSERT_X(logger, "PropertyConfigurator::parseLogger()", "pLogger must not be null.");
    Q_ASSERT_X(!key.isEmpty(), "PropertyConfigurator::parseLogger()", "rKey must not be empty.");

    const QLatin1String keyword_inherited("INHERITED");

    // - Split value on comma
    // - If level value, is specified
    //   - Test for NULL and INHERITED
    //   - Ensure root logger is not set to NULL
    //   - Set level
    // - For each entry
    //   - Create Appender

    staticLogger()->debug(QStringLiteral("Parsing logger: key '%1', value '%2'"), key, value);
    QStringList appenders = value.split(QLatin1Char(','));
    QStringListIterator i (appenders);

    // First entry is the level. There will be always one entry, even if the rValue is
    // empty or does not contain a comma.
    QString sValue = i.next().trimmed();
    if (!sValue.isEmpty())
    {
        Level level;
        if (sValue.compare(keyword_inherited, Qt::CaseInsensitive) == 0)
            level = Level::NULL_INT;
        else
            level = OptionConverter::toLevel(sValue, Level::DEBUG_INT);
        if (level == Level::NULL_INT && logger->name() == QString())
            staticLogger()->warn(QStringLiteral("The root logger level cannot be set to NULL."));
        else
        {
            logger->setLevel(level);
            staticLogger()->debug(QStringLiteral("Set level for logger '%1' to '%2'"),
                            logger->name(), logger->level().toString());
        }
    }

    logger->removeAllAppenders();
    while (i.hasNext())
    {
        sValue = i.next().trimmed();
        if (sValue.isEmpty())
            continue;
        AppenderSharedPtr appander = parseAppender(properties, sValue);
        if (appander)
            logger->addAppender(appander);
    }
}


void PropertyConfigurator::setProperties(const Properties &properties,
        const QString &prefix,
        const QStringList &exclusion,
        QObject *object)
{
    Q_ASSERT_X(!prefix.isEmpty(), "PropertyConfigurator::setProperties()", "rPrefix must not be empty.");
    Q_ASSERT_X(object, "PropertyConfigurator::setProperties()", "pObject must not be null.");

    // Iterate through all entries:
    // - Test for prefix to determine, if setting is for object
    // - Skip empty property name
    // - Skip property names in exclusion list
    // - Set property on object

    staticLogger()->debug(QStringLiteral("Setting properties for object of class '%1' from keys starting with '%2'"),
                    QLatin1String(object->metaObject()->className()),
                    prefix);

    QStringList keys = properties.propertyNames();
    for (const auto &key : qAsConst(keys))
    {
        if (!key.startsWith(prefix))
            continue;
        QString property = key.mid(prefix.length());
        if (property.isEmpty())
            continue;
        QStringList split_property = property.split(QLatin1Char('.'));
        if (exclusion.contains(split_property.at(0), Qt::CaseInsensitive))
            continue;
        QString value = OptionConverter::findAndSubst(properties, key);
        Factory::setObjectProperty(object, property, value);
    }
}

void PropertyConfigurator::startCaptureErrors()
{
    Q_ASSERT_X(!mpConfigureErrors, "PropertyConfigurator::startCaptureErrors()", "mpConfigureErrors must be empty.");

    auto *listAppender = new ListAppender();
    mpConfigureErrors.reset(listAppender);
    listAppender->setName(QStringLiteral("PropertyConfigurator"));
    listAppender->setConfiguratorList(true);
    listAppender->setThreshold(Level::ERROR_INT);
    LogManager::logLogger()->addAppender(mpConfigureErrors);
}

bool PropertyConfigurator::stopCaptureErrors()
{
    Q_ASSERT_X(mpConfigureErrors, "PropertyConfigurator::stopCaptureErrors()", "mpConfigureErrors must not be empty.");
    auto *listAppender = static_cast<ListAppender *>(mpConfigureErrors.data());
    LogManager::logLogger()->removeAppender(mpConfigureErrors);
    ConfiguratorHelper::setConfigureError(listAppender->list());
    bool result = (listAppender->list().count() == 0);
    return result;
}

} // namespace Log4Qt Logging
