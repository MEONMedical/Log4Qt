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
    , mThread(nullptr)
    , mDispatcher(nullptr)
{
}

AsyncAppender::~AsyncAppender()
{
    closeInternal();
}

bool AsyncAppender::requiresLayout() const
{
    return false;
}

void AsyncAppender::activateOptions()
{
    if (mThread != nullptr)
        return;

    mThread = new QThread();
    mDispatcher = new Dispatcher();
    mDispatcher->setAsyncAppender(this);

    mDispatcher->moveToThread(mThread);
    mThread->start();
}

void AsyncAppender::close()
{
    closeInternal();
    AppenderSkeleton::close();
}

void AsyncAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    if (mThread != nullptr)
    {
        mDispatcher->setAsyncAppender(nullptr);
        mThread->quit();
        mThread->wait();
        delete mThread;
        mThread = nullptr;
        delete mDispatcher;
        mDispatcher = nullptr;
    }
}

void AsyncAppender::callAppenders(const LoggingEvent &event) const
{
    QReadLocker locker(&mAppenderGuard);

    for (auto &&pAppender : qAsConst(mAppenders))
        pAppender->doAppend(event);
}

void AsyncAppender::append(const LoggingEvent &event)
{
    // Post to the event loop of the dispatcher
    qApp->postEvent(mDispatcher, new LoggingEvent(event));
}

bool AsyncAppender::checkEntryConditions() const
{
    if ((mThread != nullptr) && !mThread->isRunning())
    {
        LogError e =
            LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without a running dispatcher thread"),
                                APPENDER_ASNC_DISPATCHER_NOT_RUNNING);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

#include "moc_asyncappender.cpp"
