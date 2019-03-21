/******************************************************************************
 *
 * package:     Log4Qt
 * file:        loggingevent.cpp
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
 *****************************************************************************/

#include "loggingevent.h"

#include "helpers/datetime.h"
#include "helpers/initialisationhelper.h"
#include "logger.h"
#include "mdc.h"
#include "ndc.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QMutex>
#include <QThread>

namespace Log4Qt
{

Q_GLOBAL_STATIC(QMutex, sequence_guard)

LoggingEvent::LoggingEvent() :
    QEvent(eventId),
    mLevel(Level::NULL_INT),
    mLogger(nullptr),
    mMessage(),
    mNdc(NDC::peek()),
    mProperties(MDC::context()),
    mSequenceNumber(nextSequenceNumber()),
    mThreadName(),
    mTimeStamp(QDateTime::currentDateTime().toMSecsSinceEpoch())
{
    setThreadNameToCurrent();
}


LoggingEvent::LoggingEvent(const Logger *logger,
                           Level level,
                           const QString &message) :
    QEvent(eventId),
    mLevel(level),
    mLogger(logger),
    mMessage(message),
    mNdc(NDC::peek()),
    mProperties(MDC::context()),
    mSequenceNumber(nextSequenceNumber()),
    mThreadName(),
    mTimeStamp(QDateTime::currentDateTime().toMSecsSinceEpoch())
{
    setThreadNameToCurrent();
}

LoggingEvent::LoggingEvent(const Logger *logger,
                           Level level,
                           const QString &message,
                           const MessageContext &context,
                           const QString &categoryName) :
       QEvent(eventId),
       mLevel(level),
       mLogger(logger),
       mMessage(message),
       mNdc(NDC::peek()),
       mProperties(MDC::context()),
       mSequenceNumber(nextSequenceNumber()),
       mThreadName(),
       mTimeStamp(QDateTime::currentDateTime().toMSecsSinceEpoch()),
       mContext(context),
       mCategoryName(categoryName)
{
    setThreadNameToCurrent();
}


LoggingEvent::LoggingEvent(const Logger *logger,
                           Level level,
                           const QString &message,
                           qint64 timeStamp) :
    QEvent(eventId),
    mLevel(level),
    mLogger(logger),
    mMessage(message),
    mNdc(NDC::peek()),
    mProperties(MDC::context()),
    mSequenceNumber(nextSequenceNumber()),
    mThreadName(),
    mTimeStamp(timeStamp)
{
    setThreadNameToCurrent();
}


LoggingEvent::LoggingEvent(const Logger *logger,
                           Level level,
                           const QString &message,
                           const QString &ndc,
                           const QHash<QString, QString> &properties,
                           const QString &threadName,
                           qint64 timeStamp) :
    QEvent(eventId),
    mLevel(level),
    mLogger(logger),
    mMessage(message),
    mNdc(ndc),
    mProperties(properties),
    mSequenceNumber(nextSequenceNumber()),
    mThreadName(threadName),
    mTimeStamp(timeStamp)
{
}

LoggingEvent::LoggingEvent(const Logger *logger,
                           Level level,
                           const QString &message,
                           const QString &ndc,
                           const QHash<QString, QString> &properties,
                           qint64 timeStamp,
                           const MessageContext &context,
                           const QString &categoryName)
    :
       QEvent(eventId),
       mLevel(level),
       mLogger(logger),
       mMessage(message),
       mNdc(ndc),
       mProperties(properties),
       mSequenceNumber(nextSequenceNumber()),
       mThreadName(),
       mTimeStamp(timeStamp),
       mContext(context),
       mCategoryName(categoryName)
{
    setThreadNameToCurrent();
}

LoggingEvent::LoggingEvent(const Logger *logger,
                           Level level,
                           const QString &message,
                           const QString &ndc,
                           const QHash<QString, QString> &properties,
                           const QString &threadName,
                           qint64 timeStamp,
                           const MessageContext &context,
                           const QString &categoryName)
    :
       QEvent(eventId),
       mLevel(level),
       mLogger(logger),
       mMessage(message),
       mNdc(ndc),
       mProperties(properties),
       mSequenceNumber(nextSequenceNumber()),
       mThreadName(threadName),
       mTimeStamp(timeStamp),
       mContext(context),
       mCategoryName(categoryName)
{
}

QString LoggingEvent::loggename() const
{
    if (mLogger)
        return mLogger->name();
    return QString();
}


QString LoggingEvent::toString() const
{
    return level().toString() + QLatin1Char(':') + message();
}


qint64 LoggingEvent::sequenceCount()
{
    QMutexLocker locker(sequence_guard());

    return msSequenceCount;
}


qint64 LoggingEvent::startTime()
{
    return InitialisationHelper::startTime();
}


void LoggingEvent::setThreadNameToCurrent()
{
    if (QThread::currentThread())
    {
        mThreadName = QThread::currentThread()->objectName();
        // if object name is not set use thread function address for thread identification
        if (mThreadName.isEmpty())
            mThreadName = QStringLiteral("0x%1").arg(reinterpret_cast<quintptr>(QThread::currentThread()), QT_POINTER_SIZE * 2, 16, QChar('0'));

    }
}


qint64 LoggingEvent::nextSequenceNumber()
{
    QMutexLocker locker(sequence_guard());

    return ++msSequenceCount;
}

MessageContext LoggingEvent::context() const
{
    return mContext;
}

void LoggingEvent::setContext(const MessageContext &context)
{
    mContext = context;
}

QString LoggingEvent::categoryName() const
{
    return mCategoryName;
}

void LoggingEvent::setCategoryName(const QString &categoryName)
{
    mCategoryName = categoryName;
}

qint64 LoggingEvent::msSequenceCount = 0;
const QEvent::Type LoggingEvent::eventId = static_cast<QEvent::Type>(QEvent::registerEventType());

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out, const LoggingEvent &loggingEvent)
{
    // version
    quint16 version = 0;
    out << version;
    // version 0 data
    out << loggingEvent.mLevel
           << loggingEvent.loggename()
           << loggingEvent.mMessage
           << loggingEvent.mNdc
           << loggingEvent.mProperties
           << loggingEvent.mSequenceNumber
           << loggingEvent.mThreadName
           << loggingEvent.mTimeStamp;

    return out;
}


QDataStream &operator>>(QDataStream &in, LoggingEvent &loggingEvent)
{
    // version
    quint16 version;
    in >> version;
    // Version 0 data
    QString logger;
    in >> loggingEvent.mLevel
       >> logger
       >> loggingEvent.mMessage
       >> loggingEvent.mNdc
       >> loggingEvent.mProperties
       >> loggingEvent.mSequenceNumber
       >> loggingEvent.mThreadName
       >> loggingEvent.mTimeStamp;
    if (logger.isEmpty())
        loggingEvent.mLogger = nullptr;
    else
        loggingEvent.mLogger = Logger::logger(logger);

    return in;
}
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt
