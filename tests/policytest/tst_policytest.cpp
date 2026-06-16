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

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "log4qt/helpers/cronexpression.h"
#include "log4qt/helpers/datetime.h"
#include "log4qt/helpers/factory.h"
#include "log4qt/helpers/properties.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logmanager.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/rollingfileappender.h"
#include "log4qt/simplelayout.h"
#include "log4qt/spi/compositetriggeringpolicy.h"
#include "log4qt/spi/crontriggeringpolicy.h"
#include "log4qt/spi/daterolloverstrategy.h"
#include "log4qt/spi/defaultrolloverstrategy.h"
#include "log4qt/spi/onstartuptriggeringpolicy.h"
#include "log4qt/spi/sizebasedtriggeringpolicy.h"
#include "log4qt/spi/timebasedtriggeringpolicy.h"

using namespace Log4Qt;

// Exposes rollOver() so tests can trigger a rollover directly without
// routing through a triggering policy.
class RollableFileAppender : public RollingFileAppender
{
public:
    using RollingFileAppender::RollingFileAppender;
    void triggerRollover() { rollOver(); }
};

class PolicyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();

    // CronExpression
    void CronExpression_parse_data();
    void CronExpression_parse();
    void CronExpression_nextFireTime_data();
    void CronExpression_nextFireTime();
    void CronExpression_dayOfWeek();
    void CronExpression_noMatchWithin4Years();

    // SizeBasedTriggeringPolicy
    void SizeBasedTriggeringPolicy_defaults();
    void SizeBasedTriggeringPolicy_trigger_data();
    void SizeBasedTriggeringPolicy_trigger();
    void SizeBasedTriggeringPolicy_maxFileSize_string();

    // TimeBasedTriggeringPolicy
    void TimeBasedTriggeringPolicy_defaults();
    void TimeBasedTriggeringPolicy_frequency_data();
    void TimeBasedTriggeringPolicy_frequency();
    void TimeBasedTriggeringPolicy_invalidPattern();
    void TimeBasedTriggeringPolicy_intervalDefaults();
    void TimeBasedTriggeringPolicy_intervalClamping();
    void TimeBasedTriggeringPolicy_modulateAlignment();
    void TimeBasedTriggeringPolicy_maxRandomDelayNonNegative();
    void TimeBasedTriggeringPolicy_propertyConfigurator();

    // CronTriggeringPolicy
    void CronTriggeringPolicy_defaults();
    void CronTriggeringPolicy_invalidSchedule();
    void CronTriggeringPolicy_activateAndTrigger();

    // OnStartupTriggeringPolicy
    void OnStartupTriggeringPolicy_isTriggeringEvent();
    void OnStartupTriggeringPolicy_isStartupTrigger_data();
    void OnStartupTriggeringPolicy_isStartupTrigger();

    // CompositeTriggeringPolicy
    void CompositeTriggeringPolicy_empty();
    void CompositeTriggeringPolicy_orCombination();
    void CompositeTriggeringPolicy_startupTrigger();

    // DateRolloverStrategy
    void DateRolloverStrategy_defaults();
    void DateRolloverStrategy_suffixMode();
    void DateRolloverStrategy_embeddedMode();
    void DateRolloverStrategy_maxBackups();
    void DateRolloverStrategy_suffixPropertyConfigurator();
    void DateRolloverStrategy_embeddedPropertyConfigurator();
    void DateRolloverStrategy_datedActiveFile_defaults();
    void DateRolloverStrategy_initialFileName_offByDefault();
    void DateRolloverStrategy_initialFileName_datedEmbedded();
    void DateRolloverStrategy_initialFileName_datedSuffix();
    void DateRolloverStrategy_datedActiveFile_rolloverNoRename();
    void DateRolloverStrategy_datedActiveFilePropertyConfigurator();

    // DefaultRolloverStrategy
    void DefaultRolloverStrategy_defaults();
    void DefaultRolloverStrategy_rollover();
    void DefaultRolloverStrategy_noExistingFile();
    void DefaultRolloverStrategy_singleBackup();
    void DefaultRolloverStrategy_initialFileNameUnchanged();

    // RollingFileAppender integration
    void RollingFileAppender_initialFileName_appliedOnStartup();
    void RollingFileAppender_rolloverUsesBaseFileName();
    void RollingFileAppender_defaultStrategyUnchangedOnStartup();

    // Factory
    void Factory_createTriggeringPolicy_data();
    void Factory_createTriggeringPolicy();
    void Factory_createRolloverStrategy_data();
    void Factory_createRolloverStrategy();

    // PropertyConfigurator integration
    void PropertyConfigurator_cronPolicy();
};

void PolicyTest::cleanup()
{
    LogManager::resetConfiguration();
    DateTime::setProvider(nullptr);
}

// ---------------------------------------------------------------------------
// CronExpression
// ---------------------------------------------------------------------------

void PolicyTest::CronExpression_parse_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid-midnight-daily")   << "0 0 0 * * ?"      << true;
    QTest::newRow("valid-every-minute")     << "0 * * * * ?"       << true;
    QTest::newRow("valid-every-second")     << "* * * * * ?"       << true;
    QTest::newRow("valid-range")            << "0 0 9-17 * * ?"    << true;
    QTest::newRow("valid-list")             << "0 0,15,30,45 * * * ?" << true;
    QTest::newRow("valid-step")             << "0 0 0/4 * * ?"     << true;
    QTest::newRow("valid-month-name")       << "0 0 0 1 JAN ?"     << true;
    QTest::newRow("valid-dow-name")         << "0 0 8 ? * MON-FRI" << true;

    QTest::newRow("invalid-empty")          << ""                  << false;
    QTest::newRow("invalid-5-fields")       << "0 0 0 * *"         << false;
    QTest::newRow("invalid-7-fields")       << "0 0 0 * * ? 2026"  << false;
    QTest::newRow("invalid-seconds-60")     << "60 0 0 * * ?"      << false;
    QTest::newRow("invalid-hours-24")       << "0 0 24 * * ?"      << false;
    QTest::newRow("invalid-dom-32")         << "0 0 0 32 * ?"      << false;
    QTest::newRow("invalid-month-13")       << "0 0 0 * 13 ?"      << false;
    QTest::newRow("invalid-dow-8")          << "0 0 0 * * 8"       << false;
    QTest::newRow("invalid-step-zero")      << "0 0 0/0 * * ?"     << false;
    QTest::newRow("invalid-inverted-range") << "0 0 5-1 * * ?"     << false;
}

