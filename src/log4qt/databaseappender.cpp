/******************************************************************************
 *
 * package:     Log4Qt
 * file:        databaseappender.cpp
 * created:     Marth 2010
 * author:      Michael Filonenko
 *
 *
 * Copyright 2010 Michael Filonenko
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


#include "databaseappender.h"

#include "databaselayout.h"
#include "loggingevent.h"

#include "helpers/datetime.h"

#include <QStringBuilder>
#include <QThread>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

DatabaseAppender::DatabaseAppender(QObject *parent) :
      AppenderSkeleton(false, parent)
    , connectionName(QSqlDatabase::defaultConnection)
{
}


DatabaseAppender::DatabaseAppender(const LayoutSharedPtr &layout,
                                   QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , connectionName(QSqlDatabase::defaultConnection)
{
}


DatabaseAppender::DatabaseAppender(const LayoutSharedPtr &layout,
                                   const QString &tableName,
                                   const QString &connection,
                                   QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , connectionName(connection)
    , tableName(tableName)
{
}

void DatabaseAppender::setConnection(const QString &connection)
{
    QMutexLocker locker(&mObjectGuard);

    if (connectionName == connection)
        return;

    connectionName = connection;
    resetPreparedQuery();
}

void DatabaseAppender::setTable(const QString &table)
{
    QMutexLocker locker(&mObjectGuard);

    if (table == tableName)
        return;

    tableName = table;
    resetPreparedQuery();
}

void DatabaseAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (!QSqlDatabase::contains(connectionName) || tableName.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR("Activation of Appender '%1' that requires sql connection and table and has no connection or table set"
                                         , AppenderMissingDatabaseOrTableError);
        e << name();
        logger()->error(e);
        return;
    }

    prepareInsert();

    AppenderSkeleton::activateOptions();
}

DatabaseAppender::~DatabaseAppender()
{
    resetPreparedQuery();
}

void DatabaseAppender::resetPreparedQuery()
{
    if (mPreparedQuery != nullptr && mActivationThread != nullptr
        && QThread::currentThread() != mActivationThread)
    {
        // ~QSqlQuery tears down the statement through driver code — running
        // that on a thread other than the one that created the query is the
        // exact cross-thread driver use the activation-thread guard in
        // append() exists to prevent. Intentionally leak the handle instead;
        // resetting from a foreign thread is a rare reconfiguration path.
        logger()->warn(u"Appender '%1': prepared statement released from a foreign thread; leaking the handle to avoid cross-thread database driver access"_s.arg(name()));
        Q_UNUSED(mPreparedQuery.release())
    }
    mPreparedQuery.reset();
    mBindings.clear();
    mActivationThread = nullptr;
    mWrongThreadLogged = false;
}

void DatabaseAppender::prepareInsert()
{
    resetPreparedQuery();

    DatabaseLayout *dbLayout = qobject_cast<DatabaseLayout *>(layout().data());
    if (dbLayout == nullptr)
        return;

    struct ColumnSpec { QString name; ColumnSource source; };
    const ColumnSpec specs[] = {
        { dbLayout->timeStampColumn(), ColumnSource::TimeStamp },
        { dbLayout->loggerNameColumn(), ColumnSource::Loggername },
        { dbLayout->threadNameColumn(), ColumnSource::ThreadName },
        { dbLayout->levelColumn(), ColumnSource::Level },
        { dbLayout->messageColumn(), ColumnSource::Message },
    };

    QStringList columns;
    columns.reserve(std::size(specs));
    for (const auto &spec : specs)
    {
        if (spec.name.isEmpty())
            continue;
        columns.append(spec.name);
        mBindings.push_back(spec.source);
    }

    if (columns.isEmpty())
        return;

    QSqlDatabase database = QSqlDatabase::database(connectionName);

    // Escape the table and column identifiers through the driver, as the
    // previously used QSqlDriver::sqlStatement() did: quoted, mixed-case or
    // spaced identifiers from the configuration would otherwise produce an
    // invalid statement (or inject into it). Escape into locals — the
    // members hold the configured, unescaped names.
    QString escapedTable = tableName;
    if (const QSqlDriver *driver = database.driver())
    {
        escapedTable = driver->escapeIdentifier(tableName, QSqlDriver::TableName);
        for (QString &column : columns)
            column = driver->escapeIdentifier(column, QSqlDriver::FieldName);
    }

    const QString placeholders = QStringList(columns.size(), u"?"_s).join(u',');
    const QString sql = u"INSERT INTO "_s % escapedTable
                        % u" ("_s % columns.join(u',')
                        % u") VALUES ("_s % placeholders % u')';

    auto query = std::make_unique<QSqlQuery>(database);
    if (!query->prepare(sql))
    {
        LogError e = LOG4QT_ERROR("Sql prepare error: '%1'",
                                  AppenderExecSqlQueryError,
                                  Q_FUNC_INFO);
        e << query->lastError().text();
        logger()->error(e);
        mBindings.clear();
        return;
    }
    mPreparedQuery = std::move(query);
    mActivationThread = QThread::currentThread();
}

bool DatabaseAppender::requiresLayout() const
{
    return true;
}

void DatabaseAppender::bindEventValues(const LoggingEvent &event)
{
    for (std::size_t i = 0; i < mBindings.size(); ++i)
    {
        const int pos = static_cast<int>(i);
        switch (mBindings[i])
        {
        case ColumnSource::TimeStamp:
            mPreparedQuery->bindValue(pos, DateTime::fromMSecsSinceEpoch(event.timeStamp()));
            break;
        case ColumnSource::Loggername:
            mPreparedQuery->bindValue(pos, event.loggername());
            break;
        case ColumnSource::ThreadName:
            mPreparedQuery->bindValue(pos, event.threadName());
            break;
        case ColumnSource::Level:
            mPreparedQuery->bindValue(pos, event.level().toString());
            break;
        case ColumnSource::Message:
            mPreparedQuery->bindValue(pos, event.message());
            break;
        }
    }
}

void DatabaseAppender::append(const LoggingEvent &event)
{
    // The activation-time prepare may have failed — e.g. the database was
    // briefly unreachable or the table did not exist yet — so retry before
    // giving up. resetPreparedQuery() cleared mActivationThread, so the new
    // query binds to the current logging thread.
    if (mPreparedQuery == nullptr)
        prepareInsert();

    if (mPreparedQuery == nullptr)
    {
        LogError e = LOG4QT_QCLASS_ERROR("Use of appender '%1' with invalid layout or unprepared query",
                                         AppenderInvalidDatabaseLayoutError);
        e << name();
        logger()->error(e);
        return;
    }

    // QSqlQuery/QSqlDatabase are bound to the thread that created them. Using
    // them from another thread is undefined and can crash the driver, so drop
    // the event (logging the cause once) instead.
    if (mActivationThread != nullptr && QThread::currentThread() != mActivationThread)
    {
        if (!mWrongThreadLogged)
        {
            LogError e = LOG4QT_QCLASS_ERROR("Appender '%1' was fed from a thread other than the one that activated it; QSqlDatabase is not thread-safe. Front it with a MainThreadAppender or open the connection on the logging thread.",
                                             AppenderExecSqlQueryError);
            e << name();
            logger()->error(e);
            mWrongThreadLogged = true;
        }
        return;
    }

    bindEventValues(event);
    if (mPreparedQuery->exec())
        return;

    // The connection may have dropped since the statement was prepared
    // (server restart, network outage). QSqlDatabase::database() re-opens a
    // closed connection, so re-prepare the statement and retry once before
    // reporting the failure.
    const QString execError = mPreparedQuery->lastQuery() % u' '
                              % mPreparedQuery->lastError().text();
    prepareInsert();
    if (mPreparedQuery != nullptr)
    {
        bindEventValues(event);
        if (mPreparedQuery->exec())
            return;
    }

    LogError e = LOG4QT_ERROR("Sql query exec error: '%1'",
                              AppenderExecSqlQueryError,
                              Q_FUNC_INFO);
    e << execError;
    logger()->error(e);
}

bool DatabaseAppender::checkEntryConditions() const
{
    if (!QSqlDatabase::contains(connectionName) || tableName.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR("Use of appender '%1' with invalid database or empty table name",
                                         AppenderMissingDatabaseOrTableError);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

#include "moc_databaseappender.cpp"

