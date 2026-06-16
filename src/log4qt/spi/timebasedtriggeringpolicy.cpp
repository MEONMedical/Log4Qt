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

#include "spi/timebasedtriggeringpolicy.h"

#include "helpers/datetime.h"

#include <QRandomGenerator>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

TimeBasedTriggeringPolicy::TimeBasedTriggeringPolicy(QObject *parent) :
    TriggeringPolicy(parent),
    mDatePattern(u"'.'yyyy-MM-dd"_s),
    mFrequency(Frequency::Daily)
{
}

void TimeBasedTriggeringPolicy::activateOptions()
{
    if (mInterval < 1)
        mInterval = 1;
    if (mMaxRandomDelay < 0)
        mMaxRandomDelay = 0;

    computeFrequency();
    if (!mActiveDatePattern.isEmpty())
        computeRollOverTime();
}

bool TimeBasedTriggeringPolicy::isTriggeringEvent(QIODevice *activeDevice,
                                                    const LoggingEvent &event)
{
    Q_UNUSED(activeDevice)
    Q_UNUSED(event)

    if (mActiveDatePattern.isEmpty())
        return false;

    if (DateTime::currentDateTime() > mRollOverTime)
    {
        computeRollOverTime();
        return true;
    }
    return false;
}

void TimeBasedTriggeringPolicy::computeFrequency()
{
    mActiveDatePattern.clear();

    bool foundMinutes = false;
    bool foundHours = false;
    bool foundAmPm = false;
    bool foundDay = false;
    bool foundWeek = false;
    bool foundMonth = false;

    bool inQuote = false;
    const int len = mDatePattern.size();
    int i = 0;

    while (i < len)
    {
        const QChar ch = mDatePattern.at(i);

        if (ch == u'\'')
        {
            if (i + 1 < len && mDatePattern.at(i + 1) == u'\'')
            {
                i += 2;
                continue;
            }
            inQuote = !inQuote;
            ++i;
            continue;
        }

        if (inQuote)
        {
            ++i;
            continue;
        }

        // Skip consecutive identical characters (greedy tokenization)
        while (i < len && mDatePattern.at(i) == ch)
            ++i;

        if (ch == u'm')
            foundMinutes = true;
        else if (ch == u'h' || ch == u'H')
            foundHours = true;
        else if (ch == u'a' || ch == u'A')
            foundAmPm = true;
        else if (ch == u'd')
            foundDay = true;
        else if (ch == u'w')
            foundWeek = true;
        else if (ch == u'M')
            foundMonth = true;
    }

    if (foundMinutes)
        mFrequency = Frequency::Minutely;
    else if (foundHours)
        mFrequency = Frequency::Hourly;
    else if (foundAmPm)
        mFrequency = Frequency::HalfDaily;
    else if (foundDay)
        mFrequency = Frequency::Daily;
    else if (foundWeek)
        mFrequency = Frequency::Weekly;
    else if (foundMonth)
        mFrequency = Frequency::Monthly;
    else
        return;

    mActiveDatePattern = mDatePattern;
}

