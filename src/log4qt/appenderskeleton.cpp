/******************************************************************************
 *
 * package:
 * file:        appenderskeleton.cpp
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

#include "appenderskeleton.h"

#include "layout.h"
#include "loggingevent.h"
#include "logmanager.h"
#include "spi/filter.h"
#include "logger.h"

namespace Log4Qt
{

/*!
 * \brief The class RecursionGuardLocker controls a boolean flag.
 *
 * It is a helper class to control a boolean flag. The class sets the flag
 * on creation and resets it on destruction.
 */
class RecursionGuardLocker
{
public:
    RecursionGuardLocker(bool *pGuard);
    ~RecursionGuardLocker();
private:
    Q_DISABLE_COPY(RecursionGuardLocker)
private:
    bool *mpGuard;
};

inline RecursionGuardLocker::RecursionGuardLocker(bool *pGuard)
{
    Q_ASSERT_X(pGuard != nullptr, "RecursionGuardLocker::RecursionGuardLocker()", "Pointer to guard bool must not be null");

    mpGuard = pGuard;
    *mpGuard = true;
}


inline RecursionGuardLocker::~RecursionGuardLocker()
{
    *mpGuard = false;
}

AppenderSkeleton::AppenderSkeleton(QObject *pParent) :
    Appender(pParent),
    mObjectGuard(QMutex::Recursive), // Recursive for doAppend()
    mAppendRecursionGuard(false),
    mIsActive(true),
    mIsClosed(false),
    mThreshold(Level::NULL_INT)
{
}


AppenderSkeleton::AppenderSkeleton(const bool isActive,
                                   QObject *pParent) :
    Appender(pParent),
    mObjectGuard(QMutex::Recursive), // Recursive for doAppend()
    mAppendRecursionGuard(false),
    mIsActive(isActive),
    mIsClosed(false),
    mThreshold(Level::NULL_INT)
{
}

AppenderSkeleton::~AppenderSkeleton()
{
}

void AppenderSkeleton::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (requiresLayout() && !layout())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Activation of appender '%1' that requires layout and has no layout set"),
                                         APPENDER_ACTIVATE_MISSING_LAYOUT_ERROR);
        e << name();
        logger()->error(e);
        return;
    }
    mIsActive = true;
}


void AppenderSkeleton::addFilter(FilterSharedPtr pFilter)
{
    if (!pFilter)
    {
        logger()->warn("Adding null Filter to Appender '%1'", name());
        return;
    }

    QMutexLocker locker(&mObjectGuard);

    if (!mpTailFilter)
    {
        // filter list empty
        mpHeadFilter = pFilter;
        mpTailFilter = pFilter;
    }
    else
    {
        // append filter to the end of the filter list
        mpTailFilter->setNext(pFilter);
        mpTailFilter = pFilter;
    }
}


void AppenderSkeleton::clearFilters()
{
    QMutexLocker locker(&mObjectGuard);

    mpHeadFilter.reset();
}


void AppenderSkeleton::close()
{
    QMutexLocker locker(&mObjectGuard);

    mIsClosed = true;
    mIsActive = false;
}

void AppenderSkeleton::customEvent(QEvent *event)
{
    if (event->type() == LoggingEvent::eventId)
    {
        LoggingEvent *logEvent = static_cast<LoggingEvent *>(event);
        doAppend(*logEvent);
        return ;
    }
    QObject::customEvent(event);
}

void AppenderSkeleton::doAppend(const LoggingEvent &rEvent)
{
    // The mutex serialises concurrent access from multiple threads.
    // - e.g. two threads using the same logger
    // - e.g. two threads using different logger with the same appender
    //
    // A call from the same thread will pass the mutex (QMutex::Recursive)
    // and get to the recursion guard. The recursion guard blocks recursive
    // invocation and prevents a possible endless loop.
    // - e.g. an appender logs an error with a logger that uses it

    QMutexLocker locker(&mObjectGuard);

    if (mAppendRecursionGuard)
        return;

    RecursionGuardLocker recursion_locker(&mAppendRecursionGuard);

    if (!checkEntryConditions())
        return;
    if (!isAsSevereAsThreshold(rEvent.level()))
        return;

    Filter  *p_filter = mpHeadFilter.data();
    while (p_filter)
    {
        Filter::Decision decision = p_filter->decide(rEvent);
        if (decision == Filter::ACCEPT)
            break;
        else if (decision == Filter::DENY)
            return;
        else
            p_filter = p_filter->next().data();
    }

    append(rEvent);
}


bool AppenderSkeleton::checkEntryConditions() const
{
    if (!isActive())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of non activated appender '%1'"),
                                         APPENDER_NOT_ACTIVATED_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }
    if (isClosed())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of closed appender '%1'"),
                                         APPENDER_CLOSED_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }
    if (requiresLayout() && !layout())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' that requires layout and has no layout set"),
                                         APPENDER_USE_MISSING_LAYOUT_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }

    return true;
}

void Log4Qt::AppenderSkeleton::setLayout(LayoutSharedPtr pLayout)
{
    QMutexLocker locker(&mObjectGuard);
    mpLayout = pLayout;
}

LayoutSharedPtr Log4Qt::AppenderSkeleton::layout() const
{
    QMutexLocker locker(&mObjectGuard);
    return mpLayout;
}

} // namespace Log4Qt

#include "moc_appenderskeleton.cpp"
