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

#include <QTest>
#include <QSignalSpy>
#include <QThread>
#include <QElapsedTimer>

#include "log4qt/asyncappender.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/varia/listappender.h"
#include "log4qt/helpers/boundedblockingqueue.h"

using namespace Log4Qt;

LOG4QT_DECLARE_STATIC_LOGGER(test_logger, Test::AsyncAppender)

// ---------------------------------------------------------------------------
// SlowAppender — test helper that simulates slow downstream I/O
// ---------------------------------------------------------------------------
class SlowAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    explicit SlowAppender(int delayMs, QObject *parent = nullptr)
        : AppenderSkeleton(parent), mDelayMs(delayMs) {}
    bool requiresLayout() const override { return false; }
protected:
    void append(const LoggingEvent &) override
    {
        QThread::msleep(mDelayMs);
    }
private:
    int mDelayMs;
};

// ---------------------------------------------------------------------------
// ThreadCapturingAppender — records the thread name that called append()
// ---------------------------------------------------------------------------
class ThreadCapturingAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    explicit ThreadCapturingAppender(QObject *parent = nullptr)
        : AppenderSkeleton(parent) {}
    bool requiresLayout() const override { return false; }

    QStringList capturedThreadNames() const
    {
        QMutexLocker locker(&mObjectGuard);
        return mThreadNames;
    }
    int count() const
    {
        QMutexLocker locker(&mObjectGuard);
        return mThreadNames.size();
    }
protected:
    void append(const LoggingEvent &) override
    {
        mThreadNames.append(QThread::currentThread()->objectName());
    }
private:
    QStringList mThreadNames;
};

// ===========================================================================
// Test Class
// ===========================================================================
class AsyncAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();

    // BoundedBlockingQueue tests
    void BoundedBlockingQueue_enqueueDequeue();
    void BoundedBlockingQueue_capacity();
    void BoundedBlockingQueue_blockingEnqueue();
    void BoundedBlockingQueue_blockingDequeue();
    void BoundedBlockingQueue_shutdown();
    void BoundedBlockingQueue_drain();
    void BoundedBlockingQueue_drainAfterShutdown();

    // AsyncAppender core tests
    void AsyncAppender_defaultProperties();
    void AsyncAppender_basicAsyncLogging();
    void AsyncAppender_multipleAppenders();
    void AsyncAppender_preservesEventData();
    void AsyncAppender_workerThreadName();

    // Queue full policy tests
    void AsyncAppender_blockPolicy_nonBlocking();
    void AsyncAppender_discardPolicy_belowThreshold();
    void AsyncAppender_discardPolicy_aboveThreshold();
    void AsyncAppender_synchronousPolicy();
    void AsyncAppender_queueFullPolicyString();

    // Error appender tests
    void AsyncAppender_errorAppender();
    void AsyncAppender_errorRefResolution();
    void AsyncAppender_noErrorAppender();

    // Shutdown tests
    void AsyncAppender_gracefulShutdown();
    void AsyncAppender_reactivateAfterClose();

    // Batch signal tests
    void AsyncAppender_batchComplete();
};

void AsyncAppenderTest::cleanup()
{
    LogManager::resetConfiguration();
}

// ===========================================================================
// BoundedBlockingQueue Tests
// ===========================================================================

void AsyncAppenderTest::BoundedBlockingQueue_enqueueDequeue()
{
    BoundedBlockingQueue<int> queue(4);

    QVERIFY(queue.tryEnqueue(10));
    QVERIFY(queue.tryEnqueue(20));
    QVERIFY(queue.tryEnqueue(30));
    QCOMPARE(queue.size(), 3);

    int val = 0;
    QVERIFY(queue.dequeue(val));
    QCOMPARE(val, 10);
    QVERIFY(queue.dequeue(val));
    QCOMPARE(val, 20);
    QVERIFY(queue.dequeue(val));
    QCOMPARE(val, 30);

    QVERIFY(queue.isEmpty());
}