void TimeBasedTriggeringPolicy::computeRollOverTime()
{
    Q_ASSERT_X(!mActiveDatePattern.isEmpty(), "TimeBasedTriggeringPolicy::computeRollOverTime()", "No active date pattern");

    QDateTime now = DateTime::currentDateTime();
    QDate nowDate = now.date();
    QTime nowTime = now.time();

    if (mModulate)
    {
        switch (mFrequency)
        {
        case Frequency::Minutely:
        {
            // Align to interval-minute boundaries within the hour
            int minuteOfHour = nowTime.minute();
            int nextBucket = (minuteOfHour / mInterval + 1) * mInterval;
            QDateTime hourStart(nowDate, QTime(nowTime.hour(), 0, 0, 0));
            mRollOverTime = hourStart.addSecs(nextBucket * 60);
            break;
        }
        case Frequency::Hourly:
        {
            // Align to interval-hour boundaries within the day
            int hourOfDay = nowTime.hour();
            int nextBucket = (hourOfDay / mInterval + 1) * mInterval;
            QDateTime dayStart(nowDate, QTime(0, 0, 0, 0));
            mRollOverTime = dayStart.addSecs(nextBucket * 3600);
            break;
        }
        case Frequency::HalfDaily:
        {
            // 12-hour blocks from midnight
            int halfDayIndex = nowTime.hour() / 12;
            int nextBucket = (halfDayIndex / mInterval + 1) * mInterval;
            QDateTime dayStart(nowDate, QTime(0, 0, 0, 0));
            mRollOverTime = dayStart.addSecs(nextBucket * 12 * 3600);
            break;
        }
        case Frequency::Daily:
        {
            // Align to interval-day boundaries from a fixed epoch
            const QDate epoch(2000, 1, 1);
            qint64 daysSinceEpoch = epoch.daysTo(nowDate);
            qint64 nextBucket = (daysSinceEpoch / mInterval + 1) * mInterval;
            mRollOverTime = QDateTime(epoch.addDays(nextBucket), QTime(0, 0, 0, 0));
            break;
        }
        case Frequency::Weekly:
        {
            // Align to interval-week boundaries from a known Sunday
            const QDate epochSunday(2000, 1, 2); // 2000-01-02 is a Sunday
            qint64 daysSinceEpoch = epochSunday.daysTo(nowDate);
            qint64 weeksSinceEpoch = daysSinceEpoch / 7;
            qint64 nextBucket = (weeksSinceEpoch / mInterval + 1) * mInterval;
            mRollOverTime = QDateTime(epochSunday.addDays(nextBucket * 7), QTime(0, 0, 0, 0));
            break;
        }
        case Frequency::Monthly:
        {
            // Align to interval-month boundaries from January 2000
            int monthsSinceEpoch = (nowDate.year() - 2000) * 12 + (nowDate.month() - 1);
            int nextBucket = (monthsSinceEpoch / mInterval + 1) * mInterval;
            int targetYear = 2000 + nextBucket / 12;
            int targetMonth = (nextBucket % 12) + 1;
            mRollOverTime = QDateTime(QDate(targetYear, targetMonth, 1), QTime(0, 0, 0, 0));
            break;
        }
        }
    }
    else
    {
        switch (mFrequency)
        {
        case Frequency::Minutely:
        {
            QDateTime start(nowDate, QTime(nowTime.hour(), nowTime.minute(), 0, 0));
            mRollOverTime = start.addSecs(mInterval * 60);
            break;
        }
        case Frequency::Hourly:
        {
            QDateTime start(nowDate, QTime(nowTime.hour(), 0, 0, 0));
            mRollOverTime = start.addSecs(mInterval * 3600);
            break;
        }
        case Frequency::HalfDaily:
        {
            int hour = nowTime.hour() >= 12 ? 12 : 0;
            QDateTime start(nowDate, QTime(hour, 0, 0, 0));
            mRollOverTime = start.addSecs(mInterval * 12 * 3600);
            break;
        }
        case Frequency::Daily:
        {
            QDateTime start(nowDate, QTime(0, 0, 0, 0));
            mRollOverTime = start.addDays(mInterval);
            break;
        }
        case Frequency::Weekly:
        {
            int day = nowDate.dayOfWeek();
            if (day == Qt::Sunday)
                day = 0;
            QDateTime start = QDateTime(nowDate, QTime(0, 0, 0, 0)).addDays(-1 * day);
            mRollOverTime = start.addDays(mInterval * 7);
            break;
        }
        case Frequency::Monthly:
        {
            QDateTime start(QDate(nowDate.year(), nowDate.month(), 1), QTime(0, 0, 0, 0));
            mRollOverTime = start.addMonths(mInterval);
            break;
        }
        }
    }

    // Apply random delay
    if (mMaxRandomDelay > 0)
    {
        int delay = QRandomGenerator::global()->bounded(mMaxRandomDelay + 1);
        mRollOverTime = mRollOverTime.addSecs(delay);
    }
}

} // namespace Log4Qt

#include "moc_timebasedtriggeringpolicy.cpp"
