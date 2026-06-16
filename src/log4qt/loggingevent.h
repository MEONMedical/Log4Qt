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

#ifndef LOG4QT_LOG4QTEVENT_H
#define LOG4QT_LOG4QTEVENT_H

#include "level.h"

#include <QHash>
#include <QStringList>
#include <QEvent>

#include <atomic>
#ifdef __cpp_lib_source_location
#include <source_location>
#endif

#include <QSharedDataPointer>

namespace Log4Qt
{

class Logger;

class MessageContext
{
public:
    constexpr explicit MessageContext() noexcept
        : file(nullptr), line(-1), function(nullptr) {}
    constexpr explicit MessageContext(const char *fileName, int lineNumber, const char *functionName) noexcept
        : file(fileName), line(lineNumber), function(functionName) {}
#ifdef __cpp_lib_source_location
    explicit MessageContext(const std::source_location &loc)
        : file(loc.file_name()), line(static_cast<int>(loc.line())), function(loc.function_name()) {}
#endif
    const char *file;
    int line;
    const char *function;
};

/*!
 * \brief The class LoggingEvent is the internal representation of a
 *        logging event.
 *
 * The class uses milliseconds since 1970-01-01T00:00:00, Coordinated
 * Universal Time for time values. For converstion from and to QDateTime
 * use DateTime.
 */
class LOG4QT_EXPORT LoggingEvent : public QEvent
{
public:
    static const QEvent::Type eventId;
    LoggingEvent();
    virtual ~LoggingEvent();

    // QSharedDataPointer copy/assign is non-throwing (atomic refcount op only).
    LoggingEvent(const LoggingEvent &other) noexcept;
    LoggingEvent &operator=(const LoggingEvent &other) noexcept;
    LoggingEvent(LoggingEvent &&other) noexcept = default;
    LoggingEvent &operator=(LoggingEvent &&other) noexcept = default;

    LoggingEvent(const Logger *logger,
                 Level level,
                 const QString &message);
    LoggingEvent(const Logger *logger,
                 Level level,
                 const QString &message,
                 const MessageContext &context,
                 const QString &categoryName);
    LoggingEvent(const Logger *logger,
                 Level level,
                 const QString &message,
                 qint64 timeStamp);
    LoggingEvent(const Logger *logger,
                 Level level,
                 const QString &message,
                 const QString &ndc,
                 const QHash<QString, QString> &properties,
                 const QString &threadName,
                 qint64 timeStamp);
    LoggingEvent(const Logger *logger,
                 Level level,
                 const QString &message,
                 const QString &ndc,
                 const QHash<QString, QString> &properties,
                 qint64 timeStamp,
                 const MessageContext &context,
                 const QString &categoryName);
    LoggingEvent(const Logger *logger,
                 Level level,
                 const QString &message,
                 const QString &ndc,
                 const QHash<QString, QString> &properties,
                 const QString &threadName,
                 qint64 timeStamp,
                 const MessageContext &context,
                 const QString &categoryName);
    
    // Move-enabled constructors for zero-copy construction
    LoggingEvent(const Logger *logger,
                 Level level,
                 QString &&message);
    LoggingEvent(const Logger *logger,
                 Level level,
                 QString &&message,
                 const MessageContext &context,
                 QString &&categoryName);

    [[nodiscard]] Level level() const;
    // LocationInformation locationInformation() const;
    [[nodiscard]] const Logger *logger() const;
    [[nodiscard]] QString message() const;
    [[nodiscard]] QHash<QString, QString> mdc() const;
    [[nodiscard]] QString ndc() const;
    [[nodiscard]] QHash<QString, QString> properties() const;
    [[nodiscard]] qint64 sequenceNumber() const;
    [[nodiscard]] QString threadName() const;
    [[nodiscard]] qint64 timeStamp() const;

    [[nodiscard]] QString loggername() const;
    [[nodiscard]] QString property(const QString &key) const;
    [[nodiscard]] QStringList propertyKeys() const;
    void setProperty(const QString &key, const QString &value);
    QString toString() const;
    static qint64 sequenceCount();
    static qint64 startTime();

    [[nodiscard]] int lineNumber() const;
    void setLineNumber(int lineNumber);
    [[nodiscard]] QString fileName() const;
    void setFileName(const QString &fileName);
    [[nodiscard]] QString functionName() const;
    void setMethodName(const QString &functionName);
    [[nodiscard]] QString categoryName() const;
    void setCategoryName(const QString &categoryName);
    [[nodiscard]] MessageContext context() const;
    void setContext(const MessageContext &context);

private:
    void setThreadNameToCurrent();
    static qint64 nextSequenceNumber();

private:
    struct Data : public QSharedData
    {
        Data();
        Data(const Logger *logger,
             Level level,
             QString message);
        Data(const Logger *logger,
             Level level,
             QString message,
             const MessageContext &context,
             QString categoryName);
        Data(const Logger *logger,
             Level level,
             QString message,
             const QString &ndc,
             const QHash<QString, QString> &properties,
             const QString &threadName,
             qint64 timeStamp,
             const MessageContext &context,
             const QString &categoryName);

        Level mLevel;
        const Logger *mLogger;
        QString mMessage;
        QString mNdc;
        QHash<QString, QString> mProperties;
        qint64 mSequenceNumber;
        QString mThreadName;
        qint64 mTimeStamp;
        MessageContext mContext;
        QString mCategoryName;
    };

    QSharedDataPointer<Data> d;

    static std::atomic<qint64> msSequenceCount;

#ifndef QT_NO_DATASTREAM
    // Needs to be friend to stream objects
    friend LOG4QT_EXPORT QDataStream &operator<<(QDataStream &out,
                                                const LoggingEvent &loggingEvent);
    friend LOG4QT_EXPORT QDataStream &operator>>(QDataStream &in,
                                                 LoggingEvent &loggingEvent);
#endif // QT_NO_DATASTREAM
};

#ifndef QT_NO_DATASTREAM
/*!
 * \relates LoggingEvent
 *
 * Writes the given error \a rLoggingEvent to the given stream \a rStream,
 * and returns a reference to the stream.
 */
LOG4QT_EXPORT QDataStream &operator<<(QDataStream &out,
                                      const LoggingEvent &loggingEvent);

/*!
 * \relates LoggingEvent
 *
 * Reads an error from the given stream \a rStream into the given
 * error \a rLoggingEvent, and returns a reference to the stream.
 */
LOG4QT_EXPORT QDataStream &operator>>(QDataStream &in,
                                      LoggingEvent &loggingEvent);
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt


Q_DECLARE_METATYPE(Log4Qt::LoggingEvent)
Q_DECLARE_TYPEINFO(Log4Qt::LoggingEvent, Q_RELOCATABLE_TYPE);


#endif // LOG4QT_LOG4QTEVENT_H
