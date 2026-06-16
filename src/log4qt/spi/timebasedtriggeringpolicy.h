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

#ifndef LOG4QT_TIMEBASEDTRIGGERINGPOLICY_H
#define LOG4QT_TIMEBASEDTRIGGERINGPOLICY_H

#include "triggeringpolicy.h"

#include <QDateTime>

namespace Log4Qt
{

/*!
 * \brief The class TimeBasedTriggeringPolicy triggers a rollover based on
 *        a time/date pattern.
 *
 * The date pattern determines the rollover frequency. The policy triggers
 * when the current time passes the computed rollover time.
 *
 * Supported frequencies (determined automatically from the pattern):
 * - Minutely: pattern changes every minute
 * - Hourly: pattern changes every hour
 * - HalfDaily: pattern changes every 12 hours
 * - Daily: pattern changes every day
 * - Weekly: pattern changes every week
 * - Monthly: pattern changes every month
 *
 * The \c interval property multiplies the base frequency unit. For example,
 * with an hourly pattern and interval=4, rollover occurs every 4 hours.
 *
 * When \c modulate is true, rollover times are aligned to calendar
 * boundaries. For example, with interval=4 and hourly frequency,
 * rollovers align to 00:00, 04:00, 08:00, 12:00, 16:00, 20:00.
 *
 * The \c maxRandomDelay property adds a random delay in seconds to the
 * computed rollover time to prevent thundering herd in multi-process
 * scenarios.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class LOG4QT_EXPORT TimeBasedTriggeringPolicy : public TriggeringPolicy
{
    Q_OBJECT

    /*!
     * The date pattern used to determine the rollover frequency.
     * The default is "'.'yyyy-MM-dd" (daily).
     */
    Q_PROPERTY(QString datePattern READ datePattern WRITE setDatePattern)
    Q_PROPERTY(int interval READ interval WRITE setInterval)
    Q_PROPERTY(bool modulate READ modulate WRITE setModulate)
    Q_PROPERTY(int maxRandomDelay READ maxRandomDelay WRITE setMaxRandomDelay)

public:
    enum class Frequency : int
    {
        Minutely = 0,
        Hourly,
        HalfDaily,
        Daily,
        Weekly,
        Monthly,
    };
    Q_ENUM(Frequency)

    explicit TimeBasedTriggeringPolicy(QObject *parent = nullptr);

    [[nodiscard]] QString datePattern() const { return mDatePattern; }
    void setDatePattern(const QString &datePattern) { mDatePattern = datePattern; }

    [[nodiscard]] int interval() const { return mInterval; }
    // Clamp to >= 1: mInterval is a divisor in computeRollOverTime(), which runs
    // on the logging thread, so a 0/negative value must never be storable.
    void setInterval(int interval) { mInterval = interval > 0 ? interval : 1; }

    [[nodiscard]] bool modulate() const { return mModulate; }
    void setModulate(bool modulate) { mModulate = modulate; }

    [[nodiscard]] int maxRandomDelay() const { return mMaxRandomDelay; }
    void setMaxRandomDelay(int maxRandomDelay) { mMaxRandomDelay = maxRandomDelay < 0 ? 0 : maxRandomDelay; }

    [[nodiscard]] Frequency frequency() const { return mFrequency; }

    void activateOptions() override;

    bool isTriggeringEvent(QIODevice *activeDevice,
                           const LoggingEvent &event) override;

private:
    Q_DISABLE_COPY_MOVE(TimeBasedTriggeringPolicy)

    void computeFrequency();
    void computeRollOverTime();

    QString mDatePattern;
    QString mActiveDatePattern;
    Frequency mFrequency;
    int mInterval = 1;
    bool mModulate = false;
    int mMaxRandomDelay = 0;
    QDateTime mRollOverTime;
};

} // namespace Log4Qt

#endif // LOG4QT_TIMEBASEDTRIGGERINGPOLICY_H