void AsyncAppenderTest::BoundedBlockingQueue_capacity()
{
    BoundedBlockingQueue<int> queue(3);

    QVERIFY(queue.tryEnqueue(1));
    QVERIFY(queue.tryEnqueue(2));
    QVERIFY(queue.tryEnqueue(3));
    QCOMPARE(queue.size(), 3);
    QCOMPARE(queue.capacity(), 3);

    // Queue is full — tryEnqueue should fail
    QVERIFY(!queue.tryEnqueue(4));
    QCOMPARE(queue.size(), 3);
}

void AsyncAppenderTest::BoundedBlockingQueue_blockingEnqueue()
{
    BoundedBlockingQueue<int> queue(2);
    queue.tryEnqueue(1);
    queue.tryEnqueue(2);
    // Queue is now full

    bool enqueued = false;

    // Start a thread that dequeues after a short delay, unblocking the producer
    QThread *consumer = QThread::create([&queue]() {
        QThread::msleep(50);
        int val = 0;
        queue.dequeue(val);
    });
    consumer->start();

    // This should block until the consumer frees a slot
    enqueued = queue.enqueue(3);
    QVERIFY(enqueued);

    consumer->wait();
    delete consumer;
}

void AsyncAppenderTest::BoundedBlockingQueue_blockingDequeue()
{
    BoundedBlockingQueue<int> queue(4);

    int result = 0;
    bool dequeued = false;

    // Start a thread that enqueues after a short delay
    QThread *producer = QThread::create([&queue]() {
        QThread::msleep(50);
        queue.tryEnqueue(42);
    });
    producer->start();

    // This should block until the producer enqueues
    dequeued = queue.dequeue(result);
    QVERIFY(dequeued);
    QCOMPARE(result, 42);

    producer->wait();
    delete producer;
}

void AsyncAppenderTest::BoundedBlockingQueue_shutdown()
{
    BoundedBlockingQueue<int> queue(4);

    bool dequeued = true;

    // Start a thread waiting on dequeue (queue is empty, so it blocks)
    QThread *consumer = QThread::create([&queue, &dequeued]() {
        int val = 0;
        dequeued = queue.dequeue(val);
    });
    consumer->start();

    QThread::msleep(50);
    queue.shutdown();
    consumer->wait();
    delete consumer;

    QVERIFY(!dequeued); // dequeue should return false on shutdown

    // After shutdown, enqueue should also fail
    QVERIFY(!queue.tryEnqueue(1));
    QVERIFY(!queue.enqueue(1));
}

void AsyncAppenderTest::BoundedBlockingQueue_drain()
{
    BoundedBlockingQueue<int> queue(8);
    for (int i = 1; i <= 5; ++i)
        queue.tryEnqueue(i);

    std::vector<int> out;
    int count = queue.drain(out, 3);
    QCOMPARE(count, 3);
    QCOMPARE(queue.size(), 2);
    QCOMPARE(out.size(), 3u);
    QCOMPARE(out[0], 1);
    QCOMPARE(out[1], 2);
    QCOMPARE(out[2], 3);

    // Drain remaining
    count = queue.drain(out, 10);
    QCOMPARE(count, 2);
    QCOMPARE(out.size(), 5u);
    QCOMPARE(out[3], 4);
    QCOMPARE(out[4], 5);
    QVERIFY(queue.isEmpty());
}

void AsyncAppenderTest::BoundedBlockingQueue_drainAfterShutdown()
{
    BoundedBlockingQueue<int> queue(4);
    queue.tryEnqueue(10);
    queue.tryEnqueue(20);
    queue.shutdown();

    std::vector<int> out;
    int count = queue.drain(out, 10);
    QCOMPARE(count, 2);
    QCOMPARE(out[0], 10);
    QCOMPARE(out[1], 20);
}

// ===========================================================================
// AsyncAppender Core Tests
// ===========================================================================

