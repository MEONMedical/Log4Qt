# MainThreadAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `MainThreadAppender` is a *marshalling* appender: it does not write anywhere itself. Instead it ensures that the attached downstream appenders always run on the main (GUI) thread, regardless of which thread originated the log call.

This matters because many sinks are not safe to touch from arbitrary threads — most notably appenders that update Qt Widgets, models, or other GUI objects. A developer attaches GUI-bound appenders to a `MainThreadAppender` so that an event logged from a worker thread is delivered to those appenders on the main thread, where it is safe to manipulate UI state.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class) and `helpers/appenderattachable.h` (multi-appender container).
- **Implementation includes:** `loggingevent.h`, `<QCoreApplication>`, `<QReadLocker>`, `<QThread>`.
- **Qt module:** Qt Core only (`QCoreApplication`, `QThread`, the event system).
- **Project-internal types:**
  - `LoggingEvent` — the immutable record of a single log call (level, logger name, thread name, message, timestamp). It is also a `QEvent` subclass, which is what makes cross-thread delivery via `postEvent()` possible.
  - `AppenderSharedPtr` — `QSharedPointer<Appender>` alias used for attached appenders.

## 3. Class Hierarchy and Role

`MainThreadAppender` inherits from two bases:

- **`AppenderSkeleton`** (→ `Appender` → `QObject`) — provides the meta-object system, the `doAppend()` entry pipeline, threshold/filter handling, and the `mObjectGuard` mutex. `MainThreadAppender` overrides `append()`, `activateOptions()`, `requiresLayout()`, and `checkEntryConditions()`.
- **`AppenderAttachable`** — provides the container of attached appenders (`mAppenders`) guarded by `mAppenderGuard` (`QReadWriteLock`), with `addAppender()`/`removeAppender()`/`appenders()`.

Its role is a thread-affinity bridge: events arrive on any thread and are re-dispatched to the attached appenders on the GUI thread.

## 4. Q_PROPERTY Declarations

None beyond those inherited from `AppenderSkeleton` (`isActive`, `isClosed`, `threshold`).

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None declared.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### MainThreadAppender(QObject *parent = nullptr)

Constructs the appender, chaining to `AppenderSkeleton`.

#### bool requiresLayout() const override

Returns `false`. This appender forwards the event object itself; formatting (if any) is done by the downstream appenders on the main thread.

#### void activateOptions() override

Empty override. No resources need to be allocated; the class is ready to dispatch as soon as appenders are attached.

#### bool checkEntryConditions() const override

Adds no conditions of its own and simply returns `AppenderSkeleton::checkEntryConditions()`. Part of the `doAppend()` entry-condition chain.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Defined by `AppenderSkeleton`; invoked from `doAppend()` on whatever thread logged the event. Its job is to deliver the event to each attached appender on the main thread:

1. Fetches `QCoreApplication::instance()`; if there is none, returns (no event loop to post into).
2. Takes a read lock on `mAppenderGuard`.
3. Determines the application's main thread via `app->thread()`.
4. For each attached appender: if the current thread is **not** the main thread, posts a freshly heap-allocated `new LoggingEvent(event)` to that appender via `QCoreApplication::postEvent()`; if already on the main thread, hands the event straight to the appender via the inherited `forwardEvent()` helper.

The posted `LoggingEvent` is later received by the target appender's `customEvent()` (provided by `AppenderSkeleton`), which dispatches it back through the normal append pipeline — now on the main thread. Qt's event system takes ownership of the posted event and deletes it after delivery.

> Same-thread delivery must go through `forwardEvent()` rather than being skipped or specially cased. `MainThreadAppender::append()` runs inside its *own* `doAppend()`, so the recursion guard already lists this appender; forwarding to a *different* downstream appender passes the per-appender guard normally. (Before the guard became per-appender, this path dropped every event logged on the main thread.)

## 11. Ownership and Lifecycle

- The appender is a `QObject`; if given a `parent`, the parent deletes it. In normal Log4Qt usage it is held via `AppenderSharedPtr` and managed by the logger repository.
- It allocates no long-lived resources of its own.
- Each cross-thread dispatch heap-allocates one `LoggingEvent` copy and hands it to Qt's event queue, which owns and deletes it after `customEvent()` runs. There is no leak as long as the target appender's event loop is alive to process posted events.
- Attached appenders are held by `AppenderSharedPtr` in `AppenderAttachable::mAppenders`; this class shares, not exclusively owns, them.

## 12. Thread Safety

All public functions are thread-safe. The threading model is the entire point of the class:

- `append()` may be entered from any thread (it runs inside `AppenderSkeleton::doAppend()`, serialised under `mObjectGuard`).
- The attached-appender list is read under a shared `QReadLocker` on `mAppenderGuard`.
- The marshalling decision is made per attached appender by comparing `QThread::currentThread()` against `QCoreApplication::instance()->thread()`:
  - **Off the main thread** → `postEvent()` enqueues a copy into the main thread's event loop. Delivery is asynchronous; the downstream `append()` runs on the main thread.
  - **On the main thread** → direct synchronous delivery via `forwardEvent()` (i.e. the downstream `doAppend()`), with all of the target's own checks applied.

This guarantees downstream GUI-bound appenders only ever execute on the GUI thread, which is exactly where Qt Widgets / Quick objects must be touched. The trade-off is that off-thread logging is delivered asynchronously and requires a running event loop on the main thread.

## 14. Inter-Class Interactions

- Wraps any number of attached `Appender` instances (added via `AppenderAttachable::addAppender()`); these are the real sinks and typically GUI-bound ones such as a list-model or text-edit appender.
- Depends on the running `QCoreApplication`/`QApplication` event loop to deliver posted events.
- Relies on `AppenderSkeleton::customEvent()` in the *target* appenders to receive the posted `LoggingEvent` and feed it back into `doAppend()`.

## 16. Usage Example

```cpp
#include "log4qt/mainthreadappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

// guiAppender updates a QListView model and must run on the GUI thread.
AppenderSharedPtr guiAppender = makeMyGuiAppender();
guiAppender->activateOptions();

auto *toMain = new MainThreadAppender;
toMain->setName(QStringLiteral("toMainThread"));
toMain->addAppender(guiAppender);     // delivered on the GUI thread
toMain->activateOptions();

Logger::rootLogger()->addAppender(AppenderSharedPtr(toMain));

// Logged from a worker thread; guiAppender still runs on the main thread.
QtConcurrent::run([] {
    Logger::rootLogger()->info(QStringLiteral("work finished"));
});
// Requires a running QApplication event loop on the main thread.
```
