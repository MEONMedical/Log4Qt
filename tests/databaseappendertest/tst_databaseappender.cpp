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

#include <QTest>
#include <QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "log4qt/databaseappender.h"
#include "log4qt/databaselayout.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"

using namespace Log4Qt;

static const QString kConnection = QStringLiteral("tst_databaseappender");

class DatabaseAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void DatabaseAppender_basicInsert();
    void DatabaseAppender_recoversFromLateTableCreation();
    void DatabaseAppender_recoversFromClosedConnection();
    void DatabaseAppender_escapesIdentifiers();

private:
    void createLogTable();
    int rowCount(const QString &message);

    QTemporaryDir mTempDir;
    QString mDbPath;
};

void DatabaseAppenderTest::init()
{
    QVERIFY(mTempDir.isValid());
    mDbPath = mTempDir.path() + QStringLiteral("/%1.sqlite")
                  .arg(QString::fromLatin1(QTest::currentTestFunction()));

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnection);
    db.setDatabaseName(mDbPath);
    QVERIFY2(db.open(), qPrintable(db.lastError().text()));
}

void DatabaseAppenderTest::cleanup()
{
    QSqlDatabase::database(kConnection, false).close();
    QSqlDatabase::removeDatabase(kConnection);
    LogManager::resetConfiguration();
}

void DatabaseAppenderTest::createLogTable()
{
    QSqlQuery query(QSqlDatabase::database(kConnection));
    QVERIFY2(query.exec(QStringLiteral(
        "CREATE TABLE log (timestamp TEXT, logger TEXT, thread TEXT, "
        "level TEXT, message TEXT)")), qPrintable(query.lastError().text()));
}

int DatabaseAppenderTest::rowCount(const QString &message)
{
    QSqlQuery query(QSqlDatabase::database(kConnection));
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM log WHERE message = '%1'")
                        .arg(message))
        || !query.next())
        return -1;
    return query.value(0).toInt();
}

static LayoutSharedPtr makeLayout()
{
    auto *layout = new DatabaseLayout;
    layout->setTimeStampColumn(QStringLiteral("timestamp"));
    layout->setLoggernameColumn(QStringLiteral("logger"));
    layout->setThreadNameColumn(QStringLiteral("thread"));
    layout->setLevelColumn(QStringLiteral("level"));
    layout->setMessageColumn(QStringLiteral("message"));
    return LayoutSharedPtr(layout);
}

void DatabaseAppenderTest::DatabaseAppender_basicInsert()
{
    createLogTable();

    DatabaseAppender appender(makeLayout(), QStringLiteral("log"), kConnection);
    appender.setName(QStringLiteral("Db"));
    appender.activateOptions();
    QVERIFY(appender.isActive());

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("basic message")));

    QCOMPARE(rowCount(QStringLiteral("basic message")), 1);
}

// Regression test: when the prepare at activation time failed (here: the
// table does not exist yet), the appender used to log "unprepared query"
// forever — nothing ever retried the prepare, even after the cause was gone.
void DatabaseAppenderTest::DatabaseAppender_recoversFromLateTableCreation()
{
    DatabaseAppender appender(makeLayout(), QStringLiteral("log"), kConnection);
    appender.setName(QStringLiteral("Db"));
    appender.activateOptions(); // prepare fails: no table yet
    QVERIFY(appender.isActive());

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("lost message")));

    createLogTable();

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("recovered message")));

    QCOMPARE(rowCount(QStringLiteral("recovered message")), 1);
}

// Regression test: when the connection dropped after activation (server
// restart, network outage — simulated here by closing the connection), every
// exec() failed for the remainder of the process because nothing re-opened
// the connection or re-prepared the statement. The old per-event
// QSqlDatabase::database() path recovered automatically.
void DatabaseAppenderTest::DatabaseAppender_recoversFromClosedConnection()
{
    createLogTable();

    DatabaseAppender appender(makeLayout(), QStringLiteral("log"), kConnection);
    appender.setName(QStringLiteral("Db"));
    appender.activateOptions();
    QVERIFY(appender.isActive());

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("before drop")));
    QCOMPARE(rowCount(QStringLiteral("before drop")), 1);

    // Simulate a dropped connection
    QSqlDatabase::database(kConnection, false).close();

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("after drop")));

    QCOMPARE(rowCount(QStringLiteral("after drop")), 1);
}

// Regression test: the INSERT statement was built by raw string
// concatenation, without the identifier escaping the previously used
// QSqlDriver::sqlStatement() applied — table or column names containing
// spaces (or reserved words / mixed case) failed to prepare.
void DatabaseAppenderTest::DatabaseAppender_escapesIdentifiers()
{
    {
        QSqlQuery query(QSqlDatabase::database(kConnection));
        QVERIFY2(query.exec(QStringLiteral(
            "CREATE TABLE \"log table\" (\"time stamp\" TEXT, logger TEXT, "
            "thread TEXT, level TEXT, \"log message\" TEXT)")),
                 qPrintable(query.lastError().text()));
    }

    auto *layout = new DatabaseLayout;
    layout->setTimeStampColumn(QStringLiteral("time stamp"));
    layout->setLoggernameColumn(QStringLiteral("logger"));
    layout->setThreadNameColumn(QStringLiteral("thread"));
    layout->setLevelColumn(QStringLiteral("level"));
    layout->setMessageColumn(QStringLiteral("log message"));

    DatabaseAppender appender(LayoutSharedPtr(layout),
                              QStringLiteral("log table"), kConnection);
    appender.setName(QStringLiteral("Db"));
    appender.activateOptions();
    QVERIFY(appender.isActive());

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("spaced message")));

    QSqlQuery query(QSqlDatabase::database(kConnection));
    QVERIFY(query.exec(QStringLiteral(
        "SELECT COUNT(*) FROM \"log table\" WHERE \"log message\" = 'spaced message'")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);
}

QTEST_MAIN(DatabaseAppenderTest)
#include "tst_databaseappender.moc"
