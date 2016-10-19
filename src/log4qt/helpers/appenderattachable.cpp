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

void AppenderAttachable::addAppender(AppenderSharedPtr pAppender)
{
    if (pAppender.isNull())
        return;
    QWriteLocker locker(&mAppenderGuard);
    if (mAppenders.contains(pAppender))
        return;
    mAppenders.append(pAppender);
}

AppenderSharedPtr AppenderAttachable::appender(const QString &rName) const
{
    QReadLocker locker(&mAppenderGuard);

    for (auto pAppender : mAppenders)
        if (pAppender->name() == rName)
            return pAppender;
    return AppenderSharedPtr();
}

bool AppenderAttachable::isAttached(AppenderSharedPtr pAppender) const
{
    QReadLocker locker(&mAppenderGuard);
    return mAppenders.contains(pAppender);
}

void AppenderAttachable::removeAllAppenders()
{
    mAppenders.clear();
}

void AppenderAttachable::removeAppender(AppenderSharedPtr pAppender)
{
    if (pAppender.isNull())
        return;
    QWriteLocker locker(&mAppenderGuard);
    mAppenders.removeAll(pAppender);

}

void AppenderAttachable::removeAppender(const QString &rName)
{
    QWriteLocker locker(&mAppenderGuard);
    AppenderSharedPtr pAppender = appender(rName);
    if (pAppender)
        removeAppender(pAppender);
}

} // namespace Log4Qt
