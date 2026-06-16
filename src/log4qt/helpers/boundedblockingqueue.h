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

#ifndef LOG4QT_HELPERS_BOUNDEDBLOCKINGQUEUE_H
#define LOG4QT_HELPERS_BOUNDEDBLOCKINGQUEUE_H

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <atomic>
#include <vector>

namespace Log4Qt
{

/*!
 * \brief A bounded blocking queue using a circular buffer.
 *
 * Thread-safe queue with configurable capacity. Producers can block or
 * fail-fast when the queue is full. The consumer blocks until an item
 * is available or the queue is shut down.
 *
 * \tparam T Element type. Must be default-constructible and move-assignable.
 */
template<typename T>
class BoundedBlockingQueue
{
public:
    explicit BoundedBlockingQueue(int capacity)
        : mCapacity(capacity > 0 ? capacity : 1)
        , mBuffer(static_cast<std::size_t>(mCapacity))
    {
    }

    /*!
     * Enqueues an item, blocking until space is available or shutdown.
     * \return true if enqueued, false if the queue was shut down.
     */
    bool enqueue(const T &item)
    {
        QMutexLocker locker(&mMutex);
        while (mSize == mCapacity && !mShutdown.load(std::memory_order_relaxed))
            mNotFull.wait(&mMutex);

        if (mShutdown.load(std::memory_order_relaxed))
            return false;

        mBuffer[mTail] = item;
        mTail = (mTail + 1) % mCapacity;
        ++mSize;

        mNotEmpty.wakeOne();
        return true;
    }

    /*!
     * Tries to enqueue an item without blocking.
     * \return true if enqueued, false if the queue is full or shut down.
     */
    bool tryEnqueue(const T &item)
    {
        QMutexLocker locker(&mMutex);
        if (mSize == mCapacity || mShutdown.load(std::memory_order_relaxed))
            return false;

        mBuffer[mTail] = item;
        mTail = (mTail + 1) % mCapacity;
        ++mSize;

        mNotEmpty.wakeOne();
        return true;
    }

    /*!
     * Dequeues an item, blocking until one is available or shutdown.
     * \return true if an item was dequeued, false on shutdown with empty queue.
     */
    bool dequeue(T &item)
    {
        QMutexLocker locker(&mMutex);
        while (mSize == 0 && !mShutdown.load(std::memory_order_relaxed))
            mNotEmpty.wait(&mMutex);

        if (mSize == 0)
            return false;

        item = std::move(mBuffer[mHead]);
        mBuffer[mHead] = T{};
        mHead = (mHead + 1) % mCapacity;
        --mSize;

        mNotFull.wakeOne();
        return true;
    }

    /*!
     * Drains up to \a maxItems into \a out without blocking.
     * \return The number of items drained.
     */
    int drain(std::vector<T> &out, int maxItems)
    {
        QMutexLocker locker(&mMutex);
        const int count = (std::min)(mSize, maxItems);
        out.reserve(out.size() + static_cast<std::size_t>(count));

        for (int i = 0; i < count; ++i)
        {
            out.push_back(std::move(mBuffer[mHead]));
            mBuffer[mHead] = T{};
            mHead = (mHead + 1) % mCapacity;
        }
        mSize -= count;

        if (count > 0)
            mNotFull.wakeAll();

        return count;
    }

    /*!
     * Signals shutdown. All blocking enqueue/dequeue calls return false.
     * Items already in the queue can still be drained.
     */
    void shutdown()
    {
        QMutexLocker locker(&mMutex);
        mShutdown.store(true, std::memory_order_relaxed);
        mNotFull.wakeAll();
        mNotEmpty.wakeAll();
    }

    [[nodiscard]] int size() const
    {
        QMutexLocker locker(&mMutex);
        return mSize;
    }

    [[nodiscard]] int capacity() const { return mCapacity; }

    [[nodiscard]] bool isEmpty() const
    {
        QMutexLocker locker(&mMutex);
        return mSize == 0;
    }

private:
    Q_DISABLE_COPY_MOVE(BoundedBlockingQueue)

    const int mCapacity;
    std::vector<T> mBuffer;
    int mHead = 0;
    int mTail = 0;
    int mSize = 0;

    mutable QMutex mMutex;
    QWaitCondition mNotFull;
    QWaitCondition mNotEmpty;
    std::atomic<bool> mShutdown{false};
};

} // namespace Log4Qt

#endif // LOG4QT_HELPERS_BOUNDEDBLOCKINGQUEUE_H
