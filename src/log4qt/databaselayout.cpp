/******************************************************************************
 *
 * package:     Log4Qt
 * file:        databaselayout.cpp
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


#include "databaselayout.h"

#include "loggingevent.h"

#include "helpers/datetime.h"

#include <QtSql/QSqlField>

namespace Log4Qt
{

QSqlRecord DatabaseLayout::formatRecord(const LoggingEvent &event)
{
    QSqlField field;
    QSqlRecord record;

    if (!mTimeStamp.isEmpty())
    {
        field.setName(mTimeStamp);
        field.setType(QVariant::DateTime);
        field.setGenerated(true);
        field.setValue(DateTime::fromMSecsSinceEpoch(event.timeStamp()));
        record.append(field);
    }

    if (!mLoggename.isEmpty())
    {
        field.setName(mLoggename);
        field.setType(QVariant::String);
        field.setGenerated(true);
        field.setValue(event.loggename());
        record.append(field);
    }

    if (!mThreadName.isEmpty())
    {
        field.setName(mThreadName);
        field.setType(QVariant::String);
        field.setGenerated(true);
        field.setValue(event.threadName());
        record.append(field);
    }

    if (!mLevel.isEmpty())
    {
        field.setName(mLevel);
        field.setType(QVariant::String);
        field.setGenerated(true);
        field.setValue(event.level().toString());
        record.append(field);
    }

    if (!mMessage.isEmpty())
    {
        field.setName(mMessage);
        field.setType(QVariant::String);
        field.setGenerated(true);
        field.setValue(event.message());
        record.append(field);
    }
    return record;
}


QString DatabaseLayout::format(const LoggingEvent &event)
{
    QString result;

    if (!mTimeStamp.isEmpty())
    {
        result.append(mTimeStamp);
        result.append(":");
        result.append(DateTime::fromMSecsSinceEpoch(event.timeStamp()).toString(QStringLiteral("dd.MM.yyyy hh:mm")));
    }

    if (!mThreadName.isEmpty())
    {
        result.append(mThreadName);
        result.append(":");
        result.append(mThreadName);
        result.append("; ");
    }

    if (!mLevel.isEmpty())
    {
        result.append(mLevel);
        result.append(":");
        result.append(mLevel);
        result.append("; ");
    }

    if (!mLoggename.isEmpty())
    {
        result.append(mLoggename);
        result.append(":");
        result.append(mLoggename);
        result.append("; ");
    }

    if (!mMessage.isEmpty())
    {
        result.append(mMessage);
        result.append(":");
        result.append(mMessage);
        result.append("; ");
    }
    return result;
}

QString DatabaseLayout::timeStampColumn() const
{
    return mTimeStamp;
}
QString DatabaseLayout::loggenameColumn() const
{
    return mLoggename;
}
QString DatabaseLayout::threadNameColumn() const
{
    return mThreadName;
}

QString DatabaseLayout::levelColumn() const
{
    return mLevel;
}

QString DatabaseLayout::messageColumn() const
{
    return mMessage;
}


void DatabaseLayout::setTimeStampColumn(const QString &columnName)
{
    mTimeStamp = columnName;
}

void DatabaseLayout::setLoggenameColumn(const QString &columnName)
{
    mLoggename = columnName;
}

void DatabaseLayout::setThreadNameColumn(const QString &columnName)
{
    mThreadName = columnName;
}

void DatabaseLayout::setLevelColumn(const QString &columnName)
{
    mLevel = columnName;
}

void DatabaseLayout::setMessageColumn(const QString &columnName)
{
    mMessage = columnName;
}

} // namespace Log4Qt

#include "moc_databaselayout.cpp"
