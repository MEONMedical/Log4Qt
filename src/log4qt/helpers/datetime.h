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

#ifndef LOG4QT_HELPERS_DATETIME_H
#define LOG4QT_HELPERS_DATETIME_H

#include "log4qt/log4qtshared.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QTimeZone>
#include <functional>

namespace Log4Qt
{

/*!
 * \brief The class DateTime provides extended functionality for QDateTime.
 *
 * The class DateTime implements additional formatting options for
 * toString() and provides conversion functions from and to milliseconds.
 */
class LOG4QT_EXPORT DateTime : public QDateTime
{
public:
    /*!
     * Callable type used as an injectable time source.
     *
     * Components that need testable wall-clock access (e.g.
     * \c DailyRollingFileAppender, \c DateRolloverStrategy) accept a
     * \c Provider so tests can substitute a lambda that returns a
     * controlled \c QDateTime without touching global state.
     *
     * The default value for such components is always
     * \c []{ return QDateTime::currentDateTime(); }.
     */
    using Provider = std::function<QDateTime()>;
    /*!
     * Constructs a null date time.
     *
     * \sa QDateTime::QDateTime()
     */
    DateTime();

    ~DateTime();

    /*!
     * Constructs a copy of another QDateTime.
     *
     * \sa QDateTime::QDateTime(const QDateTime &other)
     */
    DateTime(const QDateTime &other) : QDateTime(other)
    {}

    DateTime(const DateTime &other) noexcept;
    DateTime(DateTime &&other) noexcept = default;

    /*!
     * Constructs a datetime with the given \a date and \a time, using
     * the time zone defined by \a QTimeZone.
     *
     * \sa QDateTime::QDateTime(const QDate &date, const QTime &time,
     *     QTimeZone = QTimeZone(QTimeZone::LocalTime))
     */
    DateTime(QDate date,
             QTime time,
             QTimeZone timeZone = QTimeZone(QTimeZone::LocalTime)) :
        QDateTime(date, time, timeZone)
    {}

    /*!
     * Assigns \a other to this DateTime and returns a reference to it.
     */
    DateTime &operator=(const DateTime &other) noexcept
    {
        QDateTime::operator=(other);
        return *this;
    }
    DateTime &operator=(DateTime &&other) noexcept = default;

    /*!
     * Returns the datetime as a string. The \a format parameter
     * determines the format of the result string.
     *
     *
     * Alternatively the \a format parameter can specify one of the
     * following strings.
     *
     * <table align="center" border="1" cellpadding="2" cellspacing="0" bordercolor="#84b0c7">
     * <tr bgcolor="#d5e1e8">
     * <th width="20%"> String </th>
     * <th> Format </th>
     * </tr><tr>
     * <td> ABSOLUTE </td>
     * <td> uses the format HH:mm:ss.zzz </td>
     * </tr><tr bgcolor="#ffffff">
     * <td> DATE </td>
     * <td> uses the format dd MMM YYYY HH:mm:ss.zzz </td>
     * </tr><tr>
     * <td> ISO8601 </td>
     * <td> uses the format yyyy-MM-dd hh:mm:ss.zzz </td>
     * </tr><tr bgcolor="#ffffff">
     * <td> NONE </td>
     * <td> uses an empty string as format </td>
     * </tr><tr bgcolor="#ffffff">
     * <td> RELATIVE </td>
     * <td> returns the milliseconds since start of the program</td>
     * </tr>
     * </table>
     *
     * \sa QDateTime::toString(const QString &format)
     */
    QString toString(const QString &format) const;

    /*!
     * Formats an epoch millisecond timestamp to a string using \a format.
     *
     * This is the preferred fast path for callers that already hold a raw
     * \c qint64 timestamp (e.g. \c LoggingEvent::timeStamp()), because it avoids
     * the redundant \c toMSecsSinceEpoch() round-trip of the instance overload.
     * Results for the named formats ISO8601, ABSOLUTE, and DATE are cached
     * per calling thread (thread-local, keyed by epoch millisecond): repeated
     * calls within the same millisecond cost only a \c qint64 comparison and a
     * string copy (~1 ns). On a cache miss, formatting delegates to
     * \c QDateTime::toString() as usual.
     *
     * For custom (non-named) format strings the method always calls
     * \c QDateTime::toString(), equivalent to the instance overload.
     *
     * \sa toString(const QString &format)
     */
    static QString formatMsecs(qint64 msecs, const QString &format);

    /*!
     * Returns the current datetime, as reported by the system clock, in
     * the local time zone.
     *
     * \sa QDateTime::currentDateTime()
     */
    static DateTime currentDateTime();

    /*!
     * Returns the current time as milliseconds since the Unix epoch.
     *
     * Uses a thread-local cache keyed by a monotonic \c QElapsedTimer so
     * that repeated calls within the same cache window (default 1 ms) cost
     * only a \c qint64 comparison and avoid a system call.
     *
     * \sa setCacheWindow(), cacheWindow()
     */
    static qint64 currentMSecsSinceEpoch();

    /*!
     * Sets the cache window for \c currentMSecsSinceEpoch() in milliseconds.
     *
     * \a cacheWindowMs == 0 disables caching (maximum precision).
     * Values of 1–100 give the best balance of performance and precision.
     * The default is 1 ms.
     */
    static void setCacheWindow(qint64 cacheWindowMs);

    /*!
     * Returns the current cache window in milliseconds.
     */
    static qint64 cacheWindow();

    /*!
     * Sets the global date/time provider used by \c currentDateTime() and
     * \c currentMSecsSinceEpoch().
     *
     * Passing a null (default-constructed) \c Provider resets to the built-in
     * default of \c QDateTime::currentDateTime().
     *
     * Thread-safe. Intended for use in tests — set once before any threads
     * start logging, reset in cleanup.
     */
    static void setProvider(Provider provider);

    static DateTime fromMSecsSinceEpoch(qint64 msecs, QTimeZone timeZone)
    {
        return DateTime(QDateTime::fromMSecsSinceEpoch(msecs, timeZone));
    }

    static DateTime fromMSecsSinceEpoch(qint64 msecs)
    {
        return DateTime(QDateTime::fromMSecsSinceEpoch(msecs));
    }

private:
    QString formatDateTime(const QString &format) const;
};

} // namespace Log4Qt

Q_DECLARE_TYPEINFO(Log4Qt::DateTime, Q_MOVABLE_TYPE);

#endif // LOG4QT_HELPERS_DATETIME_H
