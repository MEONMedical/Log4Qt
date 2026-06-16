# BoundedBlockingQueue

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging library. Its `AsyncAppender` decouples the threads that produce log events from the thread that writes them, and `BoundedBlockingQueue<T>` is the data structure that sits between the two.

`BoundedBlockingQueue<T>` is a thread-safe, fixed-capacity producer/consumer queue implemented as a circular buffer. Producers add items with `enqueue()` (blocking) or `tryEnqueue()` (fail-fast); a consumer removes items with `dequeue()` (blocking) or harvests a batch with `drain()` (non-blocking). Capacity is fixed at construction, providing **backpressure**: when the queue is full a blocking producer waits rather than allowing unbounded memory growth. A `shutdown()` call cleanly unblocks all waiters so the queue can be torn down.

In Log4Qt the queue is specialised as `BoundedBlockingQueue<LoggingEvent>` and owned by `AsyncAppender`; the `AsyncWorker` thread is the single consumer, while any number of application threads are producers (an MPSC usage pattern).

## 2. Project Structure and Dependencies

- **Header-only template.** The entire implementation lives in `helpers/boundedblockingqueue.h`; there is no `.cpp`. It is listed in `src/log4qt/CMakeLists.txt` as a public header.
- **Instantiated by:** `AsyncAppender` (`asyncappender.h`) as `std::unique_ptr<BoundedBlockingQueue<LoggingEvent>>`.
- **Consumed by:** `AsyncWorker` (`helpers/asyncworker.h`), which drains it.
- **Qt module dependency:** Qt Core — uses `QMutex`, `QMutexLocker`, and `QWaitCondition`.
- **Standard library:** `<atomic>` (`std::atomic<bool>` shutdown flag), `<vector>` (`std::vector<T>` backing buffer and `drain()` output).

## 3. Class Hierarchy and Role

`BoundedBlockingQueue<T>` is a standalone class template. It is **not** a `QObject` — it has no base class, no `Q_OBJECT` macro, no signals, and no slots. It is a plain synchronisation primitive intended to be embedded as a member of another class.

Copy and move are disabled via `Q_DISABLE_COPY_MOVE`, because the object owns OS synchronisation primitives (`QMutex`, `QWaitCondition`) and buffer state that cannot be meaningfully duplicated.

### Template Parameters

| Parameter | Constraint | Description |
|-----------|------------|-------------|
| `T` | Must be default-constructible and move-assignable | The element type stored in the queue. Slots are reset to `T{}` after an element leaves the queue, so `T` must support default construction. Elements are moved out on dequeue/drain. |

## 4. Q_PROPERTY Declarations

None (not a `QObject`).

## 5. Enumerations

None.

## 6. Public Member Variables

None. All state (`mCapacity`, `mBuffer`, `mHead`, `mTail`, `mSize`, `mMutex`, `mNotFull`, `mNotEmpty`, `mShutdown`) is private and skipped.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### explicit BoundedBlockingQueue(int capacity)

Constructs the queue with a fixed capacity. The capacity is clamped to a minimum of 1: any value `<= 0` becomes 1. The backing circular buffer is allocated up front to `capacity` slots. Capacity cannot be changed afterwards.

#### bool enqueue(const T &item)

Adds an item to the tail of the queue, **blocking** the calling thread while the queue is full until either space becomes available or `shutdown()` is called. Returns `true` if the item was enqueued, or `false` if the queue was shut down before space appeared (in which case the item is not stored). On success it wakes one waiting consumer. This is the backpressure path: a full queue throttles producers instead of growing without bound.

#### bool tryEnqueue(const T &item)

Attempts to add an item without blocking. Returns `true` if there was free space and the queue was not shut down; returns `false` immediately if the queue is full or has been shut down. On success it wakes one waiting consumer. Use this for fail-fast / discard policies where a producer must not stall.

#### bool dequeue(T &item)

