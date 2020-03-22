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

#ifndef LOG4QT_BINARYLOGGINGEVENT_H
#define LOG4QT_BINARYLOGGINGEVENT_H

#include "loggingevent.h"

namespace Log4Qt
{

class LOG4QT_EXPORT BinaryLoggingEvent : public LoggingEvent
{
public:
    BinaryLoggingEvent();
    BinaryLoggingEvent(const Logger *logger,
                       Level level,
                       const QByteArray &message);
    BinaryLoggingEvent(const Logger *logger,
                       Level level,
                       const QByteArray &message,
                       qint64 timeStamp);
    BinaryLoggingEvent(const Logger *logger,
                       Level level,
                       const QByteArray &message,
                       const QString &ndc,
                       const QHash<QString, QString> &properties,
                       const QString &threadName,
                       qint64 timeStamp);
    QByteArray binaryMessage() const;

    QString toString() const;

    static QString binaryMarker();

private:
    QByteArray mBinaryMessage;

#ifndef QT_NO_DATASTREAM
    // Needs to be friend to stream objects
    friend LOG4QT_EXPORT QDataStream &operator<<(QDataStream &out, const BinaryLoggingEvent &loggingEvent);
    friend LOG4QT_EXPORT QDataStream &operator>>(QDataStream &in, BinaryLoggingEvent &loggingEvent);
#endif // QT_NO_DATASTREAM
};

#ifndef QT_NO_DATASTREAM
LOG4QT_EXPORT QDataStream &operator<<(QDataStream &out, const BinaryLoggingEvent &loggingEvent);
LOG4QT_EXPORT QDataStream &operator>>(QDataStream &in, BinaryLoggingEvent &loggingEvent);
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt

Q_DECLARE_METATYPE(Log4Qt::BinaryLoggingEvent)
Q_DECLARE_TYPEINFO(Log4Qt::BinaryLoggingEvent, Q_MOVABLE_TYPE);

#endif // LOG4QT_BINARYLOGGINGEVENT_H
