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

#include "asyncappender.h"
#include "helpers/asyncworker.h"
#include "helpers/boundedblockingqueue.h"
#include "logger.h"
#include "loggerrepository.h"
#include "loggingevent.h"
#include "logmanager.h"

#include <QReadLocker>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

AsyncAppender::AsyncAppender(QObject *parent)
    : AppenderSkeleton(parent)
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

// --- Property accessors ------------------------------------------------------

void AsyncAppender::setBufferSize(int size)
{
    if (size <= 0)
    {
        logger()->warn(u"AsyncAppender '%1': invalid bufferSize %2, clamping to 1"_s
                       .arg(name()).arg(size));
        size = 1;
    }
    mBufferSize = size;
}

void AsyncAppender::setQueueFullPolicy(QueueFullPolicy policy)
{
    mQueueFullPolicy = policy;
}

QString AsyncAppender::queueFullPolicyString() const
{
    switch (mQueueFullPolicy)
    {
    case QueueFullPolicy::Discard:      return QStringLiteral("Discard");
    case QueueFullPolicy::Synchronous:  return QStringLiteral("Synchronous");
    default:                            return QStringLiteral("Block");
    }
}

void AsyncAppender::setQueueFullPolicyString(const QString &policy)
{
    if (policy.compare(u"Discard", Qt::CaseInsensitive) == 0)
        mQueueFullPolicy = QueueFullPolicy::Discard;
    else if (policy.compare(u"Synchronous", Qt::CaseInsensitive) == 0)
        mQueueFullPolicy = QueueFullPolicy::Synchronous;
    else
        mQueueFullPolicy = QueueFullPolicy::Block;
}

void AsyncAppender::setErrorAppender(const AppenderSharedPtr &appender)
{
    QMutexLocker locker(&mObjectGuard);
    mErrorAppender = appender;
}

void AsyncAppender::resolveErrorAppender()
{
    QString ref;
    {
        QMutexLocker locker(&mObjectGuard);
        if (mErrorAppender || mErrorRef.isEmpty())
            return;
        ref = mErrorRef;
    }

    // Search with mObjectGuard released: loggers() takes the repository's read
    // lock and appenders() the loggers' read locks, while the logging path
    // takes those before mObjectGuard (Logger::callAppenders() ->
    // Appender::doAppend()). Searching under mObjectGuard would invert that
    // order — and a thread that logs while holding the repository write lock
    // (see Hierarchy::logger()) would deadlock on the recursive
    // QReadWriteLock, which does not allow a read acquire from a writer.
    AppenderSharedPtr resolved;
    const auto loggers = LogManager::loggerRepository()->loggers();
    for (const auto *logger : loggers)
    {
        const auto appenders = logger->appenders();
        for (const auto &appender : appenders)
        {
            if (appender && appender->name() == ref)
            {
                resolved = appender;
                break;
            }
        }
        if (resolved)
            break;
    }

    if (!resolved)
    {
        // Only debug: a configurator resolves the reference once every
        // appender of the configuration exists, so a reference to an appender
        // declared later in the same file is not yet an error here.
        logger()->debug(u"AsyncAppender '%1': errorRef appender '%2' was not found on any logger"_s
                        .arg(name(), ref));
        return;
    }

    QMutexLocker locker(&mObjectGuard);
    if (mErrorRef == ref && !mErrorAppender)
        mErrorAppender = resolved;
}

// --- Lifecycle ---------------------------------------------------------------

void AsyncAppender::activateOptions()
{
    {
        QMutexLocker locker(&mObjectGuard);

        if (mWorker)
            return;

        mQueue = std::make_unique<BoundedBlockingQueue<LoggingEvent>>(mBufferSize);

        mWorker = std::make_unique<AsyncWorker>(this, mQueue.get());
        mWorker->setObjectName(QStringLiteral("Log4Qt-Async-%1").arg(name()));
        mWorker->start();
    }

    // Outside the lock: the lookup takes repository and logger locks.
    resolveErrorAppender();

    AppenderSkeleton::activateOptions();
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

    if (mQueue)
        mQueue->shutdown();

    if (mWorker)
    {
        const unsigned long timeout = (mShutdownTimeout > 0)
            ? static_cast<unsigned long>(mShutdownTimeout)
            : ULONG_MAX;

        if (!mWorker->wait(timeout))
        {
            LogError e = LOG4QT_QCLASS_ERROR(
                "Shutdown timeout expired for async appender '%1' with events still in queue",
                AppenderAsyncShutdownTimeout);
            e << name();
            logger()->warn(e);

            mWorker->terminate();
            mWorker->wait();
        }

        mWorker.reset();
    }

    mQueue.reset();
}

// --- Appending ---------------------------------------------------------------

void AsyncAppender::callAppenders(const LoggingEvent &event) const
{
    QReadLocker locker(&mAppenderGuard);

    for (const auto &appender : mAppenders)
        forwardEvent(appender, event);
}

void AsyncAppender::append(const LoggingEvent &event)
{
    if (!mQueue)
        return;

    switch (mQueueFullPolicy)
    {
    case QueueFullPolicy::Block:
        if (mBlocking)
        {
            mQueue->enqueue(event);
        }
        else
        {
            if (!mQueue->tryEnqueue(event))
                handleQueueFull(event);
        }
        break;

    case QueueFullPolicy::Discard:
        if (!mQueue->tryEnqueue(event))
        {
            if (event.level() <= mDiscardThreshold)
            {
                mDiscardedCount.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                // Events above the discard threshold still block
                mQueue->enqueue(event);
            }
        }
        break;

    case QueueFullPolicy::Synchronous:
        if (!mQueue->tryEnqueue(event))
            callAppenders(event);
        break;
    }
}

void AsyncAppender::handleQueueFull(const LoggingEvent &event)
{
    // Deliberately no errorRef lookup here: this runs under mObjectGuard on
    // the producing thread (doAppend() phase 5), so a repository search would
    // serialise every producer for its duration — exactly when the appender is
    // already saturated — and would invert the lock order of the logging path
    // (see resolveErrorAppender()). The reference is resolved once, by the
    // configurator after all appenders exist or in activateOptions().
    if (mErrorAppender)
    {
        forwardEvent(mErrorAppender, event);
        return;
    }

    if (!mErrorRef.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR(
            "Async appender '%1' queue is full, event dropped: errorRef '%2' was never resolved to an appender",
            AppenderAsyncQueueFull);
        e << name() << mErrorRef;
        logger()->warn(e);
        return;
    }

    LogError e = LOG4QT_QCLASS_ERROR(
        "Async appender '%1' queue is full, event dropped",
        AppenderAsyncQueueFull);
    e << name();
    logger()->warn(e);
}

bool AsyncAppender::checkEntryConditions() const
{
    QMutexLocker locker(&mObjectGuard);
    if (mWorker && !mWorker->isRunning())
    {
        LogError e = LOG4QT_QCLASS_ERROR(
            "Use of appender '%1' without a running dispatcher thread",
            AppenderAsncDispatcherNotRunning);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

#include "moc_asyncappender.cpp"
