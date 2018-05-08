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

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

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
}

void DatabaseAppender::setTable(const QString &table)
{
    QMutexLocker locker(&mObjectGuard);

    if (table == tableName)
        return;

    tableName = table;
}

void DatabaseAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (!QSqlDatabase::contains(connectionName) || tableName.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Activation of Appender '%1' that requires sql connection and table and has no connection or table set")
                                         , APPENDER_MISSING_DATABASE_OR_TABLE_ERROR);
        e << name();
        logger()->error(e);
        return;
    }

    AppenderSkeleton::activateOptions();
}

bool DatabaseAppender::requiresLayout() const
{
    return true;
}

void DatabaseAppender::append(const LoggingEvent &event)
{
    DatabaseLayout *databaseLayout = qobject_cast<DatabaseLayout *>(layout().data());

    if (databaseLayout != nullptr)
    {
        QSqlRecord record = databaseLayout->formatRecord(event);

        QSqlDatabase database = QSqlDatabase::database(connectionName);
        QSqlQuery query(database);
        if (!query.exec(database.driver()->sqlStatement(QSqlDriver::InsertStatement
                        , tableName, record, false)))
        {
            LogError e = LOG4QT_ERROR(QT_TR_NOOP("Sql query exec error: '%1'"),
                                      APPENDER_EXEC_SQL_QUERY_ERROR,
                                      Q_FUNC_INFO);
            e << query.lastQuery() + " " + query.lastError().text();
            logger()->error(e);
        }
    }
    else
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' with invalid layout"),
                                         APPENDER_INVALID_DATABASE_LAYOUT_ERROR);
        e << name();
        logger()->error(e);
    }
}

bool DatabaseAppender::checkEntryConditions() const
{
    if (!QSqlDatabase::contains(connectionName) || tableName.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' with invalid database or empty table name"),
                                         APPENDER_MISSING_DATABASE_OR_TABLE_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

#include "moc_databaseappender.cpp"