void PolicyTest::CronExpression_parse()
{
    QFETCH(QString, expression);
    QFETCH(bool, valid);

    Log4Qt::CronExpression cron(expression);
    QCOMPARE(cron.isValid(), valid);
    if (!valid)
        QVERIFY(!cron.errorString().isEmpty());
}

void PolicyTest::CronExpression_nextFireTime_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QDateTime>("from");
    QTest::addColumn<QDateTime>("expected");

    // midnight daily: from 10:00 -> next day 00:00
    QTest::newRow("midnight-daily")
        << "0 0 0 * * ?"
        << QDateTime(QDate(2026, 3, 28), QTime(10, 0, 0))
        << QDateTime(QDate(2026, 3, 29), QTime(0, 0, 0));

    // half-past every hour: from 10:15 -> 10:30
    QTest::newRow("half-past-hour")
        << "0 30 * * * ?"
        << QDateTime(QDate(2026, 3, 28), QTime(10, 15, 0))
        << QDateTime(QDate(2026, 3, 28), QTime(10, 30, 0));

    // first of month: from Mar 15 -> Apr 1
    QTest::newRow("first-of-month")
        << "0 0 0 1 * ?"
        << QDateTime(QDate(2026, 3, 15), QTime(0, 0, 0))
        << QDateTime(QDate(2026, 4, 1), QTime(0, 0, 0));

    // every 15 seconds: from 10:00:07 -> 10:00:15
    QTest::newRow("every-15-seconds")
        << "*/15 * * * * ?"
        << QDateTime(QDate(2026, 3, 28), QTime(10, 0, 7))
        << QDateTime(QDate(2026, 3, 28), QTime(10, 0, 15));

    // noon and midnight: from 10:00 -> 12:00
    QTest::newRow("noon-and-midnight")
        << "0 0 0,12 * * ?"
        << QDateTime(QDate(2026, 3, 28), QTime(10, 0, 0))
        << QDateTime(QDate(2026, 3, 28), QTime(12, 0, 0));

    // January only: from March -> next January
    QTest::newRow("january-only")
        << "0 0 0 * JAN ?"
        << QDateTime(QDate(2026, 3, 1), QTime(0, 0, 0))
        << QDateTime(QDate(2027, 1, 1), QTime(0, 0, 0));
}

void PolicyTest::CronExpression_nextFireTime()
{
    QFETCH(QString, expression);
    QFETCH(QDateTime, from);
    QFETCH(QDateTime, expected);

    Log4Qt::CronExpression cron(expression);
    QVERIFY(cron.isValid());

    QDateTime result = cron.nextFireTime(from);
    QVERIFY(result.isValid());
    QCOMPARE(result, expected);
}

void PolicyTest::CronExpression_dayOfWeek()
{
    // 2026-03-28 is a Saturday
    // MON schedule should fire on 2026-03-30 (Monday)
    Log4Qt::CronExpression cron("0 0 8 ? * MON");
    QVERIFY(cron.isValid());

    QDateTime from(QDate(2026, 3, 28), QTime(9, 0, 0));
    QDateTime result = cron.nextFireTime(from);
    QCOMPARE(result, QDateTime(QDate(2026, 3, 30), QTime(8, 0, 0)));
    QCOMPARE(result.date().dayOfWeek(), 1); // Qt: 1=Monday

    // SUN schedule: from Saturday -> Sunday
    Log4Qt::CronExpression cronSun("0 0 0 ? * SUN");
    QVERIFY(cronSun.isValid());
    QDateTime resultSun = cronSun.nextFireTime(from);
    QCOMPARE(resultSun, QDateTime(QDate(2026, 3, 29), QTime(0, 0, 0)));
    QCOMPARE(resultSun.date().dayOfWeek(), 7); // Qt: 7=Sunday

    // SAT schedule: from Saturday 9:00 -> next Saturday (since 8:00 already passed)
    Log4Qt::CronExpression cronSat("0 0 8 ? * SAT");
    QVERIFY(cronSat.isValid());
    QDateTime resultSat = cronSat.nextFireTime(from);
    QCOMPARE(resultSat, QDateTime(QDate(2026, 4, 4), QTime(8, 0, 0)));
}

void PolicyTest::CronExpression_noMatchWithin4Years()
{
    // Day 31 of February will never match
    Log4Qt::CronExpression cron("0 0 0 31 2 ?");
    QVERIFY(cron.isValid()); // parses fine, but no date ever matches

    QDateTime from(QDate(2026, 1, 1), QTime(0, 0, 0));
    QDateTime result = cron.nextFireTime(from);
    QVERIFY(!result.isValid());
}

// ---------------------------------------------------------------------------
// SizeBasedTriggeringPolicy
// ---------------------------------------------------------------------------

void PolicyTest::SizeBasedTriggeringPolicy_defaults()
{
    Log4Qt::SizeBasedTriggeringPolicy policy;
    QCOMPARE(policy.maximumFileSize(), static_cast<qint64>(10 * 1024 * 1024));
}

void PolicyTest::SizeBasedTriggeringPolicy_trigger_data()
{
    QTest::addColumn<qint64>("fileSize");
    QTest::addColumn<qint64>("maxSize");
    QTest::addColumn<bool>("expected");

    QTest::newRow("below-limit")    << qint64(100) << qint64(200)  << false;
    QTest::newRow("at-limit")       << qint64(200) << qint64(200)  << false; // uses >
    QTest::newRow("one-over")       << qint64(201) << qint64(200)  << true;
    QTest::newRow("zero-size")      << qint64(0)   << qint64(200)  << false;
    QTest::newRow("large-file")     << qint64(1024 * 1024 * 100) << qint64(1024 * 1024) << true;
}

