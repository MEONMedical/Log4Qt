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

#include "propertyconfigurator.h"

#include "helpers/configuratorhelper.h"
#include "helpers/factory.h"
#include "helpers/optionconverter.h"
#include "helpers/properties.h"
#include "appenderskeleton.h"
#include "abstractlayout.h"
#include "asyncappender.h"
#include "logger.h"
#include "logmanager.h"
#include "loggerrepository.h"
#include "rollingfileappender.h"
#include "spi/triggeringpolicy.h"
#include "spi/rolloverstrategy.h"
#include "spi/headerfooterprovider.h"
#include "varia/listappender.h"

#include <QFile>
#include <QSet>

#include <utility>

using namespace Qt::StringLiterals;

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
        LogError e = LOG4QT_ERROR("Unable to open property file '%1'",
                                  ConfiguratorOpeningFileError,
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
        LogError e = LOG4QT_ERROR("Unable to read property file '%1'",
                                  ConfiguratorReadingFileError,
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

    Properties translated = translateLegacyProperties(properties);
    configureGlobalSettings(translated, loggerRepository);
    configureAppenders(translated);
    resolveAppenderReferences();
    configureRootLogger(translated, loggerRepository);
    configureLoggers(translated, loggerRepository);
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

    // Reset
    QString value = properties.property(u"reset"_s);
    if (!value.isEmpty() && OptionConverter::toBoolean(value, false))
    {
        LogManager::resetConfiguration();
        staticLogger()->debug(u"Reset configuration"_s);
    }

    // Status (replaces log4j.Debug)
    value = properties.property(u"status"_s);
    if (!value.isNull())
    {
        bool ok;
        Level level = Level::fromString(value, &ok);
        if (!ok)
            level = Level::DEBUG_INT;
        LogManager::logLogger()->setLevel(level);
        staticLogger()->debug(u"Set level for Log4Qt logging to %1"_s,
                        LogManager::logLogger()->level().toString());
    }

    // Threshold
    value = properties.property(u"threshold"_s);
    if (!value.isNull())
    {
        loggerRepository->setThreshold(OptionConverter::toLevel(value, Level::ALL_INT));
        staticLogger()->debug(u"Set threshold for LoggerRepository to %1"_s,
                        loggerRepository->threshold().toString());
    }

    // Handle Qt messages
    value = properties.property(u"handleQtMessages"_s);
    if (!value.isNull())
    {
        LogManager::setHandleQtMessages(OptionConverter::toBoolean(value, false));
        staticLogger()->debug(u"Set handling of Qt messages LoggerRepository to %1"_s,
                        QVariant(LogManager::handleQtMessages()).toString());
    }

    // Watch this file
    value = properties.property(u"watchThisFile"_s);
    if (!value.isNull())
    {
        LogManager::setWatchThisFile(OptionConverter::toBoolean(value, false));
        staticLogger()->debug(u"Set watching the properties file to %1"_s,
                        QVariant(LogManager::watchThisFile()).toString());
    }

    // Filter rules
    value = properties.property(u"filterRules"_s);
    if (!value.isNull())
    {
        value.replace(u";"_s, u"\n"_s);
        LogManager::setFilterRules(value);
        staticLogger()->debug(u"Set filter rules to %1"_s, LogManager::filterRules());
    }

    // Message pattern
    value = properties.property(u"messagePattern"_s);
    if (!value.isNull())
    {
        LogManager::setMessagePattern(value);
        staticLogger()->debug(u"Set message pattern to %1"_s, LogManager::messagePattern());
    }

    // Global HeaderFooterProvider
    value = OptionConverter::findAndSubst(properties, u"headerFooterProvider.type"_s);
    if (!value.isNull())
    {
        auto provider = HeaderFooterProviderSharedPtr(Factory::createHeaderFooterProvider(value));
        if (provider)
        {
            QStringList providerExclusions = {u"type"_s};
            setProperties(properties, u"headerFooterProvider."_s, providerExclusions, provider.get());
            provider->activateOptions();
            AbstractLayout::setGlobalHeaderFooterProvider(std::move(provider));
            staticLogger()->debug(u"Set global HeaderFooterProvider of type '%1'"_s, value);
        }
        else
        {
            staticLogger()->warn(u"Unable to create global HeaderFooterProvider of class '%1'"_s, value);
        }
    }
}


void PropertyConfigurator::configureAppenders(const Properties &properties)
{
    const QString appenderPrefix = u"appender."_s;

    QStringList aliases = extractAliases(properties, appenderPrefix);

    for (const auto &alias : aliases)
    {
        const QString prefix = appenderPrefix + alias + u"."_s;

        // Read type
        QString typeName = OptionConverter::findAndSubst(properties, prefix + u"type"_s);
        if (typeName.isNull())
        {
            LogError e = LOG4QT_ERROR("Missing appender type for appender alias '%1'",
                                      ConfiguratorMissingAppenderError,
                                      "Log4Qt::PropertyConfigurator");
            e << alias;
            staticLogger()->error(e);
            continue;
        }

        // Read name
        QString appenderName = OptionConverter::findAndSubst(properties, prefix + u"name"_s);
        if (appenderName.isNull())
            appenderName = alias;

        staticLogger()->debug(u"Configuring appender '%1' of type '%2'"_s, appenderName, typeName);

        // Create appender
        AppenderSharedPtr appender(Factory::createAppender(typeName));
        if (!appender)
        {
            LogError e = LOG4QT_ERROR("Unable to create appender of class '%1' named '%2'",
                                      ConfiguratorUnknownAppenderClassError,
                                      "Log4Qt::PropertyConfigurator");
            e << typeName << appenderName;
            staticLogger()->error(e);
            continue;
        }
        appender->setName(appenderName);

        // Layout
        QString layoutType = OptionConverter::findAndSubst(properties, prefix + u"layout.type"_s);
        if (!layoutType.isNull())
        {
            LayoutSharedPtr layout(Factory::createLayout(layoutType));
            if (!layout)
            {
                LogError e = LOG4QT_ERROR("Unable to create layout of class '%1' requested by appender '%2'",
                                          ConfiguratorUnknownLayoutClassError,
                                          "Log4Qt::PropertyConfigurator");
                e << layoutType << appenderName;
                staticLogger()->error(e);
                continue;
            }

            // Set layout properties
            const QString layoutPrefix = prefix + u"layout."_s;
            QStringList layoutExclusions;
            layoutExclusions << u"type"_s << u"headerFooterProvider"_s;
            setProperties(properties, layoutPrefix, layoutExclusions, layout.data());

            // Per-layout HeaderFooterProvider
            QString providerType = OptionConverter::findAndSubst(properties, layoutPrefix + u"headerFooterProvider.type"_s);
            if (!providerType.isNull())
            {
                auto provider = HeaderFooterProviderSharedPtr(Factory::createHeaderFooterProvider(providerType));
                if (provider)
                {
                    QStringList providerExclusions = {u"type"_s};
                    setProperties(properties, layoutPrefix + u"headerFooterProvider."_s, providerExclusions, provider.get());
                    provider->activateOptions();
                    layout->setHeaderFooterProvider(std::move(provider));
                }
                else
                {
                    staticLogger()->warn(u"Unable to create layout HeaderFooterProvider of class '%1' for appender '%2'"_s, providerType, appenderName);
                }
            }

            layout->activateOptions();
            appender->setLayout(layout);
        }
        else if (appender->requiresLayout())
        {
            LogError e = LOG4QT_ERROR("Missing layout definition for appender '%1'",
                                      ConfiguratorMissingLayoutError,
                                      "Log4Qt::PropertyConfigurator");
            e << appenderName;
            staticLogger()->error(e);
            continue;
        }

        // Filters
        const QString filterPrefix = prefix + u"filter."_s;
        QStringList filterAliases = extractAliases(properties, filterPrefix);
        for (const auto &filterAlias : filterAliases)
        {
            const QString fPrefix = filterPrefix + filterAlias + u"."_s;
            QString filterType = OptionConverter::findAndSubst(properties, fPrefix + u"type"_s);
            if (filterType.isNull())
                continue;

            Filter *filter = Factory::createFilter(filterType);
            if (!filter)
            {
                staticLogger()->warn(u"Unable to create filter of class '%1' for appender '%2'"_s, filterType, appenderName);
                continue;
            }

            QStringList filterExclusions;
            filterExclusions << u"type"_s;
            setProperties(properties, fPrefix, filterExclusions, filter);
            filter->activateOptions();

            if (auto *skeleton = qobject_cast<AppenderSkeleton *>(appender.data()))
                skeleton->addFilter(FilterSharedPtr(filter));
            else
                staticLogger()->warn(u"Appender '%1' does not support filters (not an AppenderSkeleton)"_s, appenderName);
        }

        // Triggering policies (multiple, like filters)
        const QString policyPrefix = prefix + u"policy."_s;
        QStringList policyAliases = extractAliases(properties, policyPrefix);
        for (const auto &policyAlias : policyAliases)
        {
            const QString pPrefix = policyPrefix + policyAlias + u"."_s;
            QString policyType = OptionConverter::findAndSubst(properties, pPrefix + u"type"_s);
            if (policyType.isNull())
                continue;

            TriggeringPolicy *policy = Factory::createTriggeringPolicy(policyType);
            if (!policy)
            {
                staticLogger()->warn(u"Unable to create triggering policy of class '%1' for appender '%2'"_s, policyType, appenderName);
                continue;
            }

            QStringList policyExclusions;
            policyExclusions << u"type"_s;
            setProperties(properties, pPrefix, policyExclusions, policy);
            policy->activateOptions();

            if (auto *rolling = qobject_cast<RollingFileAppender *>(appender.data()))
                rolling->addTriggeringPolicy(TriggeringPolicySharedPtr(policy));
            else
                staticLogger()->warn(u"Triggering policy specified for non-RollingFileAppender '%1'"_s, appenderName);
        }

        // Rollover strategy (single, like layout)
        QString strategyType = OptionConverter::findAndSubst(properties, prefix + u"strategy.type"_s);
        if (!strategyType.isNull())
        {
            RolloverStrategy *strategy = Factory::createRolloverStrategy(strategyType);
            if (!strategy)
            {
                staticLogger()->warn(u"Unable to create rollover strategy of class '%1' for appender '%2'"_s, strategyType, appenderName);
            }
            else
            {
                const QString strategyPrefix = prefix + u"strategy."_s;
                QStringList strategyExclusions;
                strategyExclusions << u"type"_s;
                setProperties(properties, strategyPrefix, strategyExclusions, strategy);
                strategy->activateOptions();

                if (auto *rolling = qobject_cast<RollingFileAppender *>(appender.data()))
                    rolling->setRolloverStrategy(RolloverStrategySharedPtr(strategy));
                else
                    staticLogger()->warn(u"Rollover strategy specified for non-RollingFileAppender '%1'"_s, appenderName);
            }
        }

        // Set remaining appender properties
        QStringList exclusions;
        exclusions << u"type"_s << u"name"_s << u"layout"_s << u"filter"_s << u"policy"_s << u"strategy"_s;
        setProperties(properties, prefix, exclusions, appender.data());

        if (auto *skeleton = qobject_cast<AppenderSkeleton *>(appender.data()))
            skeleton->activateOptions();

        mAppenderRegistry.insert(appenderName, appender);
    }
}


void PropertyConfigurator::resolveAppenderReferences()
{
    // Appenders are activated as they are parsed, so an appender referenced by
    // an earlier one does not exist yet at that point. Resolve those
    // references here, where the registry holds every appender of this
    // configuration — the same way appenderRef is resolved for the loggers.
    for (const auto &appender : std::as_const(mAppenderRegistry))
    {
        auto *async = qobject_cast<AsyncAppender *>(appender.data());
        if (async == nullptr)
            continue;

        const QString ref = async->errorRef();
        if (ref.isEmpty())
            continue;

        // The registry is keyed by appender name (the name property, or the
        // alias when none is given), which is what errorRef refers to.
        const AppenderSharedPtr errorAppender = mAppenderRegistry.value(ref);
        if (errorAppender)
            async->setErrorAppender(errorAppender);
        else
            staticLogger()->warn(u"Unable to resolve errorRef '%1' of appender '%2'"_s, ref, async->name());
    }
}


void PropertyConfigurator::configureRootLogger(const Properties &properties,
        LoggerRepository *loggerRepository)
{
    Q_ASSERT_X(loggerRepository, "PropertyConfigurator::configureRootLogger()", "loggerRepository must not be null.");

    Logger *rootLogger = loggerRepository->rootLogger();

    // Level
    QString levelStr = OptionConverter::findAndSubst(properties, u"rootLogger.level"_s);
    if (!levelStr.isNull())
    {
        Level level = OptionConverter::toLevel(levelStr, Level::DEBUG_INT);
        if (level == Level::NULL_INT)
            staticLogger()->warn(u"The root logger level cannot be set to NULL."_s);
        else
        {
            rootLogger->setLevel(level);
            staticLogger()->debug(u"Set level for root logger to '%1'"_s, rootLogger->level().toString());
        }
    }

    // Appender refs
    const QString refPrefix = u"rootLogger.appenderRef."_s;
    QStringList refAliases = extractAliases(properties, refPrefix);
    if (!refAliases.isEmpty())
        rootLogger->removeAllAppenders();
    for (const auto &refAlias : refAliases)
    {
        QString ref = OptionConverter::findAndSubst(properties, refPrefix + refAlias + u".ref"_s);
        if (ref.isNull())
            continue;

        if (mAppenderRegistry.contains(ref))
        {
            rootLogger->addAppender(mAppenderRegistry.value(ref));
            staticLogger()->debug(u"Added appender '%1' to root logger"_s, ref);
        }
        else
        {
            staticLogger()->warn(u"Appender '%1' referenced by root logger not found"_s, ref);
        }
    }
}


void PropertyConfigurator::configureLoggers(const Properties &properties,
        LoggerRepository *loggerRepository)
{
    Q_ASSERT_X(loggerRepository, "PropertyConfigurator::configureLoggers()", "loggerRepository must not be null.");

    const QString loggerPrefix = u"logger."_s;
    QStringList aliases = extractAliases(properties, loggerPrefix);

    for (const auto &alias : aliases)
    {
        const QString prefix = loggerPrefix + alias + u"."_s;

        // Name
        QString loggerName = OptionConverter::findAndSubst(properties, prefix + u"name"_s);
        if (loggerName.isNull())
        {
            staticLogger()->warn(u"Missing name for logger alias '%1'"_s, alias);
            continue;
        }

        Logger *logger = loggerRepository->logger(loggerName);

        // Level
        QString levelStr = OptionConverter::findAndSubst(properties, prefix + u"level"_s);
        if (!levelStr.isNull())
        {
            if (levelStr.compare(u"INHERITED"_s, Qt::CaseInsensitive) == 0)
                logger->setLevel(Level::NULL_INT);
            else
                logger->setLevel(OptionConverter::toLevel(levelStr, Level::DEBUG_INT));
            staticLogger()->debug(u"Set level for logger '%1' to '%2'"_s, loggerName, logger->level().toString());
        }

        // Additivity
        QString additivityStr = OptionConverter::findAndSubst(properties, prefix + u"additivity"_s);
        if (!additivityStr.isNull())
        {
            logger->setAdditivity(OptionConverter::toBoolean(additivityStr, true));
            staticLogger()->debug(u"Set additivity for logger '%1' to '%2'"_s, loggerName, additivityStr);
        }

        // Appender refs
        const QString refPrefix = prefix + u"appenderRef."_s;
        QStringList refAliases = extractAliases(properties, refPrefix);
        if (!refAliases.isEmpty())
            logger->removeAllAppenders();
        for (const auto &refAlias : refAliases)
        {
            QString ref = OptionConverter::findAndSubst(properties, refPrefix + refAlias + u".ref"_s);
            if (ref.isNull())
                continue;

            if (mAppenderRegistry.contains(ref))
            {
                logger->addAppender(mAppenderRegistry.value(ref));
                staticLogger()->debug(u"Added appender '%1' to logger '%2'"_s, ref, loggerName);
            }
            else
            {
                staticLogger()->warn(u"Appender '%1' referenced by logger '%2' not found"_s, ref, loggerName);
            }
        }
    }
}


void PropertyConfigurator::setProperties(const Properties &properties,
        const QString &prefix,
        const QStringList &exclusion,
        QObject *object) const
{
    Q_ASSERT_X(!prefix.isEmpty(), "PropertyConfigurator::setProperties()", "rPrefix must not be empty.");
    Q_ASSERT_X(object, "PropertyConfigurator::setProperties()", "pObject must not be null.");

    staticLogger()->debug(u"Setting properties for object of class '%1' from keys starting with '%2'"_s,
                    QLatin1String(object->metaObject()->className()),
                    prefix);

    QStringList keys = properties.propertyNames();

    for (const auto &key : keys)
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


QStringList PropertyConfigurator::extractAliases(const Properties &properties,
        const QString &prefix)
{
    QSet<QString> aliasSet;
    QStringList keys = properties.propertyNames();

    for (const auto &key : keys)
    {
        if (!key.startsWith(prefix))
            continue;
        QString remainder = key.mid(prefix.length());
        int dotIndex = remainder.indexOf(QLatin1Char('.'));
        if (dotIndex > 0)
            aliasSet.insert(remainder.left(dotIndex));
    }

    QStringList result(aliasSet.begin(), aliasSet.end());
    result.sort();
    return result;
}


Properties PropertyConfigurator::translateLegacyProperties(const Properties &properties)
{
    // Detect legacy format: any key starting with "log4j."
    bool isLegacy = false;
    const QStringList allKeys = properties.propertyNames();
    for (const auto &key : allKeys)
    {
        if (key.startsWith(u"log4j."_s))
        {
            isLegacy = true;
            break;
        }
    }

    if (!isLegacy)
        return properties;

    staticLogger()->debug(u"Detected legacy Log4j1-style properties, translating to Log4j2 format"_s);

    Properties result;

    // Pass 1: Copy all original keys so ${log4j.*} variable references still resolve.
    // The log4j.* keys won't interfere with parsing (it only matches appender.*, rootLogger.*, etc.)
    for (const auto &key : allKeys)
        result.setProperty(key, properties.property(key));

    // Pass 2: Global settings
    const std::pair<QString, QString> globalMappings[] = {
        {u"log4j.reset"_s,                    u"reset"_s},
        {u"log4j.configDebug"_s,              u"status"_s},
        {u"log4j.Debug"_s,                    u"status"_s},
        {u"log4j.threshold"_s,                u"threshold"_s},
        {u"log4j.handleQtMessages"_s,         u"handleQtMessages"_s},
        {u"log4j.watchThisFile"_s,            u"watchThisFile"_s},
        {u"log4j.qtLogging.filterRules"_s,    u"filterRules"_s},
        {u"log4j.qtLogging.messagePattern"_s, u"messagePattern"_s},
    };
    for (const auto &[oldKey, newKey] : globalMappings)
    {
        QString value = properties.property(oldKey);
        if (!value.isNull())
            result.setProperty(newKey, value);
    }

    // Pass 3a: Global HeaderFooterProvider (log4j.headerFooterProvider.*)
    const QString providerOldPrefix = u"log4j.headerFooterProvider."_s;
    const QString providerNewPrefix = u"headerFooterProvider."_s;
    for (const auto &key : allKeys)
    {
        if (!key.startsWith(providerOldPrefix))
            continue;
        result.setProperty(providerNewPrefix + key.mid(providerOldPrefix.size()),
                           properties.property(key));
    }

    // Pass 3: Appenders (log4j.appender.*)
    const QString appenderOldPrefix = u"log4j.appender."_s;
    for (const auto &key : allKeys)
    {
        if (!key.startsWith(appenderOldPrefix))
            continue;

        QString remainder = key.mid(appenderOldPrefix.length());
        QString value = properties.property(key);
        int firstDot = remainder.indexOf(u'.');

        if (firstDot < 0)
        {
            // log4j.appender.A1=classname -> appender.A1.type=classname
            result.setProperty(u"appender."_s + remainder + u".type"_s, value);
        }
        else
        {
            QString alias = remainder.left(firstDot);
            QString subKey = remainder.mid(firstDot + 1);

            if (subKey == u"layout"_s)
            {
                // log4j.appender.A1.layout=classname -> appender.A1.layout.type=classname
                result.setProperty(u"appender."_s + alias + u".layout.type"_s, value);
            }
            else if (subKey.startsWith(u"filter."_s))
            {
                // Check if this is filter.F=classname (class-as-value) or filter.F.prop=value
                QString filterRemainder = subKey.mid(7); // after "filter."
                int filterDot = filterRemainder.indexOf(u'.');
                if (filterDot < 0)
                {
                    // log4j.appender.A1.filter.F=classname -> appender.A1.filter.F.type=classname
                    result.setProperty(u"appender."_s + remainder + u".type"_s, value);
                }
                else
                {
                    // log4j.appender.A1.filter.F.prop=value -> appender.A1.filter.F.prop=value
                    result.setProperty(u"appender."_s + remainder, value);
                }
            }
            else
            {
                // log4j.appender.A1.target=X -> appender.A1.target=X
                result.setProperty(u"appender."_s + remainder, value);
            }
        }
    }

    // Pass 4: Root logger (log4j.rootLogger or log4j.rootCategory)
    QString rootValue = properties.property(u"log4j.rootLogger"_s);
    if (rootValue.isNull())
        rootValue = properties.property(u"log4j.rootCategory"_s);
    if (!rootValue.isNull())
    {
        const QStringList parts = rootValue.split(u',');
        QString level = parts.at(0).trimmed();
        if (!level.isEmpty())
            result.setProperty(u"rootLogger.level"_s, level);
        for (qsizetype i = 1; i < parts.size(); ++i)
        {
            QString ref = parts.at(i).trimmed();
            if (!ref.isEmpty())
                result.setProperty(u"rootLogger.appenderRef.%1.ref"_s.arg(i - 1), ref);
        }
    }

    // Pass 5: Named loggers (log4j.logger.* and log4j.category.*)
    QHash<QString, QString> loggerNameToAlias;
    for (const auto &key : allKeys)
    {
        QString prefix;
        if (key.startsWith(u"log4j.logger."_s))
            prefix = u"log4j.logger."_s;
        else if (key.startsWith(u"log4j.category."_s))
            prefix = u"log4j.category."_s;
        else
            continue;

        QString loggerName = key.mid(prefix.length());
        QString alias = loggerName;
        alias.replace(u'.', u'_');
        loggerNameToAlias.insert(loggerName, alias);

        QString value = properties.property(key);
        const QStringList parts = value.split(u',');
        QString level = parts.at(0).trimmed();

        result.setProperty(u"logger.%1.name"_s.arg(alias), loggerName);
        if (!level.isEmpty())
            result.setProperty(u"logger.%1.level"_s.arg(alias), level);
        for (qsizetype i = 1; i < parts.size(); ++i)
        {
            QString ref = parts.at(i).trimmed();
            if (!ref.isEmpty())
                result.setProperty(u"logger.%1.appenderRef.%2.ref"_s.arg(alias).arg(i - 1), ref);
        }
    }

    // Pass 6: Additivity (log4j.additivity.*)
    const QString additivityPrefix = u"log4j.additivity."_s;
    for (const auto &key : allKeys)
    {
        if (!key.startsWith(additivityPrefix))
            continue;

        QString loggerName = key.mid(additivityPrefix.length());
        QString value = properties.property(key);
        QString alias = loggerNameToAlias.value(loggerName);
        if (alias.isEmpty())
        {
            // Logger not defined via log4j.logger.X, create minimal entry
            alias = loggerName;
            alias.replace(u'.', u'_');
            result.setProperty(u"logger.%1.name"_s.arg(alias), loggerName);
        }
        result.setProperty(u"logger.%1.additivity"_s.arg(alias), value);
    }

    return result;
}


void PropertyConfigurator::startCaptureErrors()
{
    Q_ASSERT_X(!mpConfigureErrors, "PropertyConfigurator::startCaptureErrors()", "mpConfigureErrors must be empty.");

    auto *listAppender = new ListAppender();
    mpConfigureErrors.reset(listAppender);
    listAppender->setName(u"PropertyConfigurator"_s);
    listAppender->setConfiguratorList(true);
    listAppender->setThreshold(Level::ERROR_INT);
    LogManager::logLogger()->addAppender(mpConfigureErrors);
}

bool PropertyConfigurator::stopCaptureErrors()
{
    Q_ASSERT_X(mpConfigureErrors, "PropertyConfigurator::stopCaptureErrors()", "mpConfigureErrors must not be empty.");
    const auto *listAppender = static_cast<ListAppender *>(mpConfigureErrors.data());
    LogManager::logLogger()->removeAppender(mpConfigureErrors);
    ConfiguratorHelper::setConfigureError(listAppender->list());
    bool result = (listAppender->list().size() == 0);
    return result;
}

} // namespace Log4Qt
