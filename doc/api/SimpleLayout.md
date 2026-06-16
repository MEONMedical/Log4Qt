# SimpleLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j; *layouts* convert a `LoggingEvent` into the text an appender writes. `SimpleLayout` is the minimal text layout: it emits the level and message of an event in the form `LEVEL - message` followed by a platform end-of-line, or just `message` when level output is disabled.

Reach for `SimpleLayout` for terse human-readable output (quick debugging, console logging) where timestamps, thread names, and logger names are unnecessary. For richer or configurable output use `PatternLayout`.

## 2. Project Structure and Dependencies

- **Instantiated by**: Configurators via the factory, and application code assigning a layout to an appender. `BasicConfigurator` and similar quick-start helpers commonly use a simple/pattern layout.
- **Qt modules**: Qt Core (uses `QStringBuilder` from `<QStringBuilder>` for efficient concatenation).
- **Internal types**: `AbstractStringLayout` (base), `LoggingEvent`, `Level` (via `event.level().toString()`).

## 3. Class Hierarchy and Role

`SimpleLayout` → `AbstractStringLayout` → `AbstractLayout` → `QObject`. It inherits the meta-object system, the layout contract, the header/footer provider chain, and the charset/byte-encoding path, and implements only `format()`. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `showLevel` | `bool` | `showLevel` | `setShowLevel` | — | When `true` (default), output is `LEVEL - message`; when `false`, only the message is emitted. |

## 5. Enumerations

None.

## 6. Public Member Variables

None (the `mShowLevel` member is private behind the property).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### SimpleLayout(QObject *parent = nullptr)
Constructs the layout with `showLevel` defaulting to `true`. Defined inline.

#### bool showLevel() const
Returns whether the level is included in the output. Defined inline.

#### void setShowLevel(bool show)
Enables or disables level output. Defined inline.

#### QString format(const LoggingEvent &event) [override]
Returns `event.level().toString() + " - " + event.message() + endOfLine()` when `showLevel` is `true`, otherwise `event.message() + endOfLine()`.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. The only override of an inherited virtual is `format()` (from `AbstractLayout`).

## 11. Ownership and Lifecycle

A `QObject` accepting an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. No owned heap resources. Copy/move disabled.

## 12. Thread Safety

Single-threaded by convention. `format()` reads only the `showLevel` flag and the immutable event, with no shared mutable state, so it is effectively reentrant; concurrent use is mediated by the owning appender's lock.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Appenders call `format()` per event.
- Reads `Level::toString()` and the message from the `LoggingEvent`.
- Inherits the header/footer provider chain from `AbstractLayout`.

## 16. Usage Example

```cpp
#include "log4qt/simplelayout.h"
#include "log4qt/consoleappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new SimpleLayout());
layout->activateOptions();

auto appender = AppenderSharedPtr(new ConsoleAppender(layout, ConsoleAppender::STDERR_TARGET));
appender->activateOptions();

Logger::rootLogger()->addAppender(appender);
Logger::logger("MyApp")->warn("Disk almost full");   // -> "WARN - Disk almost full"
```