void PolicyTest::SizeBasedTriggeringPolicy_trigger()
{
    QFETCH(qint64, fileSize);
    QFETCH(qint64, maxSize);
    QFETCH(bool, expected);

    Log4Qt::SizeBasedTriggeringPolicy policy;
    policy.setMaximumFileSize(maxSize);

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    buffer.buffer().resize(fileSize);
    buffer.seek(fileSize);

    LoggingEvent event;
    QCOMPARE(policy.isTriggeringEvent(&buffer, event), expected);
}

void PolicyTest::SizeBasedTriggeringPolicy_maxFileSize_string()
{
    Log4Qt::SizeBasedTriggeringPolicy policy;

    policy.setMaxFileSize("5MB");
    QCOMPARE(policy.maximumFileSize(), static_cast<qint64>(5 * 1024 * 1024));

    policy.setMaxFileSize("1KB");
    QCOMPARE(policy.maximumFileSize(), static_cast<qint64>(1024));

    // Invalid string should not change value
    qint64 before = policy.maximumFileSize();
    policy.setMaxFileSize("notanumber");
    QCOMPARE(policy.maximumFileSize(), before);
}

// ---------------------------------------------------------------------------
// TimeBasedTriggeringPolicy
// ---------------------------------------------------------------------------

void PolicyTest::TimeBasedTriggeringPolicy_defaults()
{
    Log4Qt::TimeBasedTriggeringPolicy policy;
    QCOMPARE(policy.datePattern(), QString("'.'yyyy-MM-dd"));
    QCOMPARE(policy.frequency(), Log4Qt::TimeBasedTriggeringPolicy::Frequency::Daily);
}

void PolicyTest::TimeBasedTriggeringPolicy_frequency_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("expectedFrequency");

    QTest::newRow("minutely-mm")  << "'.'mm"         << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Minutely);
    QTest::newRow("hourly-HH")   << "'.'HH"         << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Hourly);
    QTest::newRow("halfdaily-a")  << "'.'a"          << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::HalfDaily);
    QTest::newRow("daily-dd")     << "'.'yyyy-MM-dd" << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Daily);
    QTest::newRow("monthly-MM")   << "'.'yyyy-MM"    << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Monthly);
    QTest::newRow("weekly-ww")    << "'.'yyyy-ww"    << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Weekly);
    QTest::newRow("hourly-h")     << "'.'h"           << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Hourly);
    QTest::newRow("halfdaily-AP") << "'.'AP"          << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::HalfDaily);
    QTest::newRow("quoted-mm")    << "'mm'.yyyy-MM"   << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Monthly);
    QTest::newRow("dayname-ddd")  << "'.'ddd"         << static_cast<int>(TimeBasedTriggeringPolicy::Frequency::Daily);
}

void PolicyTest::TimeBasedTriggeringPolicy_frequency()
{
    QFETCH(QString, pattern);
    QFETCH(int, expectedFrequency);

    Log4Qt::TimeBasedTriggeringPolicy policy;
    policy.setDatePattern(pattern);
    policy.activateOptions();

    QCOMPARE(static_cast<int>(policy.frequency()), expectedFrequency);
}

void PolicyTest::TimeBasedTriggeringPolicy_invalidPattern()
{
    Log4Qt::TimeBasedTriggeringPolicy policy;
    policy.setDatePattern("'constant-no-change'");
    policy.activateOptions();

    // Pattern that doesn't change at any frequency -> always returns false
    LoggingEvent event;
    QCOMPARE(policy.isTriggeringEvent(nullptr, event), false);
}

void PolicyTest::TimeBasedTriggeringPolicy_intervalDefaults()
{
    Log4Qt::TimeBasedTriggeringPolicy policy;
    QCOMPARE(policy.interval(), 1);
    QCOMPARE(policy.modulate(), false);
    QCOMPARE(policy.maxRandomDelay(), 0);
}

void PolicyTest::TimeBasedTriggeringPolicy_intervalClamping()
{
    Log4Qt::TimeBasedTriggeringPolicy policy;

    // interval=0 should be clamped to 1
    policy.setInterval(0);
    policy.activateOptions();
    QCOMPARE(policy.interval(), 1);

    // interval=-5 should be clamped to 1
    policy.setInterval(-5);
    policy.activateOptions();
    QCOMPARE(policy.interval(), 1);

    // interval=4 should remain 4
    policy.setInterval(4);
    policy.activateOptions();
    QCOMPARE(policy.interval(), 4);
}

void PolicyTest::TimeBasedTriggeringPolicy_modulateAlignment()
{
    // Minutely pattern with interval=5, modulate=true
    // After activation, the policy should not trigger immediately
    Log4Qt::TimeBasedTriggeringPolicy policy;
    policy.setDatePattern("'.'mm");
    policy.setInterval(5);
    policy.setModulate(true);
    policy.activateOptions();

    QCOMPARE(policy.frequency(), Log4Qt::TimeBasedTriggeringPolicy::Frequency::Minutely);

    // Should not trigger immediately since rollover time is in the future
    LoggingEvent event;
    QCOMPARE(policy.isTriggeringEvent(nullptr, event), false);
}

void PolicyTest::TimeBasedTriggeringPolicy_maxRandomDelayNonNegative()
{
    Log4Qt::TimeBasedTriggeringPolicy policy;

    // Negative delay should be clamped to 0
    policy.setMaxRandomDelay(-10);
    policy.activateOptions();
    QCOMPARE(policy.maxRandomDelay(), 0);

    // Zero remains zero
    policy.setMaxRandomDelay(0);
    policy.activateOptions();
    QCOMPARE(policy.maxRandomDelay(), 0);

    // Positive value remains unchanged
    policy.setMaxRandomDelay(60);
    policy.activateOptions();
    QCOMPARE(policy.maxRandomDelay(), 60);
}

