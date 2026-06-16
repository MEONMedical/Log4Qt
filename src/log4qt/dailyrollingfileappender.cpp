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

#include "dailyrollingfileappender.h"

#include "helpers/datetime.h"
#include "loggingevent.h"
#include "spi/daterolloverstrategy.h"

namespace Log4Qt
{

constexpr char defaultDatePattern[] = "_yyyy_MM_dd";

DailyRollingFileAppender::~DailyRollingFileAppender()
{
    // deleteLater() defers QObject destruction, so the strategy's QFutureSynchronizer
    // would not wait synchronously if we relied on the shared_ptr's deleter alone.
    // Explicitly wait here, while the strategy is still alive.
    if (auto *strategy = qobject_cast<DateRolloverStrategy *>(rolloverStrategy().get()))
        strategy->waitForCleanup();
}

DailyRollingFileAppender::DailyRollingFileAppender(QObject *parent)
    : RollingFileAppender(parent)
    , mDatePattern(defaultDatePattern)
    , mKeepDays(0)
{
}

DailyRollingFileAppender::DailyRollingFileAppender(const LayoutSharedPtr &layout, const QString &fileName, const QString &datePattern, const int keepDays, QObject *parent)
    : RollingFileAppender(layout, fileName, parent)
    , mDatePattern(datePattern.isEmpty() ? defaultDatePattern : datePattern)
    , mKeepDays(keepDays)
{
}

QString DailyRollingFileAppender::datePattern() const
{
    QMutexLocker locker(&mObjectGuard);
    return mDatePattern;
}

void DailyRollingFileAppender::setDatePattern(const QString &datePattern)
{
    QMutexLocker locker(&mObjectGuard);
    mDatePattern = datePattern;
}

int DailyRollingFileAppender::keepDays() const
{
    QMutexLocker locker(&mObjectGuard);
    return mKeepDays;
}

void DailyRollingFileAppender::setKeepDays(const int keepDays)
{
    QMutexLocker locker(&mObjectGuard);
    mKeepDays = keepDays;
}

void DailyRollingFileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (mOriginalFilename.isEmpty())
        mOriginalFilename = file();

    // Set up DateRolloverStrategy in Embedded mode for filename construction.
    auto *strategy = new DateRolloverStrategy;
    strategy->setMode(DateRolloverStrategy::NamingMode::Embedded);
    strategy->setDatePattern(mDatePattern);
    strategy->setKeepDays(mKeepDays);
    setRolloverStrategy(RolloverStrategySharedPtr(strategy));

    mLastDate = DateTime::currentDateTime().date();
    closeFile();
    setFile(strategy->rollover(mOriginalFilename));
    strategy->waitForCleanup();

    FileAppender::activateOptions();
}

void DailyRollingFileAppender::append(const LoggingEvent &event)
{
    const auto currentDate(DateTime::currentDateTime().date());

    if (currentDate != mLastDate)
    {
        mLastDate = currentDate;

        // Reset to base filename so the strategy receives the un-dated name.
        setFile(mOriginalFilename);
        rollOver();
    }
    FileAppender::append(event);
}

}

#include "moc_dailyrollingfileappender.cpp"
