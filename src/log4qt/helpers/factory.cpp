/******************************************************************************
 *
 * package:     Log4Qt
 * file:        factory.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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
#include "dailyrollingfileappender.h"
#include "fileappender.h"
#include "helpers/logerror.h"
#include "helpers/initialisationhelper.h"
#include "helpers/optionconverter.h"
#include "patternlayout.h"
#include "rollingfileappender.h"
#include "signalappender.h"
#include "simplelayout.h"
#include "simpletimelayout.h"
#include "ttcclayout.h"
#include "binarylayout.h"
#include "binarytotextlayout.h"
#include "xmllayout.h"

#if defined(QT_NETWORK_LIB)
#include "telnetappender.h"
#endif

#if defined(QT_SQL_LIB)
#include "databaseappender.h"
#include "databaselayout.h"
#endif //#ifdef QT_SQL_LIB

#include "asyncappender.h"
#include "mainthreadappender.h"
#include "systemlogappender.h"
#include "binaryfileappender.h"
#include "rollingbinaryfileappender.h"
#include "dailyfileappender.h"
#ifdef Q_OS_WIN
#include "colorconsoleappender.h"
#include "wdcappender.h"
#endif

#include "varia/debugappender.h"
#include "varia/denyallfilter.h"
#include "varia/levelmatchfilter.h"
#include "varia/levelrangefilter.h"
#include "varia/listappender.h"
#include "varia/nullappender.h"
#include "varia/stringmatchfilter.h"
#include "varia/binaryeventfilter.h"

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

Appender *create_daily_rolling_file_appender()
{
    return new DailyRollingFileAppender;
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

#if defined(QT_SQL_LIB)
Appender *create_database_appender()
{
    return new DatabaseAppender;
}
#endif //#if defined(QT_SQL_LIB)

#if defined(QT_NETWORK_LIB)
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

Appender *create_binaryfile_appender()
{
    return new BinaryFileAppender;
}

Appender *create_rollingbinaryfile_appender()
{
    return new RollingBinaryFileAppender;
}

Appender *create_dailyrollingfile_appender()
{
    return new DailyFileAppender;
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

Filter *create_binaryevent_filter()
{
    return new BinaryEventFilter;
}

// Layouts

Layout *create_pattern_layout()
{
    return new PatternLayout;
}

Layout *create_simple_layout()
{
    return new SimpleLayout;
}

Layout *create_simple_time_layout()
{
    return new SimpleTimeLayout;
}

#if defined(QT_SQL_LIB)
Layout *create_database_layout()
{
    return new DatabaseLayout;
}
#endif //#if defined(QT_SQL_LIB)

Layout *create_ttcc_layout()
{
    return new TTCCLayout;
}
Layout *create_binary_layout()
{
    return new BinaryLayout;
}

Layout *create_binarytotext_layout()
{
    return new BinaryToTextLayout;
}

Layout *create_xml_layout()
{
    return new XMLLayout;
}

Factory::Factory()
{
    registerDefaultAppenders();
    registerDefaultFilters();
    registerDefaultLayouts();
}

LOG4QT_IMPLEMENT_INSTANCE(Factory)

Appender *Factory::doCreateAppender(const QString &appenderClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mAppenderRegistry.contains(appenderClassName))
    {
        logger()->warn(QStringLiteral("Request for the creation of Appender with class '%1', which is not registered"), appenderClassName);
        return nullptr;
    }
    return mAppenderRegistry.value(appenderClassName)();
}


Filter *Factory::doCreateFilter(const QString &filterClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mFilterRegistry.contains(filterClassName))
    {
        logger()->warn(QStringLiteral("Request for the creation of Filter with class '%1', which is not registered"), filterClassName);
        return nullptr;
    }
    return mFilterRegistry.value(filterClassName)();
}


Layout *Factory::doCreateLayout(const QString &layoutClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mLayoutRegistry.contains(layoutClassName))
    {
        logger()->warn(QStringLiteral("Request for the creation of Layout with class '%1', which is not registered"), layoutClassName);
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
        logger()->warn(QStringLiteral("Registering Appender factory function with empty class name"));
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
        logger()->warn(QStringLiteral("Registering Filter factory function with empty class name"));
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
        logger()->warn(QStringLiteral("Registering Layout factory function with empty class name"));
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
    logger()->debug(QStringLiteral("Setting property '%1' on object of class '%2' to value '%3'"),
                    propertyString,
                    QLatin1String(object->metaObject()->className()),
                    value);

    QVariant variant;
    bool ok = true;
    if (type == QStringLiteral("bool"))
        variant = OptionConverter::toBoolean(value, &ok);
    else if (type == QStringLiteral("int"))
        variant = OptionConverter::toInt(value, &ok);
    else if (type == QStringLiteral("Log4Qt::Level"))
        variant = QVariant::fromValue(OptionConverter::toLevel(value, &ok));
    else if (type == QStringLiteral("QString"))
        variant = value;
    else
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Cannot convert to type '%1' for property '%2' on object of class '%3'"),
                                  CONFIGURATOR_UNKNOWN_TYPE_ERROR,
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
        logger()->warn(QStringLiteral("Unxpected error result from QMetaProperty.write()"));
}


void Factory::doUnregisterAppender(const QString &appenderClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mAppenderRegistry.contains(appenderClassName))
    {
        logger()->warn(QStringLiteral("Request to unregister not registered Appender factory function for class '%1'"), appenderClassName);
        return;
    }
    mAppenderRegistry.remove(appenderClassName);
}


void Factory::doUnregisterFilter(const QString &filterClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mFilterRegistry.contains(filterClassName))
    {
        logger()->warn(QStringLiteral("Request to unregister not registered Filter factory function for class '%1'"), filterClassName);
        return;
    }
    mFilterRegistry.remove(filterClassName);
}


void Factory::doUnregisterLayout(const QString &layoutClassName)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mLayoutRegistry.contains(layoutClassName))
    {
        logger()->warn(QStringLiteral("Request to unregister not registered Layout factory function for class '%1'"), layoutClassName);
        return;
    }
    mLayoutRegistry.remove(layoutClassName);
}


void Factory::registerDefaultAppenders()
{
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.ConsoleAppender"), console_file_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::ConsoleAppender"), console_file_appender);
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.DailyRollingFileAppender"), create_daily_rolling_file_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::DailyRollingFileAppender"), create_daily_rolling_file_appender);
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.varia.DebugAppender"), create_debug_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::DebugAppender"), create_debug_appender);
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.FileAppender"), create_file_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::FileAppender"), create_file_appender);
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.varia.ListAppender"), create_list_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::ListAppender"), create_list_appender);
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.varia.NullAppender"), create_null_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::NullAppender"), create_null_appender);
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.RollingFileAppender"), create_rolling_file_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::RollingFileAppender"), create_rolling_file_appender);

    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.SignalAppender"), create_signal_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::SignalAppender"), create_signal_appender);
#ifdef Q_OS_WIN
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.ColorConsoleAppender"), create_color_console_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::ColorConsoleAppender"), create_color_console_appender);
#endif

#if defined(QT_SQL_LIB)
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.DatabaseAppender"), create_database_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::DatabaseAppender"), create_database_appender);
#endif //#ifdef QT_SQL_LIB

#if defined(QT_NETWORK_LIB)
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.TelnetAppender"), create_telnet_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::TelnetAppender"), create_telnet_appender);
#endif
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.AsyncAppender"), create_async_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::AsyncAppender"), create_async_appender);

    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.MainThreadAppender"), create_mainthread_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::MainThreadAppender"), create_mainthread_appender);

    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.SystemLogAppender"), create_systemlog_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::SystemLogAppender"), create_systemlog_appender);

    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.BinaryFileAppender"), create_binaryfile_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::BinaryFileAppender"), create_binaryfile_appender);

    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.RollingBinaryFileAppender"), create_rollingbinaryfile_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::RollingBinaryFileAppender"), create_rollingbinaryfile_appender);

    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.DailyFileAppender"), create_dailyrollingfile_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::DailyFileAppender"), create_dailyrollingfile_appender);
#ifdef Q_OS_WIN
    mAppenderRegistry.insert(QStringLiteral("org.apache.log4j.WDCAppender"), create_wdc_appender);
    mAppenderRegistry.insert(QStringLiteral("Log4Qt::WDCAppender"), create_wdc_appender);
#endif
}


void Factory::registerDefaultFilters()
{
    mFilterRegistry.insert(QStringLiteral("org.apache.log4j.varia.DenyAllFilter"), create_deny_all_filter);
    mFilterRegistry.insert(QStringLiteral("Log4Qt::DenyAllFilter"), create_deny_all_filter);
    mFilterRegistry.insert(QStringLiteral("org.apache.log4j.varia.LevelMatchFilter"), create_level_match_filter);
    mFilterRegistry.insert(QStringLiteral("Log4Qt::LevelMatchFilter"), create_level_match_filter);
    mFilterRegistry.insert(QStringLiteral("org.apache.log4j.varia.LevelRangeFilter"), create_level_range_filter);
    mFilterRegistry.insert(QStringLiteral("Log4Qt::LevelRangeFilter"), create_level_range_filter);
    mFilterRegistry.insert(QStringLiteral("org.apache.log4j.varia.StringMatchFilter"), create_string_match_filter);
    mFilterRegistry.insert(QStringLiteral("Log4Qt::StringMatchFilter"), create_string_match_filter);
    mFilterRegistry.insert(QStringLiteral("org.apache.log4j.varia.BinaryEventFilter"), create_binaryevent_filter);
    mFilterRegistry.insert(QStringLiteral("Log4Qt::BinaryEventFilter"), create_binaryevent_filter);
}


void Factory::registerDefaultLayouts()
{
    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.PatternLayout"), create_pattern_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::PatternLayout"), create_pattern_layout);
    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.SimpleLayout"), create_simple_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::SimpleLayout"), create_simple_layout);
    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.TTCCLayout"), create_ttcc_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::TTCCLayout"), create_ttcc_layout);

    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.SimpleTimeLayout"), create_simple_time_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::SimpleTimeLayout"), create_simple_time_layout);

#if defined(QT_SQL_LIB)
    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.DatabaseLayout"), create_database_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::DatabaseLayout"), create_database_layout);
#endif //#ifdef (QT_SQL_LIB)

    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.BinaryLayout"), create_binary_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::BinaryLayout"), create_binary_layout);

    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.BinaryToTextLayout"), create_binarytotext_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::BinaryToTextLayout"), create_binarytotext_layout);

    mLayoutRegistry.insert(QStringLiteral("org.apache.log4j.XMLLayout"), create_xml_layout);
    mLayoutRegistry.insert(QStringLiteral("Log4Qt::XMLLayout"), create_xml_layout);
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
    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to set property value on object"),
                              CONFIGURATOR_PROPERTY_ERROR,
                              context);

    if (object == nullptr)
    {
        LogError ce = LOG4QT_ERROR(QT_TR_NOOP("Invalid null object pointer"),
                                   0,
                                   context);
        e.addCausingError(ce);
        logger()->error(e);
        return false;
    }
    if (property.isEmpty())
    {
        LogError ce = LOG4QT_ERROR(QT_TR_NOOP("Invalid empty property name"),
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
            LogError ce = LOG4QT_ERROR(QT_TR_NOOP("Property '%1' does not exist in class '%2'"),
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
        LogError ce = LOG4QT_ERROR(QT_TR_NOOP("Property '%1' is not writable in class '%2'"),
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

