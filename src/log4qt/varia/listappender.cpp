/******************************************************************************
 *
 * package:     Log4Qt
 * file:        listappender.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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

#include "varia/listappender.h"

namespace Log4Qt
{

ListAppender::ListAppender(QObject *parent) :
    AppenderSkeleton(parent),
    mConfiguratorList(false),
    mMaxCount(0)
{
}

QList<LoggingEvent> ListAppender::list() const
{
    QMutexLocker locker(&mObjectGuard);
    return mList;
}

void ListAppender::setMaxCount(int n)
{
    QMutexLocker locker(&mObjectGuard);

    if (n < 0)
    {
        logger()->warn(QStringLiteral("Attempt to set maximum count for appender '%1' to %2. Using zero instead"), name(), n);
        n = 0;
    }
    mMaxCount = n;
    ensureMaxCount();
}

QList<LoggingEvent> ListAppender::clearList()
{
    QMutexLocker locker(&mObjectGuard);

    QList<LoggingEvent> result = mList;
    mList.clear();
    return result;
}

void ListAppender::append(const LoggingEvent &event)
{
    if ((mMaxCount <= 0) || (mList.size() < mMaxCount))
        mList << event;
}

void ListAppender::ensureMaxCount()
{
    if (mMaxCount <= 0)
        return;

    while (mList.size() > mMaxCount)
        mList.removeFirst();
}

} // namespace Log4Qt

#include "moc_listappender.cpp"