void AsyncAppenderTest::AsyncAppender_defaultProperties()
{
    AsyncAppender appender;
    QCOMPARE(appender.bufferSize(), 1024);
    QCOMPARE(appender.blocking(), true);
    QCOMPARE(appender.shutdownTimeout(), 0);
    QCOMPARE(appender.queueFullPolicy(), AsyncAppender::QueueFullPolicy::Block);
    QCOMPARE(appender.queueFullPolicyString(), QStringLiteral("Block"));
    QCOMPARE(appender.discardThreshold(), Level(Level::INFO_INT));
    QCOMPARE(appender.errorRef(), QString());
    QCOMPARE(appender.discardedCount(), 0);
}

void AsyncAppenderTest::AsyncAppender_basicAsyncLogging()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(16);

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    async.addAppender(AppenderSharedPtr(list));
    async.activateOptions();

    // Log several events
    const int count = 10;
    for (int i = 0; i < count; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();

    QCOMPARE(list->list().size(), count);
}

void AsyncAppenderTest::AsyncAppender_multipleAppenders()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(16);

    auto *list1 = new ListAppender;
    list1->setName(QStringLiteral("List1"));
    auto *list2 = new ListAppender;
    list2->setName(QStringLiteral("List2"));

    async.addAppender(AppenderSharedPtr(list1));
    async.addAppender(AppenderSharedPtr(list2));
    async.activateOptions();

    async.doAppend(LoggingEvent(test_logger(), Level::WARN_INT, QStringLiteral("hello")));
    async.close();

    QCOMPARE(list1->list().size(), 1);
    QCOMPARE(list2->list().size(), 1);
    QCOMPARE(list1->list().at(0).message(), QStringLiteral("hello"));
    QCOMPARE(list2->list().at(0).message(), QStringLiteral("hello"));
}

void AsyncAppenderTest::AsyncAppender_preservesEventData()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(16);

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    async.addAppender(AppenderSharedPtr(list));
    async.activateOptions();

    QThread::currentThread()->setObjectName(QStringLiteral("TestThread"));
    auto event = LoggingEvent(test_logger(), Level::ERROR_INT, QStringLiteral("test message"));

    async.doAppend(event);
    async.close();

    QCOMPARE(list->list().size(), 1);
    const auto &received = list->list().at(0);
    QCOMPARE(received.level(), Level(Level::ERROR_INT));
    QCOMPARE(received.message(), QStringLiteral("test message"));
    QCOMPARE(received.threadName(), QStringLiteral("TestThread"));
    QCOMPARE(received.timeStamp(), event.timeStamp());
}

void AsyncAppenderTest::AsyncAppender_workerThreadName()
{
    AsyncAppender async;
    async.setName(QStringLiteral("MyAsync"));
    async.setBufferSize(4);

    auto *capture = new ThreadCapturingAppender;
    capture->setName(QStringLiteral("Capture"));
    async.addAppender(AppenderSharedPtr(capture));
    async.activateOptions();

    async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT, QStringLiteral("x")));
    async.close();

    QCOMPARE(capture->count(), 1);
    QCOMPARE(capture->capturedThreadNames().at(0), QStringLiteral("Log4Qt-Async-MyAsync"));
}

// ===========================================================================
// Queue Full Policy Tests
// ===========================================================================

void AsyncAppenderTest::AsyncAppender_blockPolicy_nonBlocking()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(2);
    async.setBlocking(false);

    // Slow appender holds the worker busy so queue fills up
    auto *slow = new SlowAppender(200);
    slow->setName(QStringLiteral("Slow"));

    auto *error = new ListAppender;
    error->setName(QStringLiteral("Error"));

    async.addAppender(AppenderSharedPtr(slow));
    async.setErrorAppender(AppenderSharedPtr(error));
    async.activateOptions();

    // Fill queue + overflow
    for (int i = 0; i < 10; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();

    // At least some events should have gone to the error appender
    QVERIFY(error->list().size() > 0);
}