void PolicyTest::TimeBasedTriggeringPolicy_propertyConfigurator()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString file = tempDir.path() + "/timebased.log";

    Properties props;
    props.setProperty("appender.R.type", "RollingFile");
    props.setProperty("appender.R.file", file);
    props.setProperty("appender.R.layout.type", "SimpleLayout");
    props.setProperty("appender.R.policy.TIME.type", "TimeBasedTriggeringPolicy");
    props.setProperty("appender.R.policy.TIME.datePattern", "'.'HH");
    props.setProperty("appender.R.policy.TIME.interval", "4");
    props.setProperty("appender.R.policy.TIME.modulate", "true");
    props.setProperty("appender.R.policy.TIME.maxRandomDelay", "30");
    props.setProperty("appender.R.strategy.type", "DefaultRolloverStrategy");
    props.setProperty("rootLogger.level", "DEBUG");
    props.setProperty("rootLogger.appenderRef.0.ref", "R");

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->appenders().count(), 1);

    auto *appender = qobject_cast<RollingFileAppender *>(root->appenders().first().data());
    QVERIFY(appender != nullptr);
    QVERIFY(appender->triggeringPolicy() != nullptr);

    auto *timePolicy = qobject_cast<TimeBasedTriggeringPolicy *>(appender->triggeringPolicy().data());
    QVERIFY(timePolicy != nullptr);
    QCOMPARE(timePolicy->datePattern(), QString("'.'HH"));
    QCOMPARE(timePolicy->interval(), 4);
    QCOMPARE(timePolicy->modulate(), true);
    QCOMPARE(timePolicy->maxRandomDelay(), 30);
    QCOMPARE(timePolicy->frequency(), Log4Qt::TimeBasedTriggeringPolicy::Frequency::Hourly);
}

// ---------------------------------------------------------------------------
// CronTriggeringPolicy
// ---------------------------------------------------------------------------

void PolicyTest::CronTriggeringPolicy_defaults()
{
    Log4Qt::CronTriggeringPolicy policy;
    QCOMPARE(policy.schedule(), QString("0 0 0 * * ?"));
}

void PolicyTest::CronTriggeringPolicy_invalidSchedule()
{
    Log4Qt::CronTriggeringPolicy policy;
    policy.setSchedule("invalid cron");
    policy.activateOptions();

    LoggingEvent event;
    QCOMPARE(policy.isTriggeringEvent(nullptr, event), false);
}

void PolicyTest::CronTriggeringPolicy_activateAndTrigger()
{
    Log4Qt::CronTriggeringPolicy policy;
    // Fire every second
    policy.setSchedule("* * * * * ?");
    policy.activateOptions();

    // Wait just over 1 second for the next fire time to pass
    QTest::qWait(1100);

    LoggingEvent event;
    QCOMPARE(policy.isTriggeringEvent(nullptr, event), true);

    // After triggering, next fire time is recomputed; immediate re-check should be false
    // (unless another second boundary was crossed during the call)
}

// ---------------------------------------------------------------------------
// OnStartupTriggeringPolicy
// ---------------------------------------------------------------------------

void PolicyTest::OnStartupTriggeringPolicy_isTriggeringEvent()
{
    Log4Qt::OnStartupTriggeringPolicy policy;
    LoggingEvent event;

    // Always returns false regardless of parameters
    QCOMPARE(policy.isTriggeringEvent(nullptr, event), false);
}

void PolicyTest::OnStartupTriggeringPolicy_isStartupTrigger_data()
{
    QTest::addColumn<bool>("createFile");
    QTest::addColumn<bool>("writeContent");
    QTest::addColumn<bool>("expected");

    QTest::newRow("file-with-content")  << true  << true  << true;
    QTest::newRow("empty-file")         << true  << false << false;
    QTest::newRow("no-file")            << false << false << false;
}

void PolicyTest::OnStartupTriggeringPolicy_isStartupTrigger()
{
    QFETCH(bool, createFile);
    QFETCH(bool, writeContent);
    QFETCH(bool, expected);

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.path() + "/test.log";

    if (createFile)
    {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        if (writeContent)
            file.write("some log content\n");
        file.close();
    }

    Log4Qt::OnStartupTriggeringPolicy policy;
    QCOMPARE(policy.isStartupTrigger(filePath, 0), expected);
}

// ---------------------------------------------------------------------------
// CompositeTriggeringPolicy
// ---------------------------------------------------------------------------

void PolicyTest::CompositeTriggeringPolicy_empty()
{
    Log4Qt::CompositeTriggeringPolicy policy;
    LoggingEvent event;

    QCOMPARE(policy.isTriggeringEvent(nullptr, event), false);
    QCOMPARE(policy.isStartupTrigger(QString(), 0), false);
    QCOMPARE(policy.policies().count(), 0);
}

void PolicyTest::CompositeTriggeringPolicy_orCombination()
{
    Log4Qt::CompositeTriggeringPolicy composite;

    // Policy 1: triggers above 100 bytes
    auto policy1 = TriggeringPolicySharedPtr(new Log4Qt::SizeBasedTriggeringPolicy);
    qobject_cast<Log4Qt::SizeBasedTriggeringPolicy *>(policy1.data())->setMaximumFileSize(100);

    // Policy 2: triggers above 500 bytes
    auto policy2 = TriggeringPolicySharedPtr(new Log4Qt::SizeBasedTriggeringPolicy);
    qobject_cast<Log4Qt::SizeBasedTriggeringPolicy *>(policy2.data())->setMaximumFileSize(500);

    composite.addPolicy(policy1);
    composite.addPolicy(policy2);

    QCOMPARE(composite.policies().count(), 2);

    LoggingEvent event;

    auto makeBuffer = [](qint64 pos) {
        auto buf = std::make_unique<QBuffer>();
        buf->open(QIODevice::ReadWrite);
        buf->buffer().resize(pos);
        buf->seek(pos);
        return buf;
    };

    // 50 bytes: neither triggers
    auto buf50 = makeBuffer(50);
    QCOMPARE(composite.isTriggeringEvent(buf50.get(), event), false);

    // 200 bytes: policy1 triggers (> 100), policy2 doesn't
    auto buf200 = makeBuffer(200);
    QCOMPARE(composite.isTriggeringEvent(buf200.get(), event), true);

    // 600 bytes: both trigger
    auto buf600 = makeBuffer(600);
    QCOMPARE(composite.isTriggeringEvent(buf600.get(), event), true);
}

