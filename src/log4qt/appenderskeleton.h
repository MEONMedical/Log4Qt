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

#ifndef LOG4QT_APPENDERSKELETON_H
#define LOG4QT_APPENDERSKELETON_H

#include "appender.h"
#include "log4qtshared.h"
#include "abstractlayout.h"
#include "spi/filter.h"
#include "logger.h"

#include <QMutex>
#include <atomic>

namespace Log4Qt
{

class Logger;
class LoggingEvent;

/*!
 * \brief The class AppenderSkeleton implements general Appender functionality.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed. See
 *       \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT AppenderSkeleton : public Appender
{
    Q_OBJECT

    /*!
     * The property holds if the Appender has been activated.
     *
     * \sa isActive()
     */
    Q_PROPERTY(bool isActive READ isActive)

    /*!
     * The property holds if the Appender has been closed.
     *
     * \sa isClosed()
     */
    Q_PROPERTY(bool isClosed READ isClosed)

    /*!
     * The property holds the threshold level used by the Appender.
     *
     * \sa threshold(), setThreshold()
     */
    Q_PROPERTY(Log4Qt::Level threshold READ threshold WRITE setThreshold)

public:
    explicit AppenderSkeleton(QObject *parent = nullptr);

protected:
    explicit AppenderSkeleton(bool isActive,
                              QObject *parent = nullptr);
    explicit AppenderSkeleton(bool isActive,
                              const LayoutSharedPtr &layout,
                              QObject *parent = nullptr);
    ~AppenderSkeleton() override;

public:
    [[nodiscard]] FilterSharedPtr filter() const override;
    [[nodiscard]] LayoutSharedPtr layout() const override;
    [[nodiscard]] bool isActive() const { return mIsActive.load(std::memory_order_relaxed); }
    [[nodiscard]] bool isClosed() const { return mIsClosed.load(std::memory_order_relaxed); }
    [[nodiscard]] QString name() const override;
    [[nodiscard]] Level threshold() const { return mThreshold; }
    void setLayout(const LayoutSharedPtr &layout) override;
    void setName(const QString &name) override;
    void setThreshold(Level level) { mThreshold = level; }

    virtual void activateOptions();
    void addFilter(const FilterSharedPtr &filter) override;
    void clearFilters() override;
    void close() override;

    /*!
     * Performs checks and delegates the actual appending to the subclass.
     *
     * The function executes in five phases:
     * \li Phase 1 — Thread-local recursion guard. Prevents infinite loops when
     *     an appender internally logs a message through a logger that routes
     *     back to any appender on the same thread.
     * \li Phase 2 — Fast atomic pre-checks (\c isActive(), \c isClosed())
     *     without acquiring the lock.
     * \li Phase 3 — Entry conditions (\c checkEntryConditions()), threshold
     *     and filter-chain/layout snapshot, all under \c mObjectGuard. The
     *     lock is released at the end of this phase.
     * \li Phase 4 — Filter chain evaluation and \c preAppend() call, both
     *     \e outside \c mObjectGuard. Multiple threads may execute this phase
     *     concurrently.
     * \li Phase 5 — \c append() call under \c mObjectGuard. Serialises the
     *     actual I/O across threads.
     *
     * \sa append(), preAppend(), checkEntryConditions(),
     *     isAsSevereAsThreshold(), Filter
     */
    void doAppend(const LoggingEvent &event) override;

    FilterSharedPtr firstFilter() const
    {
        QMutexLocker locker(&mObjectGuard);
        return filter();
    }
    bool isAsSevereAsThreshold(Level level) const { return (mThreshold <= level); }

protected:
    virtual void append(const LoggingEvent &event) = 0;
    void customEvent(QEvent *event) override;

    /*!
     * Tests if all entry conditions for using append() in this class are
     * met.
     *
     * If a conditions is not met, an error is logged and the function
     * returns false.
     *
     * The checked conditions are:
     * - That the appender has been activated (AppenderNotActivatedError)
     * - That the appender was not closed (AppenderClosedError)
     * - That the appender has a layout set, if it requires one
     *   (logging_error(AppenderUseMissingLayoutError)
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by doAppend(). The doAppend() function calls the subclass
     * specific checkEntryConditions() function. The function checks the
     * class specific conditions and calls checkEntryConditions() of
     * it's parent class. The last function called is
     * AppenderSkeleton::checkEntryConditions().
     *
     * \sa doAppend()
     */
    virtual bool checkEntryConditions() const;

    /*!
     * Optional hook called \e outside \c mObjectGuard, after all entry checks
     * have passed and the filter chain has accepted the event.
     *
     * \c doAppend() calls this function between releasing the appender lock
     * (after snapshotting the filter chain and layout) and re-acquiring it for
     * the actual \c append() call. This window allows subclasses to perform
     * expensive, purely read-only preparation work — most commonly layout
     * formatting — while other threads are free to run their own \c preAppend()
     * calls in parallel.
     *
     * \par Contract
     * \li \a layout is a \c QSharedPointer snapshot taken under the lock; it
     *     remains valid for the full duration of this call even if the
     *     appender's layout is replaced concurrently.
     * \li The function must be stateless with respect to shared appender data.
     *     Any result must be stored in thread-local storage and consumed by the
     *     subsequent \c append() call.
     * \li The function must not call \c doAppend() (directly or indirectly) —
     *     the per-thread recursion guard in \c doAppend() would silently drop
     *     the nested call.
     *
     * The default implementation is a no-op; all existing appenders are
     * unaffected.
     *
     * \sa doAppend(), append(), RandomAccessFileAppender
     */
    virtual void preAppend(const LoggingEvent &event, const LayoutSharedPtr &layout);

    /*!
     * Forwards \a event to \a appender via its \c doAppend() entry point,
     * bypassing the thread-local recursion guard for this one call.
     *
     * Use this for \e intentional event forwarding (e.g. routing an overflow
     * event to an error appender) where the call is not a recursive side-effect
     * of logging but an explicit redirect. All other \c doAppend() checks
     * (active, closed, threshold, filters) still run normally on the target
     * appender.
     *
     * \note Do \e not use this inside \c append() or \c preAppend() to route
     *       internally generated log messages — use a normal logger call there;
     *       the recursion guard will silently drop true recursive loops.
     */
    static void forwardEvent(const AppenderSharedPtr &appender, const LoggingEvent &event);

protected:
    /*!
     * Lock-free read of the layout snapshot. Callable only while \c mObjectGuard
     * is held by the caller — i.e. from within \c append() (Phase 5 of
     * \c doAppend()). Returns a reference to the live \c mpLayout, so the
     * caller does not pay the QSharedPointer refcount round-trip of \c layout().
     */
    [[nodiscard]] const LayoutSharedPtr &layoutSnapshot() const { return mpLayout; }

    mutable QRecursiveMutex mObjectGuard;

private:
    Q_DISABLE_COPY_MOVE(AppenderSkeleton)
    std::atomic<bool> mIsActive{false};
    std::atomic<bool> mIsClosed{false};
    LayoutSharedPtr mpLayout;
    std::atomic<Level> mThreshold;
    FilterSharedPtr mpHeadFilter;
    FilterSharedPtr mpTailFilter;
    void closeInternal();
};

} // namespace Log4Qt

#endif // LOG4QT_APPENDERSKELETON_H