void AsyncAppenderTest::AsyncAppender_discardPolicy_belowThreshold()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(2);
    async.setQueueFullPolicy(AsyncAppender::QueueFullPolicy::Discard);
    async.setDiscardThreshold(Level(Level::INFO_INT));

    auto *slow = new SlowAppender(100);
    slow->setName(QStringLiteral("Slow"));
    async.addAppender(AppenderSharedPtr(slow));
    async.activateOptions();

    // Flood with DEBUG events (below INFO threshold) — should be discarded
    for (int i = 0; i < 20; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT,
                                    QStringLiteral("debug%1").arg(i)));

    async.close();

    QVERIFY(async.discardedCount() > 0);
}

void AsyncAppenderTest::AsyncAppender_discardPolicy_aboveThreshold()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(4);
    async.setQueueFullPolicy(AsyncAppender::QueueFullPolicy::Discard);
    async.setDiscardThreshold(Level(Level::INFO_INT));

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    async.addAppender(AppenderSharedPtr(list));
    async.activateOptions();

    // ERROR events are above threshold — should NOT be discarded
    const int count = 5;
    for (int i = 0; i < count; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::ERROR_INT,
                                    QStringLiteral("error%1").arg(i)));

    async.close();

    QCOMPARE(list->list().size(), count);
    QCOMPARE(async.discardedCount(), 0);
}

void AsyncAppenderTest::AsyncAppender_synchronousPolicy()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(2);
    async.setQueueFullPolicy(AsyncAppender::QueueFullPolicy::Synchronous);

    auto *slow = new SlowAppender(100);
    slow->setName(QStringLiteral("Slow"));

    auto *capture = new ThreadCapturingAppender;
    capture->setName(QStringLiteral("Capture"));

    async.addAppender(AppenderSharedPtr(slow));
    async.addAppender(AppenderSharedPtr(capture));
    async.activateOptions();

    // Flood to trigger synchronous fallback on the calling thread
    const QString mainThread = QThread::currentThread()->objectName();
    for (int i = 0; i < 20; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();

    // At least some events should have been dispatched on the calling thread
    bool foundMainThread = false;
    for (const auto &tn : capture->capturedThreadNames())
    {
        if (tn == mainThread)
        {
            foundMainThread = true;
            break;
        }
    }
    QVERIFY(foundMainThread);
}

void AsyncAppenderTest::AsyncAppender_queueFullPolicyString()
{
    AsyncAppender async;

    async.setQueueFullPolicyString(QStringLiteral("Block"));
    QCOMPARE(async.queueFullPolicy(), AsyncAppender::QueueFullPolicy::Block);
    QCOMPARE(async.queueFullPolicyString(), QStringLiteral("Block"));

    async.setQueueFullPolicyString(QStringLiteral("discard"));
    QCOMPARE(async.queueFullPolicy(), AsyncAppender::QueueFullPolicy::Discard);
    QCOMPARE(async.queueFullPolicyString(), QStringLiteral("Discard"));

    async.setQueueFullPolicyString(QStringLiteral("SYNCHRONOUS"));
    QCOMPARE(async.queueFullPolicy(), AsyncAppender::QueueFullPolicy::Synchronous);
    QCOMPARE(async.queueFullPolicyString(), QStringLiteral("Synchronous"));

    // Unknown value defaults to Block
    async.setQueueFullPolicyString(QStringLiteral("invalid"));
    QCOMPARE(async.queueFullPolicy(), AsyncAppender::QueueFullPolicy::Block);
}

// ===========================================================================
// Error Appender Tests
// ===========================================================================

void AsyncAppenderTest::AsyncAppender_errorAppender()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(2);
    async.setBlocking(false);

    auto *slow = new SlowAppender(200);
    slow->setName(QStringLiteral("Slow"));

    auto *errorList = new ListAppender;
    errorList->setName(QStringLiteral("ErrorList"));

    async.addAppender(AppenderSharedPtr(slow));
    async.setErrorAppender(AppenderSharedPtr(errorList));
    async.activateOptions();

    for (int i = 0; i < 10; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();

    QVERIFY(errorList->list().size() > 0);
}