void PolicyTest::CompositeTriggeringPolicy_startupTrigger()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString filePath = tempDir.path() + "/test.log";
    {
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("content");
        file.close();
    }

    Log4Qt::CompositeTriggeringPolicy composite;

    // OnStartup policy should trigger for existing file with content
    auto startupPolicy = TriggeringPolicySharedPtr(new Log4Qt::OnStartupTriggeringPolicy);
    composite.addPolicy(startupPolicy);

    QCOMPARE(composite.isStartupTrigger(filePath, 0), true);

    // Without OnStartup, size-based alone won't trigger startup
    Log4Qt::CompositeTriggeringPolicy composite2;
    auto sizePolicy = TriggeringPolicySharedPtr(new Log4Qt::SizeBasedTriggeringPolicy);
    composite2.addPolicy(sizePolicy);
    QCOMPARE(composite2.isStartupTrigger(filePath, 0), false);
}

// ---------------------------------------------------------------------------
// DateRolloverStrategy
// ---------------------------------------------------------------------------

void PolicyTest::DateRolloverStrategy_defaults()
{
    Log4Qt::DateRolloverStrategy strategy;
    QCOMPARE(strategy.datePattern(), QString("'.'yyyy-MM-dd"));
    QCOMPARE(strategy.mode(), Log4Qt::DateRolloverStrategy::NamingMode::Suffix);
    QCOMPARE(strategy.modeString(), QString("Suffix"));
    QCOMPARE(strategy.maxBackups(), 0);
}

void PolicyTest::DateRolloverStrategy_suffixMode()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString basePath = tempDir.path() + "/app.log";

    // Create the base file
    {
        QFile f(basePath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("log content");
        f.close();
    }

    Log4Qt::DateRolloverStrategy strategy;
    strategy.setDatePattern("'.'yyyy-MM-dd");
    strategy.setMode(Log4Qt::DateRolloverStrategy::NamingMode::Suffix);

    QString result = strategy.rollover(basePath);

    // Should return the same filename (appender reopens it)
    QCOMPARE(result, basePath);

    // Base file should have been renamed to basePath + date suffix
    QVERIFY(!QFile::exists(basePath));

    // A backup file with today's date suffix should exist
    QString expectedBackup = basePath + QDateTime::currentDateTime().toString("'.'yyyy-MM-dd");
    QVERIFY(QFile::exists(expectedBackup));
}

void PolicyTest::DateRolloverStrategy_embeddedMode()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString basePath = tempDir.path() + "/app.log";

    Log4Qt::DateRolloverStrategy strategy;
    strategy.setDatePattern("_yyyy-MM-dd");
    strategy.setModeString("Embedded");

    QCOMPARE(strategy.mode(), Log4Qt::DateRolloverStrategy::NamingMode::Embedded);

    QString result = strategy.rollover(basePath);

    // Should return a new filename with date embedded
    QString expectedName = tempDir.path() + "/app_"
        + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
    QCOMPARE(result, expectedName);

    // No rename happened — base file was not touched
    QVERIFY(!QFile::exists(basePath));
    QVERIFY(!QFile::exists(result)); // file doesn't exist yet (appender creates it)
}

void PolicyTest::DateRolloverStrategy_maxBackups()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString basePath = tempDir.path() + "/app.log";

    auto writeFile = [](const QString &path)
    {
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write("data");
        f.close();
    };

    // Create 5 backup files with date suffixes
    writeFile(basePath + ".2026-03-24");
    writeFile(basePath + ".2026-03-25");
    writeFile(basePath + ".2026-03-26");
    writeFile(basePath + ".2026-03-27");
    writeFile(basePath + ".2026-03-28");

    // Create the active file
    writeFile(basePath);

    {
        Log4Qt::DateRolloverStrategy strategy;
        strategy.setDatePattern("'.'yyyy-MM-dd");
        strategy.setMaxBackups(3);

        strategy.rollover(basePath);
    } // destructor waits for async cleanup to complete

    // Count remaining backup files (excluding the new one just created)
    QDir dir(tempDir.path());
    QFileInfoList entries = dir.entryInfoList({"app.log*"}, QDir::Files);

    // Should have at most maxBackups + 1 (the newly created backup)
    // The 2 oldest should have been deleted
    QVERIFY(entries.size() <= 4); // 3 kept + 1 new backup
}

void PolicyTest::DateRolloverStrategy_suffixPropertyConfigurator()
{
    // Replaces DailyRollingFileAppender: RollingFile + TimeBased + Date(Suffix)
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString file = tempDir.path() + "/app.log";

    Properties props;
    props.setProperty("appender.R.type", "RollingFile");
    props.setProperty("appender.R.file", file);
    props.setProperty("appender.R.layout.type", "SimpleLayout");
    props.setProperty("appender.R.policy.TIME.type", "TimeBasedTriggeringPolicy");
    props.setProperty("appender.R.policy.TIME.datePattern", "'.'yyyy-MM-dd");
    props.setProperty("appender.R.strategy.type", "Date");
    props.setProperty("appender.R.strategy.datePattern", "'.'yyyy-MM-dd");
    props.setProperty("appender.R.strategy.maxBackups", "30");
    props.setProperty("rootLogger.level", "DEBUG");
    props.setProperty("rootLogger.appenderRef.0.ref", "R");

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->appenders().count(), 1);

    auto *appender = qobject_cast<RollingFileAppender *>(root->appenders().first().data());
    QVERIFY(appender != nullptr);

    // Verify triggering policy
    auto *timePolicy = qobject_cast<TimeBasedTriggeringPolicy *>(appender->triggeringPolicy().data());
    QVERIFY(timePolicy != nullptr);
    QCOMPARE(timePolicy->datePattern(), QString("'.'yyyy-MM-dd"));

    // Verify rollover strategy
    auto *dateStrategy = qobject_cast<DateRolloverStrategy *>(appender->rolloverStrategy().data());
    QVERIFY(dateStrategy != nullptr);
    QCOMPARE(dateStrategy->datePattern(), QString("'.'yyyy-MM-dd"));
    QCOMPARE(dateStrategy->mode(), DateRolloverStrategy::NamingMode::Suffix);
    QCOMPARE(dateStrategy->maxBackups(), 30);
}

