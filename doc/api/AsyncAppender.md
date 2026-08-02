# AsyncAppender

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. Within it, an *appender* is the sink that writes a formatted log event somewhere (a file, the console, a socket, a database, and so on). `AsyncAppender` is a wrapping appender: it does not write anywhere itself. Instead it accepts events on the calling thread, places them into a bounded blocking queue, and replays them to a set of *attached* downstream appenders from a single dedicated worker thread.

A developer reaches for `AsyncAppender` when logging happens on latency-sensitive threads (a GUI thread, a request handler) and the underlying sink is slow (file I/O, a network socket, a database). By decoupling the producer (the application thread that logs) from the consumer (the worker thread that performs the I/O), the cost of logging on the hot path is reduced to an enqueue operation. The class also gives explicit, configurable control over what happens when the queue saturates, via the `queueFullPolicy` property.

## 2. Project Structure and Dependencies

- **Header includes:** `log4qtshared.h` (export macro), `appenderskeleton.h` (base class), `helpers/appenderattachable.h` (multi-appender container), plus `<atomic>` and `<memory>`.
- **Implementation includes:** `helpers/asyncworker.h` (the worker thread), `helpers/boundedblockingqueue.h` (the queue), `loggingevent.h`, and `<QReadLocker>`.
- **Forward declarations:** `AsyncWorker`, `LoggingEvent`, and the class template `BoundedBlockingQueue<T>` are forward-declared in the header and only fully included in the `.cpp`, keeping the public header light.
- **Qt module:** Qt Core (`QObject`, `QMutex`, `QThread` via the worker). No widgets, SQL, or networking.
- **Project-internal types:**
  - `AsyncWorker` — a `QThread` subclass whose `run()` loop drains the queue and calls `callAppenders()` on the owning `AsyncAppender`.
  - `BoundedBlockingQueue<LoggingEvent>` — a thread-safe circular-buffer queue supporting blocking and non-blocking enqueue, shutdown, and bulk drain.
  - `AppenderSharedPtr` / `LayoutSharedPtr` — `QSharedPointer` aliases used throughout Log4Qt for appender and layout ownership.

## 3. Class Hierarchy and Role

`AsyncAppender` inherits from two bases:

- **`AppenderSkeleton`** (which derives from `Appender` → `QObject`) — provides the meta-object system, signals/slots, `parent`-based ownership, the `doAppend()` entry-condition pipeline, threshold/filter handling, and the `mObjectGuard` recursive mutex. `AsyncAppender` overrides `append()`, `activateOptions()`, `close()`, `requiresLayout()`, and `checkEntryConditions()`.
- **`AppenderAttachable`** — provides the container of attached appenders (`mAppenders`) guarded by `mAppenderGuard` (a `QReadWriteLock`), with `addAppender()`, `removeAppender()`, `appenders()`, and related methods. This is what lets multiple downstream appenders receive each event.

The class's role is that of an asynchronous dispatcher: producers call the inherited `doAppend()`, which eventually calls the overridden `append()`; that enqueues onto the worker thread, which fans the event out to every attached appender.

`AsyncAppender` is declared **`final`**. The worker thread is joined in `~AsyncAppender`, so a subclass destructor would run while the worker is still calling back into the object (`callAppenders()`, `batchComplete()`) — a use-after-free on the subclass part. Extend behaviour by *attaching* appenders, not by subclassing.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `bufferSize` | `int` | `bufferSize` | `setBufferSize` | — | Maximum number of events the queue can hold. Applied when `activateOptions()` is called. Default 1024. Values `<= 0` are clamped to 1 with a warning. |
| `blocking` | `bool` | `blocking` | `setBlocking` | — | When `true` (default), under the `Block` policy the calling thread blocks until queue space frees up. When `false`, a full queue routes the event to the error appender instead of blocking. |
| `shutdownTimeout` | `int` | `shutdownTimeout` | `setShutdownTimeout` | — | Milliseconds to wait for the queue to drain during shutdown. `0` (default) waits indefinitely. On timeout the worker is terminated and a warning logged. |
| `discardThreshold` | `Log4Qt::Level` | `discardThreshold` | `setDiscardThreshold` | — | Under the `Discard` policy, events at or below this level are dropped when the queue is full; higher-priority events still block. Default `INFO`. |
| `queueFullPolicy` | `QString` | `queueFullPolicyString` | `setQueueFullPolicyString` | — | The queue-full policy as text: `"Block"`, `"Discard"`, or `"Synchronous"` (case-insensitive; unrecognised values fall back to `Block`). Default `"Block"`. |
| `errorRef` | `QString` | `errorRef` | `setErrorRef` | — | Name of a fallback appender that receives events the queue cannot accept. Resolved by searching the repository's loggers for an appender with that name: first at `activateOptions()` (warning if not found), then retried silently on each queue-full event, so an appender configured *after* this one is still picked up. |

