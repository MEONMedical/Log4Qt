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

#include "helpers/cronexpression.h"

#include <QStringList>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

CronExpression::CronExpression(const QString &expression)
{
    mValid = parse(expression);
}

bool CronExpression::parse(const QString &expression)
{
    const QStringList fields = expression.simplified().split(u' ', Qt::SkipEmptyParts);
    if (fields.size() != 6)
    {
        mErrorString = u"Expected 6 fields (seconds minutes hours day-of-month month day-of-week), got %1"_s
                            .arg(fields.size());
        return false;
    }

    // seconds: 0-59
    if (!parseField(fields[0], mSeconds, 0, 59))
        return false;

    // minutes: 0-59
    if (!parseField(fields[1], mMinutes, 0, 59))
        return false;

    // hours: 0-23
    std::bitset<60> hourBits;
    if (!parseField(fields[2], hourBits, 0, 23))
        return false;
    for (int i = 0; i < 24; ++i)
        mHours[i] = hourBits[i];

    // day-of-month: 1-31
    std::bitset<60> domBits;
    if (!parseField(fields[3], domBits, 1, 31))
        return false;
    for (int i = 1; i <= 31; ++i)
        mDaysOfMonth[i] = domBits[i];

    // month: 1-12 (with name substitution)
    QString monthField = fields[4].toUpper();
    static const QStringList monthNames = {
        u"JAN"_s, u"FEB"_s, u"MAR"_s, u"APR"_s, u"MAY"_s, u"JUN"_s,
        u"JUL"_s, u"AUG"_s, u"SEP"_s, u"OCT"_s, u"NOV"_s, u"DEC"_s
    };
    for (int i = 0; i < monthNames.size(); ++i)
        monthField.replace(monthNames[i], QString::number(i + 1));

    std::bitset<60> monthBits;
    if (!parseField(monthField, monthBits, 1, 12))
        return false;
    for (int i = 1; i <= 12; ++i)
        mMonths[i] = monthBits[i];

    // day-of-week: 1-7 (1=SUN, with name substitution)
    QString dowField = fields[5].toUpper();
    static const QStringList dowNames = {
        u"SUN"_s, u"MON"_s, u"TUE"_s, u"WED"_s, u"THU"_s, u"FRI"_s, u"SAT"_s
    };
    for (int i = 0; i < dowNames.size(); ++i)
        dowField.replace(dowNames[i], QString::number(i + 1));

    std::bitset<60> dowBits;
    if (!parseField(dowField, dowBits, 1, 7))
        return false;
    for (int i = 1; i <= 7; ++i)
        mDaysOfWeek[i] = dowBits[i];

    return true;
}

bool CronExpression::parseField(const QString &field, std::bitset<60> &bits, int minVal, int maxVal)
{
    bits.reset();

    if (field == u"?"_s || field == u"*"_s)
    {
        for (int i = minVal; i <= maxVal; ++i)
            bits.set(i);
        return true;
    }

    // Split on comma for list support: "1,5,10"
    const QStringList parts = field.split(u',', Qt::SkipEmptyParts);
    for (const QString &part : parts)
    {
        int stepStart = -1;
        int stepEnd = -1;
        int step = 1;

        // Check for step: "*/5" or "1-30/5"
        QString rangePart = part;
        const int slashIdx = part.indexOf(u'/');
        if (slashIdx >= 0)
        {
            rangePart = part.left(slashIdx);
            bool ok = false;
            step = part.mid(slashIdx + 1).toInt(&ok);
            if (!ok || step <= 0)
            {
                mErrorString = u"Invalid step value in '%1'"_s.arg(part);
                return false;
            }
        }

        if (rangePart == u"*"_s || rangePart == u"?"_s)
        {
            stepStart = minVal;
            stepEnd = maxVal;
        }
        else
        {
            // Check for range: "1-5"
            const int dashIdx = rangePart.indexOf(u'-');
            if (dashIdx >= 0)
            {
                bool ok1 = false, ok2 = false;
                stepStart = rangePart.left(dashIdx).toInt(&ok1);
                stepEnd = rangePart.mid(dashIdx + 1).toInt(&ok2);
                if (!ok1 || !ok2)
                {
                    mErrorString = u"Invalid range in '%1'"_s.arg(part);
                    return false;
                }
            }
            else
            {
                // Single value
                bool ok = false;
                stepStart = rangePart.toInt(&ok);
                if (!ok)
                {
                    mErrorString = u"Invalid value in '%1'"_s.arg(part);
                    return false;
                }
                stepEnd = (slashIdx >= 0) ? maxVal : stepStart;
            }
        }

        if (stepStart < minVal || stepEnd > maxVal || stepStart > stepEnd)
        {
            mErrorString = u"Value out of range [%1-%2] in '%3'"_s.arg(minVal).arg(maxVal).arg(part);
            return false;
        }

        for (int i = stepStart; i <= stepEnd; i += step)
            bits.set(i);
    }

    return true;
}

QDateTime CronExpression::nextFireTime(const QDateTime &from) const
{
    if (!mValid)
        return {};

    // Start from the next second after 'from'
    QDateTime dt = from.addSecs(1);
    // Clear sub-second part
    dt = QDateTime(dt.date(), QTime(dt.time().hour(), dt.time().minute(), dt.time().second()));

    // Search up to 4 years to prevent infinite loops
    const QDateTime limit = from.addYears(4);

    while (dt < limit)
    {
        // Check month
        if (!mMonths.test(dt.date().month()))
        {
            // Advance to first day of next month
            dt = QDateTime(QDate(dt.date().year(), dt.date().month(), 1).addMonths(1), QTime(0, 0, 0));
            continue;
        }

        // Check day-of-month
        if (!mDaysOfMonth.test(dt.date().day()))
        {
            dt = QDateTime(dt.date().addDays(1), QTime(0, 0, 0));
            continue;
        }

        // Check day-of-week (Qt: 1=Mon..7=Sun, Quartz: 1=Sun,2=Mon..7=Sat)
        int qtDow = dt.date().dayOfWeek(); // 1=Mon..7=Sun
        int quartzDow = (qtDow == 7) ? 1 : qtDow + 1; // convert to 1=Sun
        if (!mDaysOfWeek.test(quartzDow))
        {
            dt = QDateTime(dt.date().addDays(1), QTime(0, 0, 0));
            continue;
        }

        // Check hour
        if (!mHours.test(dt.time().hour()))
        {
            // Advance to next hour
            dt = dt.addSecs(3600 - dt.time().minute() * 60 - dt.time().second());
            continue;
        }

        // Check minute
        if (!mMinutes.test(dt.time().minute()))
        {
            dt = dt.addSecs(60 - dt.time().second());
            continue;
        }

        // Check second
        if (!mSeconds.test(dt.time().second()))
        {
            dt = dt.addSecs(1);
            continue;
        }

        return dt;
    }

    return {}; // No match found within search window
}

} // namespace Log4Qt
