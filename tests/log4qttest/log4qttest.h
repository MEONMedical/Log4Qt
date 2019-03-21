/******************************************************************************
 *
 * package:     Log4Qt
 * file:        loggingtest.h
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

#ifndef LOG4QT_LOG4QTTEST_H
#define LOG4QT_LOG4QTTEST_H

#include <QDir>
#include <QObject>

#include "log4qt/helpers/properties.h"
#include "log4qt/logger.h"
#include "log4qt/varia/listappender.h"

/*!
 * \brief The class Log4QtTest provides a unit test for the package Log4Qt.
 *
 * The class Log4QtTest implements a unit test based on th Qt testing framework.
 */
class Log4QtTest : public QObject
{
    Q_OBJECT

public:
    explicit Log4QtTest(QObject *parent = nullptr);
    virtual ~Log4QtTest();
private:
    Log4QtTest(const Log4QtTest &other); // Not implemented
    Log4QtTest &operator=(const Log4QtTest &other); // Not implemented

private slots:
    void initTestCase();
    void cleanupTestCase();

    // log4qt
    void DateTime_alternativelyFormat_data();
    void DateTime_alternativelyFormat();

    void DateTime_milliseconds_data();
    void DateTime_milliseconds();
    void PatternFormatter_data();
    void PatternFormatter();
    void Properties_default_data();
    void Properties_default();
    void Properties_names();
    void Properties_load_device_data();
    void Properties_load_device();
    void Properties_load_settings();

    // OptionConverter requires Properties
    void OptionConverter_boolean_data();
    void OptionConverter_boolean();
    void OptionConverter_filesize_data();
    void OptionConverter_filesize();
    void OptionConverter_int_data();
    void OptionConverter_int();
    void OptionConverter_level_data();
    void OptionConverter_level();
    void OptionConverter_substitution_data();
    void OptionConverter_substitution();
    void OptionConverter_target_data();
    void OptionConverter_target();

    // Factory requires OptionConverter
    void Factory_createAppender_data();
    void Factory_createAppender();
    void Factory_createFilter_data();
    void Factory_createFilter();
    void Factory_createLayout_data();
    void Factory_createLayout();
    void Factory_setObjectProperty_data();
    void Factory_setObjectProperty();

    // log4qt/varia
    void ListAppender();
    void DenyAllFilter();
    void LevelMatchFilter_data();
    void LevelMatchFilter();
    void LevelRangeFilter_data();
    void LevelRangeFilter();
    void StringMatchFilter_data();
    void StringMatchFilter();

    // log4qt
    void AppenderSkeleton_threshold();
    void AppenderSkeleton_filter_data();
    void AppenderSkeleton_filter();
    void BasicConfigurator();
    void FileAppender();
    void DailyRollingFileAppender();
    void LoggingEvent_stream_data();
    void LoggingEvent_stream();
    void LogManager_configureLogLogger();
    void PropertyConfigurator_missing_appender();
    void PropertyConfigurator_unknown_appender_class();
    void PropertyConfigurator_missing_layout();
    void PropertyConfigurator_unknown_layout_class();
    void PropertyConfigurator_reset();
    void PropertyConfigurator_debug();
    void PropertyConfigurator_threshold();
    void PropertyConfigurator_handleQtMessages();
    void PropertyConfigurator_example();
    void RollingFileAppender();

private:
    QString dailyRollingFileAppenderSuffix(const QDateTime &dateTime);
    QString enumValueToKey(QObject *pObject,
                           const char *pEnumeration,
                           int value);
    void resetLogging();
    static bool compareStringLists(const QStringList &actual,
                                   const QStringList &expected,
                                   const QString &entry,
                                   const QString &entries,
                                   QString &result);
    static bool deleteDirectoryTree(const QString &name);
    static bool validateDirContents(const QString &name,
                                    const QStringList &expected,
                                    QString &result);
    static bool validateFileContents(const QString &name,
                                     const QStringList &expected,
                                     QString &result);

private:
    bool mSkipLongTests;
    QDir mTemporaryDirectory;
    Log4Qt::AppenderSharedPtr mpLoggingEvents;
    Log4Qt::Properties mDefaultProperties;
    Log4Qt::Properties mProperties;
    Log4Qt::ListAppender *loggingEvents() const;
};


// Q_DECLARE_TYPEINFO(Log4Qt::Log4QtTest, Q_COMPLEX_TYPE); // use default


#endif // LOG4QT_LOG4QTTEST_H
