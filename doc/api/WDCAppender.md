# WDCAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `WDCAppender` (Windows Debug Console appender) writes each formatted event to the Windows debugger output channel via the `OutputDebugString` API. The text is visible in the debugger's output pane (Visual Studio, WinDbg) or in tools such as Sysinternals DebugView.

A developer uses it during development on Windows to see log output in the attached debugger without opening a file or console. On non-Windows platforms the appender compiles but does nothing — `OutputDebugString` is replaced with an empty stub — so code that configures it remains portable.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class).
- **Implementation includes:** `abstractlayout.h`, `loggingevent.h`; on Windows `<windows.h>` (for `OutputDebugString`), otherwise a no-op static stub of the same name.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `Layout` / `AbstractLayout` — formats the event into text; obtained via the lock-free `layoutSnapshot()`.
  - `LoggingEvent` — the event to format.

## 3. Class Hierarchy and Role

`WDCAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It is constructed inactive (`AppenderSkeleton(false, ...)`). It overrides `requiresLayout()` and `append()`. Its role is a debugger-output sink, primarily for Windows.

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

#### WDCAppender(QObject *parent = nullptr)

Constructs an inactive appender with no layout.

#### WDCAppender(const LayoutSharedPtr &layout, QObject *parent = nullptr)

Constructs an inactive appender with the supplied layout.

#### bool requiresLayout() const override

Returns `true` — a layout is required to render the text passed to the debugger.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Invoked from `doAppend()` under `mObjectGuard`. Takes the layout via `layoutSnapshot()`; if none is set it returns. Otherwise it formats the event to a `QString`, converts it to a wide string (`toStdWString()`), and passes it to `OutputDebugString`. On Windows this routes to the debugger output channel; on other platforms the call is a compiled-out no-op.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr` and managed by the logger repository.
- It holds no external resources — there is nothing to open or close. `OutputDebugString` requires no handle.

## 12. Thread Safety

All public functions are thread-safe. `append()` runs serialised under `AppenderSkeleton`'s `mObjectGuard`. `OutputDebugString` is itself safe to call from any thread, so no additional synchronisation is needed; serialisation only keeps the formatting and the call atomic per event.

## 14. Inter-Class Interactions

- Uses a `Layout` (via `layoutSnapshot()`) to render each event to text.

## 15. External Communication

`WDCAppender` communicates with the Windows debugging subsystem.

- **Channel:** `OutputDebugString` (Windows API). **Outbound** only. The formatted message is emitted as a single wide-string write to the process's debug output channel, where an attached debugger or a monitor like DebugView receives it.
- **Direction / protocol:** one-way text output; no reply, no connection, no handle. Each event is one `OutputDebugString` call.
- **Error handling:** none required — `OutputDebugString` returns no error to handle, and on non-Windows builds the call is a no-op stub.
- **Threading implications:** the call is synchronous on the logging thread; `OutputDebugString` is thread-safe.

## 16. Usage Example

```cpp
#include "log4qt/wdcappender.h"
#include "log4qt/patternlayout.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(QStringLiteral("%d{HH:mm:ss.zzz} %p %c - %m%n")));
layout->activateOptions();

auto *wdc = new WDCAppender(layout);
wdc->setName(QStringLiteral("debugconsole"));
wdc->activateOptions();

Logger::rootLogger()->addAppender(AppenderSharedPtr(wdc));
Logger::rootLogger()->debug(QStringLiteral("visible in the debugger / DebugView on Windows"));
```
