# AsyncWorker

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging library. It routes `LoggingEvent` objects through a hierarchy of loggers to one or more appenders that write the events to their destinations.

`AsyncWorker` is the background worker thread that backs `AsyncAppender`. Where `AsyncAppender` accepts logging events on the application's threads and buffers them in a `BoundedBlockingQueue<LoggingEvent>`, `AsyncWorker` runs on a dedicated thread, continuously draining that queue and dispatching each event to the appenders attached to the owning `AsyncAppender`. This decouples the latency of the actual log writes (file I/O, network, database) from the threads producing the log events.

A developer never instantiates `AsyncWorker` directly. It is an implementation detail created and owned by `AsyncAppender`. It replaces the older event-loop-based dispatcher with an explicit queue-draining loop so that the appender can support bounded backpressure (blocking, discarding, or synchronous fallback when the queue is full).

## 2. Project Structure and Dependencies

- **Instantiated by:** `AsyncAppender` (`asyncappender.h` / `asyncappender.cpp`) holds the sole `std::unique_ptr<AsyncWorker>` and starts it in `activateOptions()`.
- **Collaborators:**
  - `BoundedBlockingQueue<LoggingEvent>` (`helpers/boundedblockingqueue.h`) — the thread-safe queue this worker drains via `dequeue()` and `drain()`.
  - `AsyncAppender` (`asyncappender.h`) — receives dispatched events through `callAppenders()` and emits `batchComplete()` on this worker's thread.
  - `LoggingEvent` (`loggingevent.h`) — the value type carried through the queue.
- **Qt module dependency:** Qt Core (`QThread`). The Log4Qt library links `Qt::Core` publicly and `Qt::Concurrent` privately.
- **Build requirement:** compiled as part of the `log4qt` target; sources listed in `src/log4qt/CMakeLists.txt` (`helpers/asyncworker.cpp`, `helpers/asyncworker.h`).

## 3. Class Hierarchy and Role

`AsyncWorker` derives from `QThread`.

- `QThread` → provides an OS thread with its own `run()` entry point, plus the meta-object machinery (the class declares `Q_OBJECT`). Because the worker overrides `run()` with a manual loop, it does not use a per-thread event loop; the thread body is the queue-draining loop itself. `QThread` also gives the standard lifecycle API (`start()`, `wait()`, `isRunning()`) that `AsyncAppender` uses to manage the worker.

The class is `final` in spirit: copy and move are disabled via `Q_DISABLE_COPY_MOVE`, reflecting that it owns a thread of execution and references to external objects.

## 4. Q_PROPERTY Declarations

None.

## 5. Enumerations

None.

## 6. Public Member Variables

None. All data members are private (`mAppender`, `mQueue`) and are skipped.

## 7. Signals

None declared on `AsyncWorker` itself. Note that `run()` emits `AsyncAppender::batchComplete()` on the owning appender — that signal is documented with `AsyncAppender`.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### AsyncWorker(AsyncAppender *appender, BoundedBlockingQueue<LoggingEvent> *queue, QObject *parent = nullptr)

Constructs the worker, binding it to the `AsyncAppender` whose attached appenders will receive dispatched events and to the `BoundedBlockingQueue<LoggingEvent>` it will drain. Neither `appender` nor `queue` is owned by the worker — both must outlive it (they are owned by the `AsyncAppender`). The thread does not begin running until `QThread::start()` is called by the appender. The optional `parent` participates in the standard `QObject` parent-ownership chain.

## 10. Protected Virtual Methods / Event Handlers

#### void run() [override]

Overrides `QThread::run()` and constitutes the entire body of the worker thread. Its behaviour:

1. **Main drain loop** — repeatedly calls `queue->dequeue(event)`, which blocks until an event is available or the queue is shut down. For each dequeued event it calls `appender->callAppenders(event)`, dispatching it to every attached appender. After dispatch, if the queue is now empty it emits `AsyncAppender::batchComplete()` so downstream code can perform batch-flush optimisations.
2. **Shutdown drain** — once `dequeue()` returns `false` (the queue was shut down and is empty), the loop exits. The method then performs a final non-blocking `drain()` of any events that may still remain, dispatches each of them through `callAppenders()`, and emits `batchComplete()` one last time if anything was drained.

The method returns when the shutdown drain completes, which ends the thread. Subclassing is not intended (copy/move deleted, no virtual destructor beyond `QThread`'s); `Super::run()` should not be called.

## 11. Ownership and Lifecycle

- `AsyncAppender` owns the single `AsyncWorker` instance through a `std::unique_ptr<AsyncWorker>`.
- The worker holds **non-owning** raw pointers to its `AsyncAppender` and `BoundedBlockingQueue<LoggingEvent>`. Both must remain valid for the worker's entire lifetime; the appender guarantees this by owning the queue as well and by tearing the worker down before destroying the queue.
- Typical lifecycle, driven by `AsyncAppender`:
  1. `activateOptions()` constructs the queue and the worker, then calls `start()`.
  2. During logging, application threads enqueue events; the worker drains them.
  3. On `close()` / shutdown, the appender shuts the queue down (`shutdown()`), which unblocks the worker's `dequeue()`; the worker drains any remaining events and `run()` returns. The appender calls `wait()` (optionally bounded by `shutdownTimeout`) to join the thread before destroying it.
- Copy and move are deleted, so a worker cannot be duplicated or transferred.

## 12. Thread Safety

`AsyncWorker` *is* a thread. Its `run()` executes on the dedicated worker thread it represents; all event dispatch (`callAppenders()`) and `batchComplete()` emission therefore happen on that worker thread, not on the threads producing log events. Connected slots of `batchComplete()` will be invoked according to Qt's signal/slot threading rules (a directly connected slot runs on the worker thread).

The hand-off between producer threads and the worker is mediated entirely by `BoundedBlockingQueue<LoggingEvent>`, which is internally synchronised (`QMutex` + `QWaitCondition`). The worker itself adds no further synchronisation and must not be driven concurrently from multiple controllers — its `start()` / `wait()` lifecycle is managed solely by the owning `AsyncAppender`.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- **Reads from** `BoundedBlockingQueue<LoggingEvent>` via `dequeue()`, `isEmpty()`, `drain()`, and `capacity()`.
- **Calls** `AsyncAppender::callAppenders()` to fan each event out to the attached appenders.
- **Emits** `AsyncAppender::batchComplete()` (on behalf of the appender) when the queue empties after dispatch and once more after the shutdown drain. External code interested in batch boundaries connects to `AsyncAppender::batchComplete()`, not to anything on the worker.

## 15. External Communication

None directly. The worker triggers I/O only indirectly, through whatever appenders are attached to the `AsyncAppender` (file, network, database, etc.).
