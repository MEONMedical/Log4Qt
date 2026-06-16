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

#ifndef LOG4QT_COMPOSITETRIGGERINGPOLICY_H
#define LOG4QT_COMPOSITETRIGGERINGPOLICY_H

#include "triggeringpolicy.h"

#include <QList>

namespace Log4Qt
{

/*!
 * \brief The class CompositeTriggeringPolicy combines multiple triggering
 *        policies using OR logic. A rollover is triggered if ANY contained
 *        policy returns \c true.
 *
 * \note Thread-safety contract: the contained policy list is \e not
 *       independently synchronised. All access is serialised by the owning
 *       RollingFileAppender's mutex — addPolicy() runs under that lock during
 *       configuration, and activateOptions()/isTriggeringEvent()/
 *       isStartupTrigger() run under the same lock during logging. Policies
 *       must therefore be added before/at activation, never concurrently with
 *       rollover evaluation. The class is \c final so this contract cannot be
 *       broken by a subclass adding unsynchronised mutation.
 */
class LOG4QT_EXPORT CompositeTriggeringPolicy final : public TriggeringPolicy
{
    Q_OBJECT

public:
    explicit CompositeTriggeringPolicy(QObject *parent = nullptr);

    void addPolicy(const TriggeringPolicySharedPtr &policy);
    QList<TriggeringPolicySharedPtr> policies() const;

    void activateOptions() override;

    bool isTriggeringEvent(QIODevice *activeDevice,
                           const LoggingEvent &event) override;

    bool isStartupTrigger(const QString &fileName, qint64 fileSize) override;

private:
    Q_DISABLE_COPY_MOVE(CompositeTriggeringPolicy)
    QList<TriggeringPolicySharedPtr> mPolicies;
};

} // namespace Log4Qt

#endif // LOG4QT_COMPOSITETRIGGERINGPOLICY_H
