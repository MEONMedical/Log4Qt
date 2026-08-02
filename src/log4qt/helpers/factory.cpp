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

#include "helpers/factory.h"

#include "consoleappender.h"
#include "fileappender.h"
#include "helpers/logerror.h"
#include "helpers/initialisationhelper.h"
#include "helpers/optionconverter.h"
#include "jsonlayout.h"
#include "patternlayout.h"
#include "rollingfileappender.h"
#include "signalappender.h"
#include "simplelayout.h"
#include "simpletimelayout.h"
#include "ttcclayout.h"
#include "xmllayout.h"

using namespace Qt::StringLiterals;

#ifdef LOG4QT_TELNET_LOGGING_SUPPORT
#include "telnetappender.h"
#endif

#ifdef LOG4QT_DB_LOGGING_SUPPORT
#include "databaseappender.h"
#include "databaselayout.h"
#endif

#include "asyncappender.h"
#include "mainthreadappender.h"
#include "systemlogappender.h"
#include "dailyrollingfileappender.h"
#ifdef Q_OS_WIN
#include "colorconsoleappender.h"
#include "wdcappender.h"
#endif

#include "spi/sizebasedtriggeringpolicy.h"
#include "spi/timebasedtriggeringpolicy.h"
#include "spi/crontriggeringpolicy.h"
#include "spi/onstartuptriggeringpolicy.h"
#include "spi/daterolloverstrategy.h"
#include "spi/defaultrolloverstrategy.h"
#include "spi/headerfooterprovider.h"
#include "varia/debugappender.h"
#include "varia/denyallfilter.h"
#include "varia/levelmatchfilter.h"
#include "varia/levelrangefilter.h"
#include "varia/listappender.h"
#include "varia/nullappender.h"
#include "varia/stringmatchfilter.h"

#include <QMetaObject>
#include <QMetaProperty>

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::Factory)

// Appenders

Appender *console_file_appender()
{
    return new ConsoleAppender;
}

Appender *create_debug_appender()
{
    return new DebugAppender;
}

Appender *create_file_appender()
{
    return new FileAppender;
}

Appender *create_list_appender()
{
    return new ListAppender;
}

Appender *create_null_appender()
{
    return new NullAppender;
}

Appender *create_rolling_file_appender()
{
    return new RollingFileAppender;
}

Appender *create_signal_appender()
{
    return new SignalAppender;
}

#ifdef LOG4QT_DB_LOGGING_SUPPORT
Appender *create_database_appender()
{
    return new DatabaseAppender;
}
#endif

#ifdef LOG4QT_TELNET_LOGGING_SUPPORT
Appender *create_telnet_appender()
{
    return new TelnetAppender;
}
#endif

Appender *create_async_appender()
{
    return new AsyncAppender;
}

Appender *create_mainthread_appender()
{
    return new MainThreadAppender;
}

Appender *create_systemlog_appender()
{
    return new SystemLogAppender;
}

Appender *create_dailyrollingfile_appender()
{
    return new DailyRollingFileAppender;
}

#ifdef Q_OS_WIN
Appender *create_wdc_appender()
{
    return new WDCAppender;
}

Appender *create_color_console_appender()
{
    return new ColorConsoleAppender;
}
#endif

// Filters

Filter *create_deny_all_filter()
{
    return new DenyAllFilter;
}

Filter *create_level_match_filter()
{
    return new LevelMatchFilter;
}

Filter *create_level_range_filter()
{
    return new LevelRangeFilter;
}

Filter *create_string_match_filter()
{
    return new StringMatchFilter;
}

// Layouts

AbstractLayout *create_pattern_layout()
{
    return new PatternLayout;
}

AbstractLayout *create_simple_layout()
{
    return new SimpleLayout;
}

AbstractLayout *create_simple_time_layout()
{
    return new SimpleTimeLayout;
}

#ifdef LOG4QT_DB_LOGGING_SUPPORT
AbstractLayout *create_database_layout()
{
    return new DatabaseLayout;
}
#endif

