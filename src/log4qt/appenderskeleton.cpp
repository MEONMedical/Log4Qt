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

#include "appenderskeleton.h"

#include "abstractlayout.h"
#include "loggingevent.h"
#include "spi/filter.h"
#include "logger.h"

#include <QScopeGuard>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

// Per-thread recursion depth. Incremented on entry to doAppend() and
// decremented on exit. Any value > 0 means the current thread is already
// inside an appender's processing chain, so further doAppend() calls on
// the same thread are silently dropped to prevent infinite recursion.
//
// This replaces the former per-appender bool flag (mAppendRecursionGuard)
// and makes it thread-local so that doAppend() can release mObjectGuard
// before the expensive formatting step without creating a data race on the
// guard variable itself.
thread_local int s_appendDepth = 0;

AppenderSkeleton::AppenderSkeleton(QObject *parent)
    : Appender(parent)
    , mThreshold(Level::NULL_INT)
{
    mIsActive.store(true, std::memory_order_relaxed);
    mIsClosed.store(false, std::memory_order_relaxed);
}

AppenderSkeleton::AppenderSkeleton(bool isActive,
                                   QObject *parent)
    : Appender(parent)
    , mThreshold(Level::NULL_INT)
{
    mIsActive.store(isActive, std::memory_order_relaxed);
    mIsClosed.store(false, std::memory_order_relaxed);
}

AppenderSkeleton::AppenderSkeleton(bool isActive,
                                   const LayoutSharedPtr &layout,
                                   QObject *parent)
    : Appender(parent)
    , mpLayout(layout)
    , mThreshold(Level::NULL_INT)
{
    mIsActive.store(isActive, std::memory_order_relaxed);
    mIsClosed.store(false, std::memory_order_relaxed);
}

AppenderSkeleton::~AppenderSkeleton()
{
    closeInternal();
}

void AppenderSkeleton::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (requiresLayout() && !layout())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Activation of appender '%1' that requires layout and has no layout set"),
                                         AppenderActivateMissingLayoutError);
        e << name();
        logger()->error(e);
        return;
    }
    mIsActive.store(true, std::memory_order_relaxed);
}

void AppenderSkeleton::addFilter(const FilterSharedPtr &filter)
{
    if (!filter)
    {
        logger()->warn(u"Adding null Filter to Appender '%1'"_s, name());
        return;
    }

    QMutexLocker locker(&mObjectGuard);

    if (!mpTailFilter)
    {
        // filter list empty
        mpHeadFilter = filter;
        mpTailFilter = filter;
    }
    else
    {
        // append filter to the end of the filter list
        mpTailFilter->setNext(filter);
        mpTailFilter = filter;
    }
}

void AppenderSkeleton::clearFilters()
{
    QMutexLocker locker(&mObjectGuard);

    mpHeadFilter.reset();
}

void AppenderSkeleton::close()
{
    closeInternal();
}

void AppenderSkeleton::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    mIsClosed.store(true, std::memory_order_relaxed);
    mIsActive.store(false, std::memory_order_relaxed);
}

void AppenderSkeleton::customEvent(QEvent *event)
{
    if (event->type() == LoggingEvent::eventId)
    {
        const auto logEvent = static_cast<LoggingEvent *>(event);
        doAppend(*logEvent);
        return ;
    }
    QObject::customEvent(event);
}

void AppenderSkeleton::doAppend(const LoggingEvent &event)
{
    // Phase 1 — recursion guard (thread-local, no lock needed).
    // Prevents infinite loops when an appender internally logs an error
    // through a logger that routes back to the same (or any) appender.
    if (s_appendDepth > 0)
        return;

    ++s_appendDepth;
    const auto depthGuard = qScopeGuard([]{ --s_appendDepth; });

    // Phase 2 — fast pre-checks via atomics (no lock needed).
    if (!isActive() || isClosed())
        return;

    // Phase 3 — entry conditions + config snapshot (under lock, then release).
    // mpHeadFilter and mpLayout are snapshots: they keep their objects alive
    // even if the appender is reconfigured or closed after we drop the lock.
    FilterSharedPtr headFilter;
    LayoutSharedPtr layoutSnap;
    {
        QMutexLocker locker(&mObjectGuard);

        if (!checkEntryConditions())
            return;
        if (!isAsSevereAsThreshold(event.level()))
            return;

        headFilter  = mpHeadFilter;
        layoutSnap  = mpLayout;
    } // mObjectGuard released — expensive work happens outside the lock

    // Phase 4 — filter chain (outside lock, filter::decide() is const).
    const auto *filter = headFilter.data();
    while (filter)
    {
        const Filter::Decision decision = filter->decide(event);
        if (decision == Filter::Accept)
            break;
        else if (decision == Filter::Deny)
            return;
        else
            filter = filter->next().data();
    }

    // Phase 4b — pre-format hook (outside lock).
    // Subclasses such as RandomAccessFileAppender override this to encode the
    // log message into a thread-local buffer while the lock is free, so that
    // multiple threads can format concurrently.
    preAppend(event, layoutSnap);

    // Phase 5 — actual I/O (under lock).
    // Re-check the full entry conditions: close(), setWriter(nullptr) or a
    // reconfiguration may have torn down the appender's resources while the
    // lock was released during Phases 4–4b. isActive() alone does not cover
    // subclass resources (writer, file, dispatcher thread).
    QMutexLocker locker(&mObjectGuard);
    if (checkEntryConditions())
        append(event);
}

void AppenderSkeleton::preAppend(const LoggingEvent & /*event*/, const LayoutSharedPtr & /*layout*/)
{
    // Default implementation: no-op.
    // Subclasses that want to pre-format outside the lock override this.
}

void AppenderSkeleton::forwardEvent(const AppenderSharedPtr &appender, const LoggingEvent &event)
{
    if (!appender)
        return;
    // Temporarily reset the recursion depth so that doAppend() on the target
    // appender is not silently dropped. This is safe because forwarding is an
    // intentional redirect, not a recursive side-effect of logging.
    const int saved = s_appendDepth;
    s_appendDepth = 0;
    appender->doAppend(event);
    s_appendDepth = saved;
}

bool AppenderSkeleton::checkEntryConditions() const
{
    if (!isActive())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of non activated appender '%1'"),
                                         AppenderNotActivatedError);
        e << name();
        logger()->error(e);
        return false;
    }
    if (isClosed())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of closed appender '%1'"),
                                         AppenderClosedError);
        e << name();
        logger()->error(e);
        return false;
    }
    if (requiresLayout() && !layout())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' that requires layout and has no layout set"),
                                         AppenderUseMissingLayoutError);
        e << name();
        logger()->error(e);
        return false;
    }

    return true;
}

void Log4Qt::AppenderSkeleton::setLayout(const LayoutSharedPtr &layout)
{
    QMutexLocker locker(&mObjectGuard);
    mpLayout = layout;
}

LayoutSharedPtr Log4Qt::AppenderSkeleton::layout() const
{
    QMutexLocker locker(&mObjectGuard);
    return mpLayout;
}

FilterSharedPtr AppenderSkeleton::filter() const
{
    QMutexLocker locker(&mObjectGuard);
    return mpHeadFilter;
}

QString AppenderSkeleton::name() const
{
    QMutexLocker locker(&mObjectGuard);
    return objectName();
}

void AppenderSkeleton::setName(const QString &name)
{
    QMutexLocker locker(&mObjectGuard);
    setObjectName(name);
}

} // namespace Log4Qt

#include "moc_appenderskeleton.cpp"