// Regression test: the errorRef property was documented as "resolved at
// activateOptions() time" but was never resolved — a fallback appender
// configured by name was silently ignored and overflow events vanished.
void AsyncAppenderTest::AsyncAppender_errorRefResolution()
{
    // The referenced appender is attached to a logger, where the resolution
    // must find it by name
    auto *errorList = new ListAppender;
    errorList->setName(QStringLiteral("ErrorRefList"));
    AppenderSharedPtr errorPtr(errorList);
    Logger *holder = LogManager::logger(QStringLiteral("Test::ErrorRefHolder"));
    holder->addAppender(errorPtr);

    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(2);
    async.setBlocking(false);
    async.setErrorRef(QStringLiteral("ErrorRefList"));

    auto *slow = new SlowAppender(200);
    slow->setName(QStringLiteral("Slow"));
    async.addAppender(AppenderSharedPtr(slow));
    async.activateOptions();

    for (int i = 0; i < 10; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();

    QVERIFY(errorList->list().size() > 0);
    holder->removeAppender(errorPtr);
}

void AsyncAppenderTest::AsyncAppender_noErrorAppender()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(2);
    async.setBlocking(false);

    auto *slow = new SlowAppender(200);
    slow->setName(QStringLiteral("Slow"));
    async.addAppender(AppenderSharedPtr(slow));
    // No error appender set — should not crash
    async.activateOptions();

    for (int i = 0; i < 10; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();
    // Just verifying no crash/hang
}

// ===========================================================================
// Shutdown Tests
// ===========================================================================

void AsyncAppenderTest::AsyncAppender_gracefulShutdown()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(32);

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    async.addAppender(AppenderSharedPtr(list));
    async.activateOptions();

    const int count = 20;
    for (int i = 0; i < count; ++i)
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("msg%1").arg(i)));

    async.close();

    // All events must have been drained before close() returned
    QCOMPARE(list->list().size(), count);
}

// Regression test: activateOptions() after close() used to restart the
// worker thread without clearing the closed flag — the re-activated
// appender dropped every event, and destruction skipped the shutdown
// (closeInternal() early-returned on isClosed()), destroying a running
// QThread together with the queue it was blocked on.
void AsyncAppenderTest::AsyncAppender_reactivateAfterClose()
{
    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    AppenderSharedPtr listPtr(list);

    {
        AsyncAppender async;
        async.setName(QStringLiteral("TestAsync"));
        async.setBufferSize(16);
        async.addAppender(listPtr);

        async.activateOptions();
        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("before close")));
        async.close();
        QVERIFY(async.isClosed());

        async.activateOptions();
        QVERIFY(async.isActive());
        QVERIFY(!async.isClosed());

        async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                    QStringLiteral("after reactivate")));

        // Destruction of the re-activated appender must join the restarted
        // worker thread and drain the queue.
    }

    QCOMPARE(list->list().size(), 2);
    QCOMPARE(list->list().at(0).message(), QStringLiteral("before close"));
    QCOMPARE(list->list().at(1).message(), QStringLiteral("after reactivate"));
}

// ===========================================================================
// Batch Signal Tests
// ===========================================================================

void AsyncAppenderTest::AsyncAppender_batchComplete()
{
    AsyncAppender async;
    async.setName(QStringLiteral("TestAsync"));
    async.setBufferSize(16);

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    async.addAppender(AppenderSharedPtr(list));

    QSignalSpy spy(&async, &AsyncAppender::batchComplete);
    QVERIFY(spy.isValid());

    async.activateOptions();

    async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT, QStringLiteral("a")));
    async.doAppend(LoggingEvent(test_logger(), Level::INFO_INT, QStringLiteral("b")));

    async.close();

    // batchComplete should have been emitted at least once
    QVERIFY(spy.count() > 0);
}

QTEST_MAIN(AsyncAppenderTest)
#include "tst_asyncappender.moc"