AbstractLayout *create_ttcc_layout()
{
    return new TTCCLayout;
}

AbstractLayout *create_xml_layout()
{
    return new XMLLayout;
}

AbstractLayout *create_json_layout()
{
    return new JsonLayout;
}

// TriggeringPolicies

TriggeringPolicy *create_size_based_triggering_policy()
{
    return new SizeBasedTriggeringPolicy;
}

TriggeringPolicy *create_time_based_triggering_policy()
{
    return new TimeBasedTriggeringPolicy;
}

TriggeringPolicy *create_cron_triggering_policy()
{
    return new CronTriggeringPolicy;
}

TriggeringPolicy *create_on_startup_triggering_policy()
{
    return new OnStartupTriggeringPolicy;
}

// HeaderFooterProviders

HeaderFooterProvider *create_pattern_header_footer_provider()
{
    return new PatternHeaderFooterProvider;
}

// RolloverStrategies

RolloverStrategy *create_default_rollover_strategy()
{
    return new DefaultRolloverStrategy;
}

RolloverStrategy *create_date_rollover_strategy()
{
    return new DateRolloverStrategy;
}

Factory::Factory()
{
    registerDefaultAppenders();
    registerDefaultFilters();
    registerDefaultLayouts();
    registerDefaultTriggeringPolicies();
    registerDefaultRolloverStrategies();
    registerDefaultHeaderFooterProviders();
}

LOG4QT_IMPLEMENT_INSTANCE(Factory)

Appender *Factory::doCreateAppender(const QString &appenderClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mAppenderRegistry.contains(appenderClassName))
    {
        logger()->warn(u"Request for the creation of Appender with class '%1', which is not registered"_s, appenderClassName);
        return nullptr;
    }
    return mAppenderRegistry.value(appenderClassName)();
}


Filter *Factory::doCreateFilter(const QString &filterClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mFilterRegistry.contains(filterClassName))
    {
        logger()->warn(u"Request for the creation of Filter with class '%1', which is not registered"_s, filterClassName);
        return nullptr;
    }
    return mFilterRegistry.value(filterClassName)();
}


AbstractLayout *Factory::doCreateLayout(const QString &layoutClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mLayoutRegistry.contains(layoutClassName))
    {
        logger()->warn(u"Request for the creation of Layout with class '%1', which is not registered"_s, layoutClassName);
        return nullptr;
    }
    return mLayoutRegistry.value(layoutClassName)();
}


void Factory::doRegisterAppender(const QString &appenderClassName,
                                 AppenderFactoryFunc appenderFactoryFunc)
{
    QMutexLocker locker(&mObjectGuard);

    if (appenderClassName.isEmpty())
    {
        logger()->warn(u"Registering Appender factory function with empty class name"_s);
        return;
    }
    mAppenderRegistry.insert(appenderClassName, appenderFactoryFunc);
}


void Factory::doRegisterFilter(const QString &filterClassName,
                               FilterFactoryFunc filterFactoryFunc)
{
    QMutexLocker locker(&mObjectGuard);

    if (filterClassName.isEmpty())
    {
        logger()->warn(u"Registering Filter factory function with empty class name"_s);
        return;
    }
    mFilterRegistry.insert(filterClassName, filterFactoryFunc);
}


void Factory::doRegisterLayout(const QString &layoutClassName,
                               LayoutFactoryFunc layoutFactoryFunc)
{
    QMutexLocker locker(&mObjectGuard);

    if (layoutClassName.isEmpty())
    {
        logger()->warn(u"Registering Layout factory function with empty class name"_s);
        return;
    }
    mLayoutRegistry.insert(layoutClassName, layoutFactoryFunc);
}