void PolicyTest::DateRolloverStrategy_embeddedPropertyConfigurator()
{
    // Replaces DailyRollingFileAppender: RollingFile + TimeBased + OnStartup + Date(Embedded)
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString file = tempDir.path() + "/app.log";

    // Create a non-empty file so OnStartupTriggeringPolicy fires
    {
        QFile f(file);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("previous session\n");
        f.close();
    }

    Properties props;
    props.setProperty("appender.R.type", "RollingFile");
    props.setProperty("appender.R.file", file);
    props.setProperty("appender.R.layout.type", "SimpleLayout");
    props.setProperty("appender.R.policy.TIME.type", "TimeBasedTriggeringPolicy");
    props.setProperty("appender.R.policy.TIME.datePattern", "_yyyy_MM_dd");
    props.setProperty("appender.R.policy.STARTUP.type", "OnStartupTriggeringPolicy");
    props.setProperty("appender.R.strategy.type", "Date");
    props.setProperty("appender.R.strategy.datePattern", "_yyyy_MM_dd");
    props.setProperty("appender.R.strategy.mode", "Embedded");
    props.setProperty("appender.R.strategy.maxBackups", "90");
    props.setProperty("rootLogger.level", "DEBUG");
    props.setProperty("rootLogger.appenderRef.0.ref", "R");

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->appenders().count(), 1);

    auto *appender = qobject_cast<RollingFileAppender *>(root->appenders().first().data());
    QVERIFY(appender != nullptr);

    // Verify rollover strategy
    auto *dateStrategy = qobject_cast<DateRolloverStrategy *>(appender->rolloverStrategy().data());
    QVERIFY(dateStrategy != nullptr);
    QCOMPARE(dateStrategy->datePattern(), QString("_yyyy_MM_dd"));
    QCOMPARE(dateStrategy->mode(), DateRolloverStrategy::NamingMode::Embedded);
    QCOMPARE(dateStrategy->maxBackups(), 90);

    // After activation with OnStartup, the file should have the date embedded
    QString expectedSuffix = QDateTime::currentDateTime().toString("_yyyy_MM_dd");
    QVERIFY(appender->file().contains(expectedSuffix));
}

void PolicyTest::DateRolloverStrategy_datedActiveFile_defaults()
{
    Log4Qt::DateRolloverStrategy strategy;
    QCOMPARE(strategy.datedActiveFile(), false);
}

void PolicyTest::DateRolloverStrategy_initialFileName_offByDefault()
{
    const QString path = QStringLiteral("logs/app.log");

    Log4Qt::DateRolloverStrategy suffix;
    suffix.setMode(Log4Qt::DateRolloverStrategy::NamingMode::Suffix);
    QCOMPARE(suffix.initialFileName(path), path);

    Log4Qt::DateRolloverStrategy embedded;
    embedded.setMode(Log4Qt::DateRolloverStrategy::NamingMode::Embedded);
    QCOMPARE(embedded.initialFileName(path), path);
}

void PolicyTest::DateRolloverStrategy_initialFileName_datedEmbedded()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 20), QTime(10, 0)); });

    Log4Qt::DateRolloverStrategy strategy;
    strategy.setDatePattern("_yyyy_MM_dd");
    strategy.setMode(Log4Qt::DateRolloverStrategy::NamingMode::Embedded);
    strategy.setDatedActiveFile(true);

    const QString basePath = tempDir.path() + "/app.log";
    const QString expected = tempDir.path() + "/app_2026_04_20.log";
    QCOMPARE(strategy.initialFileName(basePath), expected);
}

void PolicyTest::DateRolloverStrategy_initialFileName_datedSuffix()
{
    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 20), QTime(10, 0)); });

    Log4Qt::DateRolloverStrategy strategy;
    strategy.setDatePattern("'.'yyyy-MM-dd");
    strategy.setMode(Log4Qt::DateRolloverStrategy::NamingMode::Suffix);
    strategy.setDatedActiveFile(true);

    const QString basePath = QStringLiteral("logs/app.log");
    QCOMPARE(strategy.initialFileName(basePath),
             QStringLiteral("logs/app.log.2026-04-20"));
}

void PolicyTest::DateRolloverStrategy_datedActiveFile_rolloverNoRename()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 20), QTime(10, 0)); });

    const QString basePath = tempDir.path() + "/app.log";
    const QString expected = tempDir.path() + "/app_2026_04_20.log";

    // Pre-create the base file; it must not be touched by rollover.
    {
        QFile f(basePath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("untouched");
        f.close();
    }

    Log4Qt::DateRolloverStrategy strategy;
    strategy.setDatePattern("_yyyy_MM_dd");
    strategy.setMode(Log4Qt::DateRolloverStrategy::NamingMode::Embedded);
    strategy.setDatedActiveFile(true);

    const QString result = strategy.rollover(basePath);
    QCOMPARE(result, expected);

    // Base file survives — no rename, no delete.
    QVERIFY(QFile::exists(basePath));
    // Creating the new active file is the appender's job, not the strategy's.
    QVERIFY(!QFile::exists(expected));
}

void PolicyTest::DateRolloverStrategy_datedActiveFilePropertyConfigurator()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 20), QTime(10, 0)); });

    const QString file = tempDir.path() + "/app.log";

    Properties props;
    props.setProperty("appender.R.type", "RollingFile");
    props.setProperty("appender.R.file", file);
    props.setProperty("appender.R.layout.type", "SimpleLayout");
    props.setProperty("appender.R.policy.TIME.type", "TimeBasedTriggeringPolicy");
    props.setProperty("appender.R.policy.TIME.datePattern", "_yyyy_MM_dd");
    props.setProperty("appender.R.strategy.type", "Date");
    props.setProperty("appender.R.strategy.datePattern", "_yyyy_MM_dd");
    props.setProperty("appender.R.strategy.mode", "Embedded");
    props.setProperty("appender.R.strategy.datedActiveFile", "true");
    props.setProperty("rootLogger.level", "DEBUG");
    props.setProperty("rootLogger.appenderRef.0.ref", "R");

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->appenders().count(), 1);

    auto *appender = qobject_cast<RollingFileAppender *>(root->appenders().first().data());
    QVERIFY(appender != nullptr);

    auto *dateStrategy = qobject_cast<DateRolloverStrategy *>(appender->rolloverStrategy().data());
    QVERIFY(dateStrategy != nullptr);
    QCOMPARE(dateStrategy->datedActiveFile(), true);

    // The active file carries the current date already — no rollover happened.
    const QString expected = tempDir.path() + "/app_2026_04_20.log";
    QCOMPARE(appender->file(), expected);
    QVERIFY(QFile::exists(expected));
    QVERIFY(!QFile::exists(file));
}

