/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#include "qmllogger.h"

#include "logger.h"
#include "level.h"

#include <QStringBuilder>
#include <QTimer>

namespace Log4Qt
{

QmlLogger::QmlLogger(QObject *parent) :
    QObject(parent)
    , mContext(QStringLiteral("Qml"))
    , mLogger(nullptr)
{
}

void QmlLogger::trace(const QString &message) const
{
    logger()->trace(message);
}

void QmlLogger::debug(const QString &message) const
{
    logger()->debug(message);
}

void QmlLogger::info(const QString &message) const
{
    logger()->info(message);
}

void QmlLogger::error(const QString &message) const
{
    logger()->error(message);
}

void QmlLogger::fatal(const QString &message) const
{
    logger()->fatal(message);
}

void QmlLogger::log(Level level, const QString &message) const
{
    Log4Qt::Level loglevel(static_cast<Log4Qt::Level::Value>(level));
    logger()->log(loglevel, message);
}

QString QmlLogger::name() const
{
    return mName;
}

void QmlLogger::setName(const QString &name)
{
    if (mName != name)
    {
        mName = name;
        Q_EMIT nameChanged(name);
    }
}

QString QmlLogger::context() const
{
    return mContext;
}

QmlLogger::Level QmlLogger::level() const
{
    return static_cast<QmlLogger::Level>(mLogger->level().toInt());
}

void QmlLogger::setContext(const QString &context)
{
    if (mContext != context)
    {
        mContext = context;
        Q_EMIT contextChanged(context);
    }
}

void QmlLogger::setLevel(QmlLogger::Level level)
{
    if (this->level() != level)
    {
        mLogger->setLevel(Log4Qt::Level(static_cast<Log4Qt::Level::Value>(level)));
        Q_EMIT levelChanged(level);
    }
}

QString QmlLogger::loggename() const
{
    if (mName.isEmpty() && (parent() != nullptr))
        mName = parent()->objectName();

    if (!mContext.isEmpty())
        return mContext % "." % mName;
    return mName;
}

Logger *QmlLogger::logger() const
{
    if (mLogger == nullptr)
        mLogger = Log4Qt::Logger::logger(loggename());

    return  mLogger;
}

}

#include "moc_qmllogger.cpp"