void Factory::doSetObjectProperty(QObject *object,
                                  const QString &property,
                                  const QString &value)
{
    // - Validate property
    // - Get correct property name from meta object
    // - Find specific property setter
    // - If no specfifc propery setter can be found,
    //   find general property setter
    // - Call property setter

    QMetaProperty meta_property;
    if (!validateObjectProperty(meta_property, property, object))
        return;

    QString propertyString = QLatin1String(meta_property.name());
    QString type = QLatin1String(meta_property.typeName());
    logger()->debug(u"Setting property '%1' on object of class '%2' to value '%3'"_s,
                    propertyString,
                    QLatin1String(object->metaObject()->className()),
                    value);

    QVariant variant;
    bool ok = true;
    if (type == u"bool"_s)
        variant = OptionConverter::toBoolean(value, &ok);
    else if (type == u"int"_s)
        variant = OptionConverter::toInt(value, &ok);
    else if (type == u"Log4Qt::Level"_s)
        variant = QVariant::fromValue(OptionConverter::toLevel(value, &ok));
    else if (type == u"QString"_s)
        variant = value;
    else if (type == u"QStringConverter::Encoding"_s)
        variant = QVariant::fromValue(OptionConverter::toEncoding(value, &ok));
    else if (meta_property.isEnumType())
    {
        // Generic fallback for enum (and flag) properties, e.g.
        // Qt::CaseSensitivity on StringMatchFilter: convert the configured
        // key name through the property's QMetaEnum.
        const QMetaEnum metaEnum = meta_property.enumerator();
        const QByteArray keys = value.toLatin1();
        int enumValue = 0;
        if (metaEnum.isValid())
            enumValue = metaEnum.isFlag() ? metaEnum.keysToValue(keys.constData(), &ok)
                                          : metaEnum.keyToValue(keys.constData(), &ok);
        else
            ok = false;
        if (!ok)
        {
            LogError e = LOG4QT_ERROR("Cannot convert value '%1' to enum type '%2' for property '%3' on object of class '%4'",
                                      ConfiguratorUnknownTypeError,
                                      "Log4Qt::Factory");
            e << value
              << type
              << property
              << QString::fromLatin1(object->metaObject()->className());
            logger()->error(e);
            return;
        }
        variant = QVariant(enumValue);
    }
    else
    {
        LogError e = LOG4QT_ERROR("Cannot convert to type '%1' for property '%2' on object of class '%3'",
                                  ConfiguratorUnknownTypeError,
                                  "Log4Qt::Factory");
        e << type
          << property
          << QString::fromLatin1(object->metaObject()->className());
        logger()->error(e);
        return;
    }
    if (!ok)
        return;

    // Everything is checked and the type is the one of the property.
    // Write should never return false
    if (!meta_property.write(object, variant))
        logger()->warn(u"Unexpected error result from QMetaProperty.write()"_s);
}


void Factory::doUnregisterAppender(const QString &appenderClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mAppenderRegistry.contains(appenderClassName))
    {
        logger()->warn(u"Request to unregister not registered Appender factory function for class '%1'"_s, appenderClassName);
        return;
    }
    mAppenderRegistry.remove(appenderClassName);
}


void Factory::doUnregisterFilter(const QString &filterClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mFilterRegistry.contains(filterClassName))
    {
        logger()->warn(u"Request to unregister not registered Filter factory function for class '%1'"_s, filterClassName);
        return;
    }
    mFilterRegistry.remove(filterClassName);
}


TriggeringPolicy *Factory::doCreateTriggeringPolicy(const QString &className)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mTriggeringPolicyRegistry.contains(className))
    {
        logger()->warn(u"Request for the creation of TriggeringPolicy with class '%1', which is not registered"_s, className);
        return nullptr;
    }
    return mTriggeringPolicyRegistry.value(className)();
}


void Factory::doRegisterTriggeringPolicy(const QString &className,
                                          TriggeringPolicyFactoryFunc func)
{
    QMutexLocker locker(&mObjectGuard);

    if (className.isEmpty())
    {
        logger()->warn(u"Registering TriggeringPolicy factory function with empty class name"_s);
        return;
    }
    mTriggeringPolicyRegistry.insert(className, func);
}


