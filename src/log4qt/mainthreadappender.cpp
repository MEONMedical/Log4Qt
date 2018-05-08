/******************************************************************************
 *
 * package:     Log4Qt
 * file:        mainthreadappender.cpp
 * created:     February 2011
 * author:      Andreas Bacher
 *
 *
 * Copyright 2011 Andreas Bacher
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

#include "mainthreadappender.h"
#include "loggingevent.h"
#include "helpers/dispatcher.h"

#include <QCoreApplication>
#include <QReadLocker>
#include <QThread>

namespace Log4Qt
{

MainThreadAppender::MainThreadAppender(QObject *parent) : AppenderSkeleton(parent)
{}

bool MainThreadAppender::requiresLayout() const
{
    return false;
}

void MainThreadAppender::activateOptions()
{
}

void MainThreadAppender::append(const LoggingEvent &event)
{
    QReadLocker locker(&mAppenderGuard);

    for (auto &&pAppender : qAsConst(mAppenders))
    {
        if (QThread::currentThread() != qApp->thread())
            qApp->postEvent(pAppender.data(), new LoggingEvent(event));
        else
            pAppender->doAppend(event);
    }
}

bool MainThreadAppender::checkEntryConditions() const
{
    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

#include "moc_mainthreadappender.cpp"