// ---------------------------------------------------------------------------
// DefaultRolloverStrategy
// ---------------------------------------------------------------------------

void PolicyTest::DefaultRolloverStrategy_defaults()
{
    Log4Qt::DefaultRolloverStrategy strategy;
    QCOMPARE(strategy.minIndex(), 1);
    QCOMPARE(strategy.maxIndex(), 7);
}

void PolicyTest::DefaultRolloverStrategy_rollover()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString basePath = tempDir.path() + "/app.log";

    Log4Qt::DefaultRolloverStrategy strategy;
    strategy.setMinIndex(1);
    strategy.setMaxIndex(3);

    // Create the base file
    auto writeFile = [](const QString &path, const QString &content)
    {
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write(content.toUtf8());
        f.close();
    };

    auto readFile = [](const QString &path) -> QString
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            return QString();
        return QString::fromUtf8(f.readAll());
    };

    // First rollover: app.log -> app.log.1
    writeFile(basePath, "round1");
    QString result = strategy.rollover(basePath);
    QCOMPARE(result, basePath);
    QVERIFY(QFile::exists(basePath + ".1"));
    QCOMPARE(readFile(basePath + ".1"), "round1");
    QVERIFY(!QFile::exists(basePath));

    // Second rollover: app.log.1 -> app.log.2, app.log -> app.log.1
    writeFile(basePath, "round2");
    strategy.rollover(basePath);
    QCOMPARE(readFile(basePath + ".1"), "round2");
    QCOMPARE(readFile(basePath + ".2"), "round1");

    // Third rollover: app.log.2 -> app.log.3, app.log.1 -> app.log.2, app.log -> app.log.1
    writeFile(basePath, "round3");
    strategy.rollover(basePath);
    QCOMPARE(readFile(basePath + ".1"), "round3");
    QCOMPARE(readFile(basePath + ".2"), "round2");
    QCOMPARE(readFile(basePath + ".3"), "round1");

    // Fourth rollover: app.log.3 deleted, others shifted
    writeFile(basePath, "round4");
    strategy.rollover(basePath);
    QCOMPARE(readFile(basePath + ".1"), "round4");
    QCOMPARE(readFile(basePath + ".2"), "round3");
    QCOMPARE(readFile(basePath + ".3"), "round2");
    // "round1" was in .3 and got deleted
    QVERIFY(!QFile::exists(basePath + ".4"));
}

void PolicyTest::DefaultRolloverStrategy_noExistingFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString basePath = tempDir.path() + "/nonexistent.log";

    Log4Qt::DefaultRolloverStrategy strategy;
    strategy.setMaxIndex(3);

    // Should not crash, returns the filename
    QString result = strategy.rollover(basePath);
    QCOMPARE(result, basePath);
}

void PolicyTest::DefaultRolloverStrategy_singleBackup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString basePath = tempDir.path() + "/single.log";

    Log4Qt::DefaultRolloverStrategy strategy;
    strategy.setMinIndex(1);
    strategy.setMaxIndex(1);

    auto writeFile = [](const QString &path, const QString &content)
    {
        QFile f(path);
        f.open(QIODevice::WriteOnly);
        f.write(content.toUtf8());
        f.close();
    };

    auto readFile = [](const QString &path) -> QString
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            return QString();
        return QString::fromUtf8(f.readAll());
    };

    // First rollover
    writeFile(basePath, "first");
    strategy.rollover(basePath);
    QCOMPARE(readFile(basePath + ".1"), "first");

    // Second rollover: .1 is deleted (maxIndex), then base -> .1
    writeFile(basePath, "second");
    strategy.rollover(basePath);
    QCOMPARE(readFile(basePath + ".1"), "second");
    QVERIFY(!QFile::exists(basePath + ".2"));
}

void PolicyTest::DefaultRolloverStrategy_initialFileNameUnchanged()
{
    Log4Qt::DefaultRolloverStrategy strategy;
    const QString path = QStringLiteral("logs/app.log");
    QCOMPARE(strategy.initialFileName(path), path);
}

// ---------------------------------------------------------------------------
// RollingFileAppender integration
// ---------------------------------------------------------------------------

void PolicyTest::RollingFileAppender_initialFileName_appliedOnStartup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 20), QTime(10, 0)); });

    const QString basePath = tempDir.path() + "/app.log";
    const QString expected = tempDir.path() + "/app_2026_04_20.log";

    auto layout = LayoutSharedPtr(new SimpleLayout);
    RollingFileAppender appender(layout, basePath);

    auto strategy = RolloverStrategySharedPtr(new DateRolloverStrategy);
    auto *ds = qobject_cast<DateRolloverStrategy *>(strategy.data());
    QVERIFY(ds != nullptr);
    ds->setDatePattern("_yyyy_MM_dd");
    ds->setMode(DateRolloverStrategy::NamingMode::Embedded);
    ds->setDatedActiveFile(true);
    appender.setRolloverStrategy(strategy);

    appender.activateOptions();

    QCOMPARE(appender.file(), expected);
    QVERIFY(QFile::exists(expected));
    QVERIFY(!QFile::exists(basePath));

    appender.close();
}

