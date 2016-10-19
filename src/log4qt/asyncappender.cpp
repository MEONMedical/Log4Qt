/******************************************************************************
 *
 * package:     Log4Qt
 * file:        asyncappender.cpp
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

#include "asyncappender.h"
#include "loggingevent.h"
#include "helpers/dispatcher.h"

#include <QCoreApplication>
#include <QReadLocker>
#include <QThread>

namespace Log4Qt
{

AsyncAppender::AsyncAppender(QObject *parent) : AppenderSkeleton(parent)
    , mpThread(Q_NULLPTR)
    , mpDispatcher(Q_NULLPTR)
{
}

AsyncAppender::~AsyncAppender()
{
    close();
}

bool AsyncAppender::requiresLayout() const
{
    return false;
}

void AsyncAppender::activateOptions()
{
    if (mpThread)
        return;

    mpThread = new QThread();
    mpDispatcher = new Dispatcher();
    mpDispatcher->setAsyncAppender(this);

    mpDispatcher->moveToThread(mpThread);
    mpThread->start();
}

void AsyncAppender::close()
{
    if (mpThread)
    {
        mpDispatcher->setAsyncAppender(Q_NULLPTR);
        mpThread->quit();
        mpThread->wait();
        delete mpThread;
        mpThread = 0;
        delete mpDispatcher;
        mpDispatcher = 0;
    }
}

void AsyncAppender::callAppenders(const LoggingEvent &rEvent) const
{
    QReadLocker locker(&mAppenderGuard);

    for (auto pAppender : mAppenders)
        pAppender->doAppend(rEvent);
}

void AsyncAppender::append(const LoggingEvent &rEvent)
{
    // Post to the event loop of the dispatcher
    LoggingEvent *event = new LoggingEvent(rEvent);
    qApp->postEvent(mpDispatcher, event);
}

bool AsyncAppender::checkEntryConditions() const
{
    if (mpThread && !mpThread->isRunning())
    {
        LogError
        e =
            LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without a running dispatcher thread"),
                                APPENDER_ASNC_DISPATCHER_NOT_RUNNING);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