Removes the item at the head of the queue into `item`, **blocking** the calling thread while the queue is empty until either an item arrives or `shutdown()` is called. Returns `true` and move-assigns the head element into `item` on success; returns `false` when the queue is empty *and* has been shut down (the consumer's signal to stop). On success the vacated slot is reset to `T{}` and one waiting producer is woken.

#### int drain(std::vector<T> &out, int maxItems)

Removes up to `maxItems` elements (or all currently queued elements, whichever is smaller) from the head and appends them to `out`, **without blocking**. Returns the number of elements moved. Elements are moved out in FIFO order and their slots reset to `T{}`. If at least one element was drained, all waiting producers are woken (`wakeAll`). This is used during shutdown to flush whatever remains in the queue in a single pass.

#### void shutdown()

Marks the queue as shut down and wakes **all** waiting producers and consumers. After this call, every blocking `enqueue()` and `dequeue()` that was waiting (or is subsequently called) returns `false`. Items already stored in the queue are *not* discarded and can still be retrieved with `dequeue()` (until empty) or `drain()`. This is the controlled-teardown entry point.

#### [[nodiscard]] int size() const

Returns the number of elements currently in the queue. Acquires the internal mutex, so the value is a consistent snapshot at the moment of the call.

#### [[nodiscard]] int capacity() const

Returns the fixed capacity set at construction. Does not lock (the value is immutable after construction).

#### [[nodiscard]] bool isEmpty() const

Returns `true` if the queue currently holds no elements. Acquires the internal mutex for a consistent snapshot.

## 10. Protected Virtual Methods / Event Handlers

None.

## 11. Ownership and Lifecycle

- The queue owns its backing `std::vector<T>` buffer and its synchronisation primitives by value; destruction releases them automatically (RAII). No manual cleanup is required.
- It is **non-copyable and non-movable**. It is meant to be held by stable storage — in Log4Qt, a `std::unique_ptr` inside `AsyncAppender`.
- Correct teardown ordering matters: call `shutdown()` first so any blocked producers/consumers return, join the consumer thread, and only then destroy the queue. Destroying a queue while threads are still blocked in `enqueue()`/`dequeue()` is undefined; `AsyncAppender` enforces the shutdown-then-join order.

## 12. Thread Safety

**Fully thread-safe.** This is the central guarantee of the class.

- A single `QMutex` (`mMutex`) guards all buffer state (`mHead`, `mTail`, `mSize`, and the buffer slots). Every public operation except `capacity()` takes a `QMutexLocker` on entry, so concurrent producers and consumers never corrupt the buffer. `size()` and `isEmpty()` lock to return consistent snapshots; `capacity()` is lock-free because the value is immutable.
- Two condition variables coordinate blocking:
  - `mNotFull` — a producer in `enqueue()` waits on this while `mSize == mCapacity`. Consumers signal it (`wakeOne` after `dequeue()`, `wakeAll` after `drain()`) when they free space.
  - `mNotEmpty` — a consumer in `dequeue()` waits on this while `mSize == 0`. Producers signal it (`wakeOne`) after adding an item.
- **Blocking-on-full semantics:** `enqueue()` parks the caller on `mNotFull` in a loop, re-checking `mSize == mCapacity` and the shutdown flag on every wakeup, so it is immune to spurious wakeups. It returns `false` if it observes shutdown.
- **Blocking-on-empty semantics:** `dequeue()` parks the caller on `mNotEmpty` in a loop, re-checking `mSize == 0` and the shutdown flag. It returns `false` only when the queue is empty after a shutdown.
- The shutdown flag is a `std::atomic<bool>` (`mShutdown`) read with relaxed ordering inside the wait predicates; `shutdown()` sets it and wakes all waiters on both condition variables so no thread is left blocked.
- The design supports multiple producers and one consumer (MPSC) as used by `AsyncAppender`/`AsyncWorker`, and is equally correct for single-producer/single-consumer use.

## 13. QML Exposure

Not exposed to QML (a non-`QObject` template cannot be registered).

## 14. Inter-Class Interactions

- **`AsyncAppender`** constructs and owns the queue, sizes it from its `bufferSize` property, and (depending on its `queueFullPolicy`) calls `enqueue()` (Block policy), `tryEnqueue()` (Discard/fail-fast), or `shutdown()` during close.
- **`AsyncWorker`** is the consumer: it calls `dequeue()` in its main loop, `isEmpty()` to detect batch boundaries, and `drain()` plus `capacity()` to flush remaining events at shutdown.

## 15. External Communication

None. It is a purely in-process synchronisation primitive.

## 16. Usage Example

```cpp
#include "helpers/boundedblockingqueue.h"
#include <QThread>

using namespace Log4Qt;

// A queue holding up to 1024 integers.
BoundedBlockingQueue<int> queue(1024);

// Consumer thread: drain until shutdown.
struct Consumer : QThread
{
    BoundedBlockingQueue<int> *q;
    explicit Consumer(BoundedBlockingQueue<int> *queue) : q(queue) {}
    void run() override
    {
        int value;
        while (q->dequeue(value))   // blocks while empty; false on shutdown
            process(value);

        // Flush anything left after shutdown was signalled.
        std::vector<int> rest;
        q->drain(rest, q->capacity());
        for (int v : rest)
            process(v);
    }
    static void process(int) { /* ... */ }
};

Consumer consumer(&queue);
consumer.start();

// Producer side (any number of threads):
queue.enqueue(42);          // blocks if full; returns false if shut down
if (!queue.tryEnqueue(43))  // never blocks; false if full or shut down
    /* apply discard policy */;

// Clean shutdown: unblock waiters, then join.
queue.shutdown();
consumer.wait();
```
