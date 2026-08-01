/******************************************************************************
 *
 * package:         Log4Qt
 * file:        databaseappender.h
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

#ifndef LOG4QT_DATABASEAPPENDER_H
#define LOG4QT_DATABASEAPPENDER_H

#include "appenderskeleton.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <memory>
#include <vector>

class QThread;

namespace Log4Qt
{

/*!
 * \brief The class DatabaseAppender appends log events to a sql database.
 *
 * \note A QSqlDatabase connection and the QSqlQuery prepared from it may only
 *       be used on the thread that created them (a Qt requirement). This
 *       appender prepares its statement in activateOptions() and executes it
 *       in append(); both must therefore run on the same thread, and the
 *       QSqlDatabase connection must have been opened on that thread. If a
 *       logging call reaches append() from a different thread the event is
 *       dropped and an error is logged (rather than corrupting the SQL
 *       driver). To log to a database from arbitrary threads, place a
 *       MainThreadAppender (or AsyncAppender bound to the DB thread) in front.
 * &nbsp;
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT DatabaseAppender : public AppenderSkeleton
{
    Q_OBJECT

    /*!
     * The property holds sql database connection the appender uses.
     *
     * The default is null to use the codec the writer has set.
     *
     * \sa connection(), setConnection()
     */
    Q_PROPERTY(QString connection READ connection WRITE setConnection)

    /*!
     * The property holds sql database table name
     *
    \sa table(), setTable()
     */
    Q_PROPERTY(QString table READ table WRITE setTable)
public:
    DatabaseAppender(QObject *parent = nullptr);
    DatabaseAppender(const LayoutSharedPtr &layout,
                     QObject *parent = nullptr);
    DatabaseAppender(const LayoutSharedPtr &layout
                     , const QString &tableName
                     , const QString &connection = QSqlDatabase::defaultConnection
                     , QObject *parent = nullptr);
    ~DatabaseAppender() override;

private:
    Q_DISABLE_COPY_MOVE(DatabaseAppender)

public:
    bool requiresLayout() const override;

    QString connection() const
    {
        QMutexLocker locker(&mObjectGuard);
        return connectionName;
    }
    QString table() const
    {
        QMutexLocker locker(&mObjectGuard);
        return tableName;
    }

    void setConnection(const QString &connection);
    void setTable(const QString &table);

    void activateOptions() override;

protected:
    void append(const LoggingEvent &event) override;

    /*!
     * Tests if all entry conditions for using append() in this class are
     * met.
     *
     * If a conditions is not met, an error is logged and the function
     * returns false. Otherwise the result of
     * AppenderSkeleton::checkEntryConditions() is returned.
     *
     * The checked conditions are:
     * - A writer has been set (AppenderUseMissingWriterError)
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by AppenderSkeleton::doAppend().
     *
     * \sa AppenderSkeleton::doAppend(),
     *     AppenderSkeleton::checkEntryConditions()
     */
    bool checkEntryConditions() const override;

    void closeWriter();

private:
    enum class ColumnSource { TimeStamp, Loggername, ThreadName, Level, Message };

    void resetPreparedQuery();
    void prepareInsert();
    void bindEventValues(const LoggingEvent &event);

    QString connectionName;
    QString tableName;
    std::unique_ptr<QSqlQuery> mPreparedQuery;
    std::vector<ColumnSource> mBindings;
    // Thread that prepared mPreparedQuery (set in activateOptions). The query
    // must only be exec'd on this thread. Accessed only under mObjectGuard.
    QThread *mActivationThread = nullptr;
    bool mWrongThreadLogged = false;
};

} // namespace Log4Qt


#endif // LOG4QT_DATABASEAPPENDER_H