void Factory::doUnregisterTriggeringPolicy(const QString &className)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mTriggeringPolicyRegistry.contains(className))
    {
        logger()->warn(u"Request to unregister not registered TriggeringPolicy factory function for class '%1'"_s, className);
        return;
    }
    mTriggeringPolicyRegistry.remove(className);
}


RolloverStrategy *Factory::doCreateRolloverStrategy(const QString &className)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mRolloverStrategyRegistry.contains(className))
    {
        logger()->warn(u"Request for the creation of RolloverStrategy with class '%1', which is not registered"_s, className);
        return nullptr;
    }
    return mRolloverStrategyRegistry.value(className)();
}


void Factory::doRegisterRolloverStrategy(const QString &className,
                                          RolloverStrategyFactoryFunc func)
{
    QMutexLocker locker(&mObjectGuard);

    if (className.isEmpty())
    {
        logger()->warn(u"Registering RolloverStrategy factory function with empty class name"_s);
        return;
    }
    mRolloverStrategyRegistry.insert(className, func);
}


void Factory::doUnregisterRolloverStrategy(const QString &className)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mRolloverStrategyRegistry.contains(className))
    {
        logger()->warn(u"Request to unregister not registered RolloverStrategy factory function for class '%1'"_s, className);
        return;
    }
    mRolloverStrategyRegistry.remove(className);
}


void Factory::doUnregisterLayout(const QString &layoutClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mLayoutRegistry.contains(layoutClassName))
    {
        logger()->warn(u"Request to unregister not registered Layout factory function for class '%1'"_s, layoutClassName);
        return;
    }
    mLayoutRegistry.remove(layoutClassName);
}