void PolicyTest::RollingFileAppender_rolloverUsesBaseFileName()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString basePath = tempDir.path() + "/app.log";
    const QString day1Path = tempDir.path() + "/app_2026_04_20.log";
    const QString day2Path = tempDir.path() + "/app_2026_04_21.log";

    // Day 1: activate with datedActiveFile=true — file becomes app_2026_04_20.log
    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 20), QTime(10, 0)); });

    auto layout = LayoutSharedPtr(new SimpleLayout);
    RollableFileAppender appender(layout, basePath);

    auto strategy = RolloverStrategySharedPtr(new DateRolloverStrategy);
    auto *ds = qobject_cast<DateRolloverStrategy *>(strategy.data());
    QVERIFY(ds != nullptr);
    ds->setDatePattern("_yyyy_MM_dd");
    ds->setMode(DateRolloverStrategy::NamingMode::Embedded);
    ds->setDatedActiveFile(true);
    appender.setRolloverStrategy(strategy);

    appender.activateOptions();
    QCOMPARE(appender.file(), day1Path);

    // Day 2: trigger rollover — next file name must derive from the original
    // base ("app.log"), not from the current dated file. If the base were
    // not preserved we'd end up with "app_2026_04_20_2026_04_21.log".
    DateTime::setProvider([] { return QDateTime(QDate(2026, 4, 21), QTime(10, 0)); });
    appender.triggerRollover();

    QCOMPARE(appender.file(), day2Path);
    QVERIFY(QFile::exists(day2Path));

    appender.close();
}

void PolicyTest::RollingFileAppender_defaultStrategyUnchangedOnStartup()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString basePath = tempDir.path() + "/app.log";

    auto layout = LayoutSharedPtr(new SimpleLayout);
    RollingFileAppender appender(layout, basePath);
    // No explicit strategy → activateOptions installs DefaultRolloverStrategy,
    // whose initialFileName() must not change the configured filename.

    appender.activateOptions();

    QCOMPARE(appender.file(), basePath);
    QVERIFY(QFile::exists(basePath));

    appender.close();
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

void PolicyTest::Factory_createTriggeringPolicy_data()
{
    QTest::addColumn<QString>("className");

    QTest::newRow("SizeBasedTriggeringPolicy")      << "SizeBasedTriggeringPolicy";
    QTest::newRow("SizeBased")                      << "SizeBased";
    QTest::newRow("Log4Qt::SizeBasedTriggeringPolicy") << "Log4Qt::SizeBasedTriggeringPolicy";

    QTest::newRow("TimeBasedTriggeringPolicy")      << "TimeBasedTriggeringPolicy";
    QTest::newRow("TimeBased")                      << "TimeBased";
    QTest::newRow("Log4Qt::TimeBasedTriggeringPolicy") << "Log4Qt::TimeBasedTriggeringPolicy";

    QTest::newRow("CronTriggeringPolicy")           << "CronTriggeringPolicy";
    QTest::newRow("Cron")                           << "Cron";
    QTest::newRow("Log4Qt::CronTriggeringPolicy")   << "Log4Qt::CronTriggeringPolicy";

    QTest::newRow("OnStartupTriggeringPolicy")      << "OnStartupTriggeringPolicy";
    QTest::newRow("OnStartup")                      << "OnStartup";
    QTest::newRow("Log4Qt::OnStartupTriggeringPolicy") << "Log4Qt::OnStartupTriggeringPolicy";
}

void PolicyTest::Factory_createTriggeringPolicy()
{
    QFETCH(QString, className);

    TriggeringPolicy *policy = Factory::createTriggeringPolicy(className);
    QVERIFY(policy != nullptr);
    delete policy;
}

void PolicyTest::Factory_createRolloverStrategy_data()
{
    QTest::addColumn<QString>("className");

    QTest::newRow("DefaultRolloverStrategy")        << "DefaultRolloverStrategy";
    QTest::newRow("Default")                        << "Default";
    QTest::newRow("Log4Qt::DefaultRolloverStrategy") << "Log4Qt::DefaultRolloverStrategy";

    QTest::newRow("DateRolloverStrategy")           << "DateRolloverStrategy";
    QTest::newRow("Date")                           << "Date";
    QTest::newRow("Log4Qt::DateRolloverStrategy")   << "Log4Qt::DateRolloverStrategy";
}

void PolicyTest::Factory_createRolloverStrategy()
{
    QFETCH(QString, className);

    RolloverStrategy *strategy = Factory::createRolloverStrategy(className);
    QVERIFY(strategy != nullptr);
    delete strategy;
}

// ---------------------------------------------------------------------------
// PropertyConfigurator integration
// ---------------------------------------------------------------------------

void PolicyTest::PropertyConfigurator_cronPolicy()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString file = tempDir.path() + "/cron.log";

    Properties props;
    props.setProperty("appender.R.type", "RollingFile");
    props.setProperty("appender.R.file", file);
    props.setProperty("appender.R.layout.type", "SimpleLayout");
    props.setProperty("appender.R.policy.CRON.type", "CronTriggeringPolicy");
    props.setProperty("appender.R.policy.CRON.schedule", "0 0 0 * * ?");
    props.setProperty("appender.R.strategy.type", "DefaultRolloverStrategy");
    props.setProperty("appender.R.strategy.maxIndex", "5");
    props.setProperty("rootLogger.level", "DEBUG");
    props.setProperty("rootLogger.appenderRef.0.ref", "R");

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->appenders().count(), 1);

    auto *appender = qobject_cast<RollingFileAppender *>(root->appenders().first().data());
    QVERIFY(appender != nullptr);
    QVERIFY(appender->triggeringPolicy() != nullptr);
    QVERIFY(appender->rolloverStrategy() != nullptr);

    // Check that the cron policy was correctly configured
    auto *cronPolicy = qobject_cast<CronTriggeringPolicy *>(appender->triggeringPolicy().data());
    QVERIFY(cronPolicy != nullptr);
    QCOMPARE(cronPolicy->schedule(), QString("0 0 0 * * ?"));

    auto *strategy = qobject_cast<DefaultRolloverStrategy *>(appender->rolloverStrategy().data());
    QVERIFY(strategy != nullptr);
    QCOMPARE(strategy->maxIndex(), 5);
}

QTEST_MAIN(PolicyTest)

#include "tst_policytest.moc"
