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

#ifndef LOG4QT_ROLINGFILEAPPENDER_H
#define LOG4QT_ROLINGFILEAPPENDER_H

#include "fileappender.h"
#include "spi/triggeringpolicy.h"
#include "spi/rolloverstrategy.h"

namespace Log4Qt
{

/*!
 * \brief The class RollingFileAppender extends FileAppender to roll over
 *        the log file based on configurable triggering policies and
 *        rollover strategies.
 *
 * A TriggeringPolicy determines WHEN a rollover should occur
 * (e.g. when the file exceeds a certain size, at a time interval,
 * or on application startup).
 *
 * A RolloverStrategy determines HOW the rollover is performed
 * (e.g. numbered backup file rotation).
 *
 * Multiple triggering policies can be combined (OR logic) using
 * addTriggeringPolicy(). If no strategy is set, a
 * DefaultRolloverStrategy is used.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT RollingFileAppender : public FileAppender
{
    Q_OBJECT

    /*!
     * The property controls whether the layout footer is suppressed when a
     * startup rollover occurs.
     *
     * When true and a triggering policy fires on startup (e.g.
     * OnStartupTriggeringPolicy), the footer is not written to the previous
     * log file before rolling over. This is useful when the footer is a
     * structural delimiter (such as \c "]" for JsonLayout) that should only
     * appear in normally-closed files.
     *
     * The default is false (footer is always written).
     *
     * \sa skipFooterOnStartup(), setSkipFooterOnStartup()
     */
    Q_PROPERTY(bool skipFooterOnStartup READ skipFooterOnStartup WRITE setSkipFooterOnStartup)

public:
    RollingFileAppender(QObject *parent = nullptr);
    RollingFileAppender(const LayoutSharedPtr &layout,
                        const QString &fileName,
                        QObject *parent = nullptr);
    RollingFileAppender(const LayoutSharedPtr &layout,
                        const QString &fileName,
                        bool append,
                        QObject *parent = nullptr);

private:
    Q_DISABLE_COPY_MOVE(RollingFileAppender)

public:
    void setTriggeringPolicy(const TriggeringPolicySharedPtr &policy);
    void addTriggeringPolicy(const TriggeringPolicySharedPtr &policy);
    TriggeringPolicySharedPtr triggeringPolicy() const
    {
        QMutexLocker locker(&mObjectGuard);
        return mTriggeringPolicy;
    }

    void setRolloverStrategy(const RolloverStrategySharedPtr &strategy);
    RolloverStrategySharedPtr rolloverStrategy() const
    {
        QMutexLocker locker(&mObjectGuard);
        return mRolloverStrategy;
    }

    [[nodiscard]] bool skipFooterOnStartup() const { return mSkipFooterOnStartup; }
    void setSkipFooterOnStartup(bool skip) { mSkipFooterOnStartup = skip; }

    void activateOptions() override;

protected:
    void append(const LoggingEvent &event) override;
    virtual void rollOver();

private:
    TriggeringPolicySharedPtr mTriggeringPolicy;
    RolloverStrategySharedPtr mRolloverStrategy;
    QString mBaseFileName;
    // Active filename last set internally (initial name or rollover result);
    // used to tell a strategy-transformed file() from a user-configured one
    // when (re-)activating.
    QString mActiveFileName;
    bool mSkipFooterOnStartup = false;
};

} // namespace Log4Qt

#endif // LOG4QT_ROLINGFILEAPPENDER_H
