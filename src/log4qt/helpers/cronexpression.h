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

#ifndef LOG4QT_HELPERS_CRONEXPRESSION_H
#define LOG4QT_HELPERS_CRONEXPRESSION_H

#include "log4qt/log4qt.h"

#include <QDateTime>
#include <QString>

#include <bitset>

namespace Log4Qt
{

/*!
 * \brief Parses and evaluates Quartz-style cron expressions.
 *
 * Supports 6 fields: seconds minutes hours day-of-month month day-of-week
 *
 * Field ranges:
 * - seconds:      0-59
 * - minutes:      0-59
 * - hours:        0-23
 * - day-of-month: 1-31
 * - month:        1-12 (or JAN-DEC)
 * - day-of-week:  1-7  (1=SUN, or SUN-SAT)
 *
 * Supported specifiers: * , - / ?
 *
 * Example: "0 0 0 * * ?" fires at midnight every day.
 */
class LOG4QT_EXPORT CronExpression
{
public:
    CronExpression() = default;
    explicit CronExpression(const QString &expression);

    bool isValid() const { return mValid; }
    QString errorString() const { return mErrorString; }

    /*!
     * Returns the next QDateTime at or after \a from that matches
     * the cron expression. Returns an invalid QDateTime if no match
     * is found within a reasonable search window (4 years).
     */
    QDateTime nextFireTime(const QDateTime &from) const;

private:
    bool parse(const QString &expression);
    bool parseField(const QString &field, std::bitset<60> &bits, int minVal, int maxVal);

    static int monthNameToNumber(const QString &name);
    static int dowNameToNumber(const QString &name);

    std::bitset<60> mSeconds;
    std::bitset<60> mMinutes;
    std::bitset<24> mHours;
    std::bitset<32> mDaysOfMonth;   // bit 1..31
    std::bitset<13> mMonths;        // bit 1..12
    std::bitset<8>  mDaysOfWeek;    // bit 1..7  (1=Sunday)

    bool mValid = false;
    QString mErrorString;
};

} // namespace Log4Qt

#endif // LOG4QT_HELPERS_CRONEXPRESSION_H
