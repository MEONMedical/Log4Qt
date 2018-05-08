/******************************************************************************
 *
 * package:     Log4Qt
 * file:        appenderattachable.cpp
 * created:     Dezember 2010
 * author:      Andreas Bacher
 *
 *
 * Copyright 2007 Andreas Bacher
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

#include "helpers/appenderattachable.h"
#include "varia/listappender.h"
#include "appender.h"

namespace Log4Qt
{

AppenderAttachable::AppenderAttachable() :
    mAppenderGuard(QReadWriteLock::Recursive)
{
}

QList<AppenderSharedPtr> AppenderAttachable::appenders() const
{
    QReadLocker locker(&mAppenderGuard);
    return mAppenders;
}

AppenderSharedPtr AppenderAttachable::appender(const QString &name) const
{
    QReadLocker locker(&mAppenderGuard);

    for (auto &&pAppender : qAsConst(mAppenders))
        if (pAppender->name() == name)
            return pAppender;
    return AppenderSharedPtr();
}

void AppenderAttachable::addAppender(const AppenderSharedPtr &appender)
{
    if (appender.isNull())
        return;
    QWriteLocker locker(&mAppenderGuard);
    if (mAppenders.contains(appender))
        return;
    mAppenders.append(appender);
}

bool AppenderAttachable::isAttached(const AppenderSharedPtr &appender) const
{
    QReadLocker locker(&mAppenderGuard);
    return mAppenders.contains(appender);
}

void AppenderAttachable::removeAllAppenders()
{
    mAppenders.clear();
}

void AppenderAttachable::removeAppender(const AppenderSharedPtr &appender)
{
    if (appender.isNull())
        return;
    QWriteLocker locker(&mAppenderGuard);
    mAppenders.removeAll(appender);

}

void AppenderAttachable::removeAppender(const QString &name)
{
    QWriteLocker locker(&mAppenderGuard);
    AppenderSharedPtr pAppender = appender(name);
    if (pAppender)
        removeAppender(pAppender);
}

} // namespace Log4Qt

