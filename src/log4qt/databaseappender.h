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

namespace Log4Qt
{

/*!
 * \brief The class DatabaseAppender appends log events to a sql database.
 *
 * \note All the functions declared in this class are thread-safe.
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

private:
    Q_DISABLE_COPY(DatabaseAppender)

public:
    bool requiresLayout() const override;

    QString connection() const;
    QString table() const;

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
     * - A writer has been set (APPENDER_USE_MISSING_WRITER_ERROR)
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
    QString connectionName;
    QString tableName;
};

inline QString DatabaseAppender::connection() const
{
    QMutexLocker locker(&mObjectGuard);
    return connectionName;
}

inline QString DatabaseAppender::table() const
{
    QMutexLocker locker(&mObjectGuard);
    return tableName;
}

} // namespace Log4Qt


#endif // LOG4QT_DATABASEAPPENDER_H
