/******************************************************************************
 *
 * package:     Log4Qt
 * file:        databaselayout.h
 * created:     March 2010
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

#ifndef LOG4QT_DATABASELAYOUT_H
#define LOG4QT_DATABASELAYOUT_H


#include "layout.h"

#include <QtSql/QSqlRecord>


namespace Log4Qt
{

/*!
 * \brief The class DatabaseLayout outputs loggin event into sql table.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT  DatabaseLayout : public Layout
{
    Q_OBJECT

    /*!
        The property holds column name to save timestamp of log event
    */
    Q_PROPERTY(QString timeStampColumn READ timeStampColumn WRITE setTimeStampColumn)
    /*!
        The property holds column name to save logger name of log event
    */
    Q_PROPERTY(QString loggenameColumn READ loggenameColumn WRITE setLoggenameColumn)
    /*!
        The property holds column name to save thread name of log event
    */
    Q_PROPERTY(QString threadNameColumn READ threadNameColumn WRITE setThreadNameColumn)
    /*!
        The property holds column name to save level of log event
    */
    Q_PROPERTY(QString levelColumn READ levelColumn WRITE setLevelColumn)
    /*!
        The property holds column name to save message of log event
    */
    Q_PROPERTY(QString messageColumn READ messageColumn WRITE setMessageColumn)


public:
    DatabaseLayout(QObject *parent = nullptr);
    // virtual ~DatabaseLayout(); // Use compiler default
private:
    Q_DISABLE_COPY(DatabaseLayout)

public:
    virtual QSqlRecord formatRecord(const LoggingEvent &event);
    QString format(const LoggingEvent &event) override;

    QString timeStampColumn() const;
    QString loggenameColumn() const;
    QString threadNameColumn() const;
    QString levelColumn() const;
    QString messageColumn() const;

    void setTimeStampColumn(const QString &columnName);
    void setLoggenameColumn(const QString &columnName);
    void setThreadNameColumn(const QString &columnName);
    void setLevelColumn(const QString &columnName);
    void setMessageColumn(const QString &columnName);

private:
    QString mTimeStamp;
    QString mLoggename;
    QString mThreadName;
    QString mLevel;
    QString mMessage;
};


inline DatabaseLayout::DatabaseLayout(QObject *parent) :
    Layout(parent)
{}


} // namespace Log4Qt

#endif // LOG4QT_DATABASELAYOUT_H