void Factory::registerDefaultAppenders()
{
    mAppenderRegistry.insert(u"org.apache.log4j.ConsoleAppender"_s, console_file_appender);
    mAppenderRegistry.insert(u"Log4Qt::ConsoleAppender"_s, console_file_appender);
    mAppenderRegistry.insert(u"Console"_s, console_file_appender);
    mAppenderRegistry.insert(u"org.apache.log4j.varia.DebugAppender"_s, create_debug_appender);
    mAppenderRegistry.insert(u"Log4Qt::DebugAppender"_s, create_debug_appender);
    mAppenderRegistry.insert(u"Debug"_s, create_debug_appender);
    mAppenderRegistry.insert(u"org.apache.log4j.FileAppender"_s, create_file_appender);
    mAppenderRegistry.insert(u"Log4Qt::FileAppender"_s, create_file_appender);
    mAppenderRegistry.insert(u"File"_s, create_file_appender);
    mAppenderRegistry.insert(u"org.apache.log4j.varia.ListAppender"_s, create_list_appender);
    mAppenderRegistry.insert(u"Log4Qt::ListAppender"_s, create_list_appender);
    mAppenderRegistry.insert(u"List"_s, create_list_appender);
    mAppenderRegistry.insert(u"org.apache.log4j.varia.NullAppender"_s, create_null_appender);
    mAppenderRegistry.insert(u"Log4Qt::NullAppender"_s, create_null_appender);
    mAppenderRegistry.insert(u"Null"_s, create_null_appender);
    mAppenderRegistry.insert(u"org.apache.log4j.RollingFileAppender"_s, create_rolling_file_appender);
    mAppenderRegistry.insert(u"Log4Qt::RollingFileAppender"_s, create_rolling_file_appender);
    mAppenderRegistry.insert(u"RollingFile"_s, create_rolling_file_appender);

    mAppenderRegistry.insert(u"org.apache.log4j.SignalAppender"_s, create_signal_appender);
    mAppenderRegistry.insert(u"Log4Qt::SignalAppender"_s, create_signal_appender);
    mAppenderRegistry.insert(u"Signal"_s, create_signal_appender);
#ifdef Q_OS_WIN
    mAppenderRegistry.insert(u"org.apache.log4j.ColorConsoleAppender"_s, create_color_console_appender);
    mAppenderRegistry.insert(u"Log4Qt::ColorConsoleAppender"_s, create_color_console_appender);
    mAppenderRegistry.insert(u"ColorConsole"_s, create_color_console_appender);
#endif

#ifdef LOG4QT_DB_LOGGING_SUPPORT
    mAppenderRegistry.insert(u"org.apache.log4j.DatabaseAppender"_s, create_database_appender);
    mAppenderRegistry.insert(u"Log4Qt::DatabaseAppender"_s, create_database_appender);
    mAppenderRegistry.insert(u"Database"_s, create_database_appender);
#endif

#ifdef LOG4QT_TELNET_LOGGING_SUPPORT
    mAppenderRegistry.insert(u"org.apache.log4j.TelnetAppender"_s, create_telnet_appender);
    mAppenderRegistry.insert(u"Log4Qt::TelnetAppender"_s, create_telnet_appender);
    mAppenderRegistry.insert(u"Telnet"_s, create_telnet_appender);
#endif
    mAppenderRegistry.insert(u"org.apache.log4j.AsyncAppender"_s, create_async_appender);
    mAppenderRegistry.insert(u"Log4Qt::AsyncAppender"_s, create_async_appender);
    mAppenderRegistry.insert(u"Async"_s, create_async_appender);

    mAppenderRegistry.insert(u"org.apache.log4j.MainThreadAppender"_s, create_mainthread_appender);
    mAppenderRegistry.insert(u"Log4Qt::MainThreadAppender"_s, create_mainthread_appender);
    mAppenderRegistry.insert(u"MainThread"_s, create_mainthread_appender);

    mAppenderRegistry.insert(u"org.apache.log4j.SystemLogAppender"_s, create_systemlog_appender);
    mAppenderRegistry.insert(u"Log4Qt::SystemLogAppender"_s, create_systemlog_appender);
    mAppenderRegistry.insert(u"SystemLog"_s, create_systemlog_appender);

    mAppenderRegistry.insert(u"org.apache.log4j.DailyRollingFileAppender"_s, create_dailyrollingfile_appender);
    mAppenderRegistry.insert(u"Log4Qt::DailyRollingFileAppender"_s, create_dailyrollingfile_appender);
    mAppenderRegistry.insert(u"DailyFile"_s, create_dailyrollingfile_appender);
#ifdef Q_OS_WIN
    mAppenderRegistry.insert(u"org.apache.log4j.WDCAppender"_s, create_wdc_appender);
    mAppenderRegistry.insert(u"Log4Qt::WDCAppender"_s, create_wdc_appender);
    mAppenderRegistry.insert(u"WDC"_s, create_wdc_appender);
#endif
}


void Factory::registerDefaultFilters()
{
    mFilterRegistry.insert(u"org.apache.log4j.varia.DenyAllFilter"_s, create_deny_all_filter);
    mFilterRegistry.insert(u"Log4Qt::DenyAllFilter"_s, create_deny_all_filter);
    mFilterRegistry.insert(u"DenyAll"_s, create_deny_all_filter);
    mFilterRegistry.insert(u"org.apache.log4j.varia.LevelMatchFilter"_s, create_level_match_filter);
    mFilterRegistry.insert(u"Log4Qt::LevelMatchFilter"_s, create_level_match_filter);
    mFilterRegistry.insert(u"LevelMatch"_s, create_level_match_filter);
    mFilterRegistry.insert(u"org.apache.log4j.varia.LevelRangeFilter"_s, create_level_range_filter);
    mFilterRegistry.insert(u"Log4Qt::LevelRangeFilter"_s, create_level_range_filter);
    mFilterRegistry.insert(u"LevelRange"_s, create_level_range_filter);
    mFilterRegistry.insert(u"org.apache.log4j.varia.StringMatchFilter"_s, create_string_match_filter);
    mFilterRegistry.insert(u"Log4Qt::StringMatchFilter"_s, create_string_match_filter);
    mFilterRegistry.insert(u"StringMatch"_s, create_string_match_filter);
}


