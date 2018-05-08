/******************************************************************************
 *
 * package:     Log4Qt
 * file:        datetime.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:     Sep 2008, Martin Heinrich:
 *              - Resolved compilation problem with Microsoft Visual Studio 2005
 *
 *
 * Copyright 2007 - 2008 Martin Heinrich
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

#include <log4qt/log4qtshared.h>

#include <QDateTime>

namespace Log4Qt
{

/*!
 * \brief The class DateTime provides extended functionality for QDateTime.
 *
 * The class DateTime implements additional formatting options for
 * toString() and provides conversion functions from and to milliseconds.
 */
class LOG4QT_EXPORT  DateTime : public QDateTime
{
public:
    /*!
     * Constructs a null date time.
     *
     * \sa QDateTime::QDateTime()
     */
    DateTime();

    /*!
     * Constructs a copy of another QDateTime.
     *
     * \sa QDateTime::QDateTime(const QDateTime &other)
     */
    DateTime(const QDateTime &other);

    /*!
     * Constructs a datetime with the given \a date and \a time, using
     * the time specification defined by \a timeSpec.
     *
     * \sa QDateTime::QDateTime(const QDate &date, const QTime &time,
     *     Qt::TimeSpec timeSpec = Qt::LocalTime)
     */
    DateTime(const QDate &date,
             const QTime &time,
             Qt::TimeSpec timeSpec = Qt::LocalTime);

    /*!
     * Assigns \a other to this DateTime and returns a reference to it.
     */
    DateTime &operator=(const DateTime &other);

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
     * Returns the current datetime, as reported by the system clock, in
     * the local time zone.
     *
     * \sa QDateTime::currentDateTime()
     */
    static DateTime currentDateTime();
    static DateTime fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec, int offsetSeconds = 0);
    static DateTime fromMSecsSinceEpoch(qint64 msecs);

private:
    QString formatDateTime(const QString &format) const;
};

inline DateTime::DateTime() : QDateTime()
{}

inline DateTime::DateTime(const QDateTime &other) : QDateTime(other)
{}

inline DateTime::DateTime(const QDate &date,
                          const QTime &time,
                          Qt::TimeSpec timeSpec) :
    QDateTime(date, time, timeSpec)
{}

inline DateTime &DateTime::operator=(const DateTime &other)
{
    QDateTime::operator=(other);
    return *this;
}

inline DateTime DateTime::currentDateTime()
{
    return DateTime(QDateTime::currentDateTime());
}

inline DateTime DateTime::fromMSecsSinceEpoch(qint64 msecs)
{
    return DateTime(QDateTime::fromMSecsSinceEpoch(msecs));
}

inline DateTime DateTime::fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec, int offsetSeconds)
{
    return DateTime(QDateTime::fromMSecsSinceEpoch(msecs, spec, offsetSeconds));

}

} // namespace Log4Qt

Q_DECLARE_TYPEINFO(Log4Qt::DateTime, Q_MOVABLE_TYPE);

#endif // LOG4QT_HELPERS_DATETIME_H
