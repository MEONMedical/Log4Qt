# DebugAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `DebugAppender` writes each formatted log line to the platform's native debug output. On Windows the message is sent to the attached debugger via `OutputDebugStringW` (visible in the Visual Studio output window, DebugView, or any attached debugger). On all other platforms it is written to `stderr`.

A developer uses `DebugAppender` to surface log output where a developer is already watching during debugging — the IDE/debugger console on Windows or the terminal `stderr` stream elsewhere — without configuring a file or a custom device. It is one of the "varia" convenience appenders.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class).
- **Implementation includes:** `abstractlayout.h`, `loggingevent.h`, `<windows.h>` (Windows only), `<iostream>`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `AbstractLayout` / `Layout` — formats the event into the string written to the debug output; obtained via `layout()`.
  - `LoggingEvent` — the event to format.

## 3. Class Hierarchy and Role

`DebugAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It overrides `requiresLayout()` and `append()`. Its role is a developer-facing sink that targets the platform debug channel.

## 4. Q_PROPERTY Declarations

None beyond those inherited from `AppenderSkeleton` (`isActive`, `isClosed`, `threshold`).

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### DebugAppender(QObject *parent = nullptr)

Default constructor. Chains to `AppenderSkeleton(parent)`; the appender starts inactive and without a layout.

#### DebugAppender(const LayoutSharedPtr &layout, QObject *parent = nullptr)

Convenience constructor. Chains to `AppenderSkeleton(true, layout, parent)`, so the appender is created already active with the supplied `layout`.

#### bool requiresLayout() const override

Returns `true` — a layout is mandatory because `append()` formats the event into a string before writing it.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Defined as a pure virtual in `AppenderSkeleton`; this override performs the actual write. Invoked from `doAppend()` under `mObjectGuard` (Phase 5) after entry conditions, threshold, and the filter chain have passed. It asserts that a layout is present, formats the event with `layout()->format(event)`, then:

- **Windows (`Q_OS_WIN`):** passes the message to `OutputDebugStringW` (converted to a wide string).
- **Other platforms:** writes the message to `std::cerr` followed by `std::endl` and an explicit flush.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr` and managed by the logger repository.
- It holds no external file or socket resources — the debug channel is a process-global facility provided by the OS, so there is nothing to open or close.

## 12. Thread Safety

All public functions are thread-safe (as documented on the class). The actual write in `append()` runs serialised under `AppenderSkeleton`'s `mObjectGuard` (Phase 5 of `doAppend()`), so concurrent log calls do not interleave their output. `OutputDebugStringW` and `std::cerr` are themselves process-wide, but the appender's own serialisation keeps each formatted line intact.

## 15. External Communication

Writes to a process-external channel: the OS debugger interface (`OutputDebugStringW`) on Windows, or the `stderr` stream on other platforms. There is no networking, file handle, or database involvement.

## 16. Usage Example

```cpp
#include "log4qt/varia/debugappender.h"
#include "log4qt/patternlayout.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(QStringLiteral("%d{HH:mm:ss} %p %c - %m%n")));
layout->activateOptions();

// Create already-active via the layout constructor.
auto *debugAppender = new DebugAppender(layout);
debugAppender->setName(QStringLiteral("debug"));

Logger::rootLogger()->addAppender(AppenderSharedPtr(debugAppender));
Logger::rootLogger()->debug(QStringLiteral("sent to the debugger / stderr"));
```