void Factory::registerDefaultLayouts()
{
    mLayoutRegistry.insert(u"org.apache.log4j.PatternLayout"_s, create_pattern_layout);
    mLayoutRegistry.insert(u"Log4Qt::PatternLayout"_s, create_pattern_layout);
    mLayoutRegistry.insert(u"PatternLayout"_s, create_pattern_layout);
    mLayoutRegistry.insert(u"org.apache.log4j.SimpleLayout"_s, create_simple_layout);
    mLayoutRegistry.insert(u"Log4Qt::SimpleLayout"_s, create_simple_layout);
    mLayoutRegistry.insert(u"SimpleLayout"_s, create_simple_layout);
    mLayoutRegistry.insert(u"org.apache.log4j.TTCCLayout"_s, create_ttcc_layout);
    mLayoutRegistry.insert(u"Log4Qt::TTCCLayout"_s, create_ttcc_layout);
    mLayoutRegistry.insert(u"TTCCLayout"_s, create_ttcc_layout);

    mLayoutRegistry.insert(u"org.apache.log4j.SimpleTimeLayout"_s, create_simple_time_layout);
    mLayoutRegistry.insert(u"Log4Qt::SimpleTimeLayout"_s, create_simple_time_layout);
    mLayoutRegistry.insert(u"SimpleTimeLayout"_s, create_simple_time_layout);

#ifdef LOG4QT_DB_LOGGING_SUPPORT
    mLayoutRegistry.insert(u"org.apache.log4j.DatabaseLayout"_s, create_database_layout);
    mLayoutRegistry.insert(u"Log4Qt::DatabaseLayout"_s, create_database_layout);
    mLayoutRegistry.insert(u"DatabaseLayout"_s, create_database_layout);
#endif

    mLayoutRegistry.insert(u"org.apache.log4j.XMLLayout"_s, create_xml_layout);
    mLayoutRegistry.insert(u"Log4Qt::XMLLayout"_s, create_xml_layout);
    mLayoutRegistry.insert(u"XMLLayout"_s, create_xml_layout);

    mLayoutRegistry.insert(u"Log4Qt::JsonLayout"_s, create_json_layout);
    mLayoutRegistry.insert(u"JsonLayout"_s, create_json_layout);
}


void Factory::registerDefaultTriggeringPolicies()
{
    mTriggeringPolicyRegistry.insert(u"Log4Qt::SizeBasedTriggeringPolicy"_s, create_size_based_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"SizeBasedTriggeringPolicy"_s, create_size_based_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"SizeBased"_s, create_size_based_triggering_policy);

    mTriggeringPolicyRegistry.insert(u"Log4Qt::TimeBasedTriggeringPolicy"_s, create_time_based_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"TimeBasedTriggeringPolicy"_s, create_time_based_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"TimeBased"_s, create_time_based_triggering_policy);

    mTriggeringPolicyRegistry.insert(u"Log4Qt::CronTriggeringPolicy"_s, create_cron_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"CronTriggeringPolicy"_s, create_cron_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"Cron"_s, create_cron_triggering_policy);

    mTriggeringPolicyRegistry.insert(u"Log4Qt::OnStartupTriggeringPolicy"_s, create_on_startup_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"OnStartupTriggeringPolicy"_s, create_on_startup_triggering_policy);
    mTriggeringPolicyRegistry.insert(u"OnStartup"_s, create_on_startup_triggering_policy);
}


void Factory::registerDefaultRolloverStrategies()
{
    mRolloverStrategyRegistry.insert(u"Log4Qt::DefaultRolloverStrategy"_s, create_default_rollover_strategy);
    mRolloverStrategyRegistry.insert(u"DefaultRolloverStrategy"_s, create_default_rollover_strategy);
    mRolloverStrategyRegistry.insert(u"Default"_s, create_default_rollover_strategy);

    mRolloverStrategyRegistry.insert(u"Log4Qt::DateRolloverStrategy"_s, create_date_rollover_strategy);
    mRolloverStrategyRegistry.insert(u"DateRolloverStrategy"_s, create_date_rollover_strategy);
    mRolloverStrategyRegistry.insert(u"Date"_s, create_date_rollover_strategy);
}