## 5. Enumerations

### QueueFullPolicy (Q_ENUM)

Controls what happens when the bounded queue is full. Used by `queueFullPolicy()` / `setQueueFullPolicy()` and the string-based `queueFullPolicy` property.

| Value | Integer | Description |
|-------|---------|-------------|
| `Block` | 0 | The caller blocks until space is available (default). If `blocking` is `false`, the event is instead routed to the error appender via `handleQueueFull()`. |
| `Discard` | 1 | Events at or below `discardThreshold` are silently dropped (and counted in `discardedCount()`); events above the threshold still block until enqueued. |
| `Synchronous` | 2 | If the event cannot be enqueued without blocking, it is dispatched directly on the calling thread via `callAppenders()`, bypassing the worker. |

## 6. Public Member Variables

None. All state is private (`m`-prefixed) and exposed through accessors.

## 7. Signals

#### void batchComplete()

Emitted from the worker thread when the queue becomes empty after a dispatch pass (and once more after the post-shutdown drain if any events remained). Connect to this for batch-flush optimisations — for example, telling a downstream buffered sink to flush only once a burst of events has been fully drained, rather than after every event. Because it fires on the worker thread, any connected slot runs on that thread unless a queued connection is used.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### AsyncAppender(QObject *parent = nullptr)

Constructs the appender. No worker thread or queue exists yet; both are created lazily in `activateOptions()`.

#### bool requiresLayout() const override

Returns `false`. `AsyncAppender` does not format events itself — formatting is the responsibility of the attached downstream appenders.

#### int bufferSize() const

Returns the configured maximum queue capacity.

#### void setBufferSize(int size)

Sets the queue capacity (an atomic store). Non-positive values are clamped to 1 and a warning is logged. The new size takes effect only at the next `activateOptions()`, since the queue is sized when it is created.

#### bool blocking() const / void setBlocking(bool blocking)

Get/set whether a full queue under the `Block` policy blocks the caller (`true`) or diverts to the error appender (`false`).

#### int shutdownTimeout() const / void setShutdownTimeout(int timeoutMs)

Get/set the drain timeout (milliseconds) used during shutdown.

#### Level discardThreshold() const / void setDiscardThreshold(Level level)

Get/set the level threshold used by the `Discard` policy.

#### QueueFullPolicy queueFullPolicy() const / void setQueueFullPolicy(QueueFullPolicy policy)

Get/set the queue-full policy using the strongly typed enum.

#### QString queueFullPolicyString() const / void setQueueFullPolicyString(const QString &policy)

String-based get/set used by the `queueFullPolicy` property and configurators. Recognises `"Discard"` and `"Synchronous"` case-insensitively; anything else maps to `Block`.

#### QString errorRef() const / void setErrorRef(const QString &name)

Get/set the name of the fallback error appender. Both are inline but take `mObjectGuard`, so the reference can be reconfigured from any thread. Setting a *different* name also clears the cached `mErrorAppender`, so the new reference is re-resolved on demand; setting the same name is a no-op.

#### void setErrorAppender(const AppenderSharedPtr &appender)

Directly assigns the fallback appender that receives events when the queue is full and they cannot be enqueued (used under the non-blocking `Block` path). Holds a shared reference, and takes precedence over `errorRef` because name resolution is skipped once a fallback appender is cached. Acquires `mObjectGuard`.

#### qint64 discardedCount() const

Returns the running total of events dropped under the `Discard` policy. Read with relaxed atomic semantics — safe to call from any thread.

#### void activateOptions() override

Creates the `BoundedBlockingQueue` (sized to `bufferSize`) and the `AsyncWorker`, names the worker thread `Log4Qt-Async-<name>`, starts it, resolves `errorRef` into the fallback appender (logging a warning if the name matches no appender on any logger), then chains to `AppenderSkeleton::activateOptions()`. Idempotent: if a worker already exists it returns immediately. Guarded by `mObjectGuard`.

#### void close() override

Shuts down the worker and queue via `closeInternal()`, then chains to `AppenderSkeleton::close()`.

#### void callAppenders(const LoggingEvent &event) const

Fans `event` out to every attached appender, forwarding through `forwardEvent()` under a read lock on `mAppenderGuard`. Called by the worker thread for normal dispatch and by `append()` directly under the `Synchronous` policy.

#### bool checkEntryConditions() const override

Returns `false` (logging an error) if the worker thread exists but is not running; otherwise chains to `AppenderSkeleton::checkEntryConditions()`. Guarded by `mObjectGuard`.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Defined by `AppenderSkeleton`. Invoked from the producer thread inside `doAppend()` (Phase 5, under `mObjectGuard`). It does not perform I/O; it enqueues the event according to `mQueueFullPolicy`:

