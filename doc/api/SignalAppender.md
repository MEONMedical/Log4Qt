# SignalAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `SignalAppender` is the simplest possible bridge into Qt's signal/slot system: instead of writing the formatted log line to a device, it **emits it as a Qt signal**. Any object can `connect` to that signal and react — appending to a status bar, a `QTextEdit`, an in-app log panel, a test harness, or any custom handler.

A developer uses it to surface log output inside the application's own UI or logic without coupling the logging library to a specific widget. The appender formats the event with its layout and emits the resulting string.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class) and `loggingevent.h`.
- **Implementation includes:** `abstractlayout.h`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `Layout` / `AbstractLayout` — formats the event into the string carried by the signal; obtained via `layout()`.
  - `LoggingEvent` — the event to format.

## 3. Class Hierarchy and Role

`SignalAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system (required to declare a signal), parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It overrides `requiresLayout()` and `preAppend()` (where it formats and emits) and provides an empty `append()` override; it adds one signal. Its role is a signal-emitting sink.

## 4. Q_PROPERTY Declarations

None beyond those inherited from `AppenderSkeleton` (`isActive`, `isClosed`, `threshold`).

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

#### void appended(const QString &message)

Emitted from `preAppend()` once per accepted log event, carrying the fully layout-formatted message string. This is the class's entire purpose, so it warrants careful handling:

- **Trigger:** fired whenever an event passes the appender's entry conditions, threshold, and filter chain and reaches the pre-append hook. The payload is exactly `layout->format(event)` using the layout snapshot taken by `doAppend()`.
- **Expected handler behaviour:** a connected slot displays, stores, or otherwise reacts to the message — for example appending it to a `QPlainTextEdit`, pushing it into a model, or asserting on it in a test.
- **Emitted outside the appender lock:** the emission happens in `preAppend()` (Phase 4b of `doAppend()`), which runs **without** `mObjectGuard` held. This is deliberate — emitting under the lock would let a `Qt::DirectConnection` slot execute synchronously while the lock is held, a deadlock hazard if the slot acquires another lock.
- **Thread affinity (important):** the signal is still emitted on **whatever thread called the logger** (`preAppend()` runs on the producer thread). If the connected receiver lives on the GUI thread and the log call may originate elsewhere, connect with `Qt::QueuedConnection` (or front the appender with a `MainThreadAppender`) so the slot runs on the receiver's thread. A default (auto) connection across threads already queues, but a direct connection would run the slot on the logging thread — unsafe for GUI updates.
- **Re-entrancy:** the slot must not log in a way that routes back to this same appender on the same thread, or the per-thread recursion guard in `doAppend()` will silently drop the nested event.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### explicit SignalAppender(QObject *parent = nullptr)

Constructs the appender, chaining to `AppenderSkeleton`.

#### bool requiresLayout() const override

Returns `true` — a layout is required to produce the message string carried by `appended()`.

## 10. Protected Virtual Methods

#### void preAppend(const Log4Qt::LoggingEvent &event, const LayoutSharedPtr &layout) override

The hook where all work happens. `doAppend()` calls it in Phase 4b — after entry conditions, threshold, and filters pass, and **outside** `mObjectGuard` — passing the layout snapshot. The override formats the event via `layout->format(event)` (guarding against a null layout) and emits `appended(message)`. Using the passed snapshot avoids both re-acquiring the lock through `layout()` and racing a concurrent `setLayout()`.

#### void append(const Log4Qt::LoggingEvent &event) override

Empty no-op. `AppenderSkeleton` declares `append()` pure-virtual so it must be implemented, but `SignalAppender` does all its work in `preAppend()` (outside the lock); there is nothing left to do under `mObjectGuard`.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr` and managed by the logger repository.
- It holds no external resources. Connections to `appended()` are owned by the standard Qt connection mechanism; when either the appender or the receiver is destroyed, the connection is removed automatically.

## 12. Thread Safety

All public functions are thread-safe. The format-and-emit work runs in `preAppend()`, which `doAppend()` invokes **outside** `mObjectGuard` — so a `Qt::DirectConnection` slot does not execute while the appender lock is held (avoiding a lock-ordering deadlock). The key consideration is the emission thread: `appended()` is emitted on the thread that performed the log call. Cross-thread receivers should use a queued connection (or interpose a `MainThreadAppender`) so their slot runs on the correct thread; this is the recommended pattern when the receiver updates the GUI.

## 14. Inter-Class Interactions

- Uses a `Layout` (via `layout()`) to render each event to text.
- Emits `appended(const QString &)`, which application code connects to in order to receive log lines (e.g. a UI log view, a status indicator, or a test observer).

## 16. Usage Example

```cpp
#include "log4qt/signalappender.h"
#include "log4qt/patternlayout.h"
#include "log4qt/logger.h"

#include <QPlainTextEdit>

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(QStringLiteral("%d{HH:mm:ss} %p %c - %m%n")));
layout->activateOptions();

auto *signalAppender = new SignalAppender;
signalAppender->setName(QStringLiteral("toUi"));
signalAppender->setLayout(layout);
signalAppender->activateOptions();

auto *logView = new QPlainTextEdit;

// Queued connection: the log call may come from a worker thread, but the
// QPlainTextEdit must be updated on the GUI thread.
QObject::connect(signalAppender, &SignalAppender::appended,
                 logView, [logView](const QString &message) {
                     logView->appendPlainText(message);
                 }, Qt::QueuedConnection);

Logger::rootLogger()->addAppender(AppenderSharedPtr(signalAppender));
Logger::rootLogger()->info(QStringLiteral("shown in the in-app log view"));
```
