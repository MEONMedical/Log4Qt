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

#ifndef LOG4QT_ASYNCAPPENDER_H
#define LOG4QT_ASYNCAPPENDER_H

#include "log4qtshared.h"
#include "appenderskeleton.h"
#include "helpers/appenderattachable.h"

#include <atomic>
#include <memory>

namespace Log4Qt
{

class AsyncWorker;
class LoggingEvent;
template<typename T> class BoundedBlockingQueue;

/*!
 * \brief The class AsyncAppender lets users log events asynchronously.
 *
 * The AsyncAppender collects events sent to it into a bounded blocking
 * queue and dispatches them to all attached appenders on a dedicated
 * worker thread. You can attach multiple appenders to an AsyncAppender.
 *
 * When the queue is full, the behaviour is controlled by the
 * \l queueFullPolicy property:
 *
 * \list
 * \li \b Block — the calling thread blocks until space is available
 *     (default). If \l blocking is false, the event is routed to the
 *     error appender instead of blocking.
 * \li \b Discard — events at or below the \l discardThreshold level
 *     are silently dropped; higher-priority events still block.
 * \li \b Synchronous — the event is logged directly on the calling
 *     thread, bypassing the queue.
 * \endlist
 *
 * \note All the functions declared in this class are thread-safe.
 * &nbsp;
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 *
 * \note The class is \c final by design. The worker thread is joined in
 *       \c ~AsyncAppender, so a subclass destructor would run while the
 *       worker is still calling back into this object (callAppenders(),
 *       batchComplete()) — a use-after-free hazard on the subclass part.
 *       Extend behaviour by attaching appenders, not by subclassing.
 */
class LOG4QT_EXPORT AsyncAppender final : public AppenderSkeleton, public AppenderAttachable
{
    Q_OBJECT

    /*!
     * The maximum number of events the queue can hold. Applied when
     * activateOptions() is called. The default is 1024.
     */
    Q_PROPERTY(int bufferSize READ bufferSize WRITE setBufferSize)

    /*!
     * If true (default), the calling thread blocks when the queue is
     * full (Block policy). If false, the event is routed to the error
     * appender immediately.
     */
    Q_PROPERTY(bool blocking READ blocking WRITE setBlocking)

    /*!
     * Milliseconds to wait for the queue to drain during shutdown.
     * 0 (default) means wait indefinitely.
     */
    Q_PROPERTY(int shutdownTimeout READ shutdownTimeout WRITE setShutdownTimeout)

    /*!
     * Events at or below this level are discarded when the queue is
     * full and the Discard policy is active. The default is INFO.
     */
    Q_PROPERTY(Log4Qt::Level discardThreshold READ discardThreshold WRITE setDiscardThreshold)

    /*!
     * The queue-full policy as a string: "Block", "Discard", or
     * "Synchronous". The default is "Block".
     */
    Q_PROPERTY(QString queueFullPolicy READ queueFullPolicyString WRITE setQueueFullPolicyString)

    /*!
     * Name of a fallback appender that receives events when the queue
     * is full and the event cannot be enqueued. Resolved at
     * activateOptions() time.
     */
    Q_PROPERTY(QString errorRef READ errorRef WRITE setErrorRef)

public:
    /*!
     * Controls what happens when the queue is full.
     */
    enum class QueueFullPolicy
    {
        Block,        //!< Caller blocks until space is available (default)
        Discard,      //!< Drop events at/below discardThreshold
        Synchronous   //!< Log directly on the calling thread
    };
    Q_ENUM(QueueFullPolicy)

    AsyncAppender(QObject *parent = nullptr);
    ~AsyncAppender() override;

    bool requiresLayout() const override;

    // --- Property accessors ---------------------------------------------------

    int bufferSize() const { return mBufferSize; }
    void setBufferSize(int size);

    bool blocking() const { return mBlocking; }
    void setBlocking(bool blocking) { mBlocking = blocking; }

    int shutdownTimeout() const { return mShutdownTimeout; }
    void setShutdownTimeout(int timeoutMs) { mShutdownTimeout = timeoutMs; }

    Level discardThreshold() const { return mDiscardThreshold; }
    void setDiscardThreshold(Level level) { mDiscardThreshold = level; }

    QueueFullPolicy queueFullPolicy() const { return mQueueFullPolicy; }
    void setQueueFullPolicy(QueueFullPolicy policy);

    QString queueFullPolicyString() const;
    void setQueueFullPolicyString(const QString &policy);

    QString errorRef() const { return mErrorRef; }
    void setErrorRef(const QString &name) { mErrorRef = name; }

    void setErrorAppender(const AppenderSharedPtr &appender);

    qint64 discardedCount() const { return mDiscardedCount.load(std::memory_order_relaxed); }

    // --- Lifecycle ------------------------------------------------------------

    void activateOptions() override;
    void close() override;
    void callAppenders(const LoggingEvent &event) const;

    bool checkEntryConditions() const override;

Q_SIGNALS:
    /*!
     * Emitted on the worker thread when the queue becomes empty after
     * dispatching. Downstream code can connect to this for batch-flush
     * optimisation.
     */
    void batchComplete();

protected:
    void append(const LoggingEvent &event) override;

private:
    Q_DISABLE_COPY_MOVE(AsyncAppender)

    void closeInternal();
    void handleQueueFull(const LoggingEvent &event);

    int mBufferSize = 1024;
    bool mBlocking = true;
    int mShutdownTimeout = 0;
    Level mDiscardThreshold = Level::INFO_INT;
    QueueFullPolicy mQueueFullPolicy = QueueFullPolicy::Block;
    QString mErrorRef;
    AppenderSharedPtr mErrorAppender;

    std::unique_ptr<BoundedBlockingQueue<LoggingEvent>> mQueue;
    std::unique_ptr<AsyncWorker> mWorker;

    std::atomic<qint64> mDiscardedCount{0};
};

} // namespace Log4Qt

#endif // LOG4QT_ASYNCAPPENDER_H