- **Block** — `enqueue()` (blocks) if `blocking`, otherwise `tryEnqueue()` and on failure `handleQueueFull()`.
- **Discard** — `tryEnqueue()`; on failure, drop and count events at/below `discardThreshold`, or `enqueue()` (block) for higher levels.
- **Synchronous** — `tryEnqueue()`; on failure, dispatch inline via `callAppenders()`.

If no queue exists (not activated), the call returns immediately.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; if constructed with a `parent`, that parent deletes it. In typical Log4Qt usage appenders are held by `AppenderSharedPtr` (`QSharedPointer`) and the configurator/logger repository manages lifetime — see the project's object-ownership documentation.
- The `BoundedBlockingQueue` and `AsyncWorker` are owned via `std::unique_ptr` and created in `activateOptions()`, destroyed in `closeInternal()`.
- The destructor calls `closeInternal()`, which signals the queue to shut down, waits for the worker (up to `shutdownTimeout`, else indefinitely), and on timeout `terminate()`s and re-`wait()`s the worker before resetting both pointers. This guarantees the worker thread is joined before the appender is destroyed.
- Attached downstream appenders are held by `AppenderSharedPtr` in `AppenderAttachable::mAppenders`; the error appender is held by `mErrorAppender` (also a shared pointer). `AsyncAppender` shares, not exclusively owns, those.

## 12. Thread Safety

All public functions are thread-safe. This class is explicitly a multi-threaded handoff:

- **Producer side:** any number of application threads may call the inherited `doAppend()` concurrently. `AppenderSkeleton::doAppend()` serialises the final `append()` under `mObjectGuard`, and `append()` itself only touches the internally synchronised `BoundedBlockingQueue`.
- **Consumer side:** a single `AsyncWorker` thread runs `dequeue()` in a loop and calls `callAppenders()`, which takes a read lock on `mAppenderGuard` while iterating attached appenders.
- **Queue:** `BoundedBlockingQueue` uses a `QMutex` plus two `QWaitCondition`s (`mNotFull`, `mNotEmpty`) and an atomic shutdown flag, providing blocking and non-blocking enqueue, blocking dequeue, bulk drain, and a clean shutdown that wakes all waiters.
- **Backpressure:** under the blocking `Block` policy a full queue throttles producers, which is the mechanism that prevents unbounded memory growth.
- **Configuration:** `mBufferSize`, `mBlocking`, `mShutdownTimeout`, `mDiscardThreshold` and `mQueueFullPolicy` are `std::atomic`, because `append()` and `closeInternal()` read them while the public setters may run concurrently on another thread. `mErrorRef` and `mErrorAppender` are `QString`/`AppenderSharedPtr` instead and are therefore guarded by `mObjectGuard` — `errorRef()`, `setErrorRef()`, `setErrorAppender()` and the private `resolveErrorAppender()` all run under that lock.
- `discardedCount` is a `std::atomic<qint64>`; lifecycle transitions are guarded by `mObjectGuard`.

The `batchComplete()` signal is emitted on the worker thread — connect with `Qt::QueuedConnection` if the receiver lives on another thread.

## 14. Inter-Class Interactions

- Wraps and drives any number of attached `Appender` instances (added via `AppenderAttachable::addAppender()`); these are the real sinks.
- Owns and starts an `AsyncWorker`, which calls back into `callAppenders()` and emits `batchComplete()` through the appender.
- Optionally forwards overflow events to an error appender resolved from `errorRef` (set by a configurator) or assigned via `setErrorAppender()`.
- Uses `forwardEvent()` (inherited static helper) to push events into downstream appenders' `doAppend()` while bypassing the recursion guard for these intentional redirects.

## 16. Usage Example

```cpp
#include "log4qt/asyncappender.h"
#include "log4qt/fileappender.h"
#include "log4qt/patternlayout.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(QStringLiteral("%d{ISO8601} [%t] %p %c - %m%n")));
layout->activateOptions();

auto fileAppender = AppenderSharedPtr(new FileAppender(layout, QStringLiteral("app.log")));
fileAppender->setName(QStringLiteral("file"));
fileAppender->activateOptions();

auto *async = new AsyncAppender;                 // parent-less; held below by the logger
async->setName(QStringLiteral("async"));
async->setBufferSize(4096);
async->setQueueFullPolicyString(QStringLiteral("Discard"));
async->setDiscardThreshold(Level::DEBUG_INT);    // drop DEBUG and below under pressure
async->addAppender(fileAppender);                // fan-out target on the worker thread
async->activateOptions();                        // starts the worker thread

QObject::connect(async, &AsyncAppender::batchComplete, [] {
    // queue drained — a good point to flush downstream buffers
});

Logger::rootLogger()->addAppender(AppenderSharedPtr(async));
Logger::rootLogger()->info(QStringLiteral("logged from the hot path, written from the worker"));

// On shutdown, async->close() (or destruction) drains the queue before joining the worker.
```