HeaderFooterProvider *Factory::doCreateHeaderFooterProvider(const QString &className)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mHeaderFooterProviderRegistry.contains(className))
    {
        logger()->warn(u"Request for the creation of HeaderFooterProvider with class '%1', which is not registered"_s, className);
        return nullptr;
    }
    return mHeaderFooterProviderRegistry.value(className)();
}


void Factory::doRegisterHeaderFooterProvider(const QString &className,
                                              HeaderFooterProviderFactoryFunc func)
{
    QMutexLocker locker(&mObjectGuard);

    if (className.isEmpty())
    {
        logger()->warn(u"Registering HeaderFooterProvider factory function with empty class name"_s);
        return;
    }
    mHeaderFooterProviderRegistry.insert(className, func);
}


void Factory::doUnregisterHeaderFooterProvider(const QString &className)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mHeaderFooterProviderRegistry.contains(className))
    {
        logger()->warn(u"Request to unregister not registered HeaderFooterProvider factory function for class '%1'"_s, className);
        return;
    }
    mHeaderFooterProviderRegistry.remove(className);
}


void Factory::registerDefaultHeaderFooterProviders()
{
    mHeaderFooterProviderRegistry.insert(u"Log4Qt::PatternHeaderFooterProvider"_s, create_pattern_header_footer_provider);
    mHeaderFooterProviderRegistry.insert(u"PatternHeaderFooterProvider"_s, create_pattern_header_footer_provider);
    mHeaderFooterProviderRegistry.insert(u"Pattern"_s, create_pattern_header_footer_provider);
}


bool Factory::validateObjectProperty(QMetaProperty &metaProperty,
                                     const QString &property,
                                     QObject *object)
{
    // Validate:
    // - No null object pointer
    // - No empty property name
    // - Property exists on the object (QT or Java name)
    // - Property is readable
    // - Property is writable

    const char *context = "Log4Qt::Factory";
    LogError e = LOG4QT_ERROR("Unable to set property value on object",
                              ConfiguratorPropertyError,
                              context);

    if (object == nullptr)
    {
        LogError ce = LOG4QT_ERROR("Invalid null object pointer",
                                   0,
                                   context);
        e.addCausingError(ce);
        logger()->error(e);
        return false;
    }
    if (property.isEmpty())
    {
        LogError ce = LOG4QT_ERROR("Invalid empty property name",
                                   0,
                                   context);
        e.addCausingError(ce);
        logger()->error(e);
        return false;
    }
    const QMetaObject *p_meta_object = object->metaObject();
    QString propertyString = property;
    int i = p_meta_object->indexOfProperty(propertyString.toLatin1().constData());
    if (i < 0)
    {
        // Try name with lower case first character. Java properties names
        // start upper case
        propertyString[0] = propertyString[0].toLower();
        i = p_meta_object->indexOfProperty(propertyString.toLatin1().constData());
        if (i < 0)
        {
            LogError ce = LOG4QT_ERROR("Property '%1' does not exist in class '%2'",
                                       0,
                                       context);
            ce << propertyString
               << QString::fromLatin1(object->metaObject()->className());
            e.addCausingError(ce);
            logger()->error(e);
            return false;
        }
    }
    metaProperty = p_meta_object->property(i);
    if (!metaProperty.isWritable())
    {
        LogError ce = LOG4QT_ERROR("Property '%1' is not writable in class '%2'",
                                   0,
                                   context);
        ce << property
           << QString::fromLatin1(object->metaObject()->className());
        e.addCausingError(ce);
        logger()->error(e);
        return false;
    }

    return true;
}

} // namespace Log4Qt

