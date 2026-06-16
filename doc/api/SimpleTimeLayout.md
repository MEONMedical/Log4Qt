# SimpleTimeLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j; *layouts* turn a `LoggingEvent` into the text an appender writes. `SimpleTimeLayout` produces a compact one-line record containing the timestamp, thread name, level, logger name, and message — a fixed, non-configurable format.

Reach for it when you want a slightly richer default line than `SimpleLayout` (with time, thread, and logger) but do not need the configurability of `PatternLayout`. The exact format is `dd.MM.yyyy hh:mm[thread] LEVEL logger - message` followed by a platform end-of-line.

## 2. Project Structure and Dependencies

- **Instantiated by**: Configurators via the factory and application code assigning a layout to an appender.
- **Qt modules**: Qt Core (`QStringBuilder` for concatenation).
- **Internal types**: `AbstractStringLayout` (base), `LoggingEvent`, `Level`, and `DateTime` (`helpers/datetime.h`) whose `formatMsecs()` renders the timestamp.

## 3. Class Hierarchy and Role

`SimpleTimeLayout` → `AbstractStringLayout` → `AbstractLayout` → `QObject`. It inherits the meta-object system, layout contract, header/footer provider chain, and charset/byte path, and implements only `format()`. It declares no properties of its own. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

None of its own (inherits `charset`, `header`, `footer`, `contentType` from base classes).

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### SimpleTimeLayout(QObject *parent = nullptr)
Constructs the layout. Defined inline; no configuration.

#### QString format(const LoggingEvent &event) [override]
Returns a single line composed of: the timestamp formatted as `dd.MM.yyyy hh:mm` via `DateTime::formatMsecs(event.timeStamp(), …)`, then `[thread] `, the level string, a space and the logger name, ` - `, the message, and a trailing end-of-line.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. The only override of an inherited virtual is `format()` (from `AbstractLayout`).

## 11. Ownership and Lifecycle

A `QObject` accepting an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. No owned heap resources. Copy/move disabled.

## 12. Thread Safety

Single-threaded by convention. `format()` reads only the immutable event and has no shared mutable state, so it is effectively reentrant; concurrent use is mediated by the owning appender's lock.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Appenders call `format()` per event.
- Uses `DateTime::formatMsecs()` to render the timestamp and reads thread name, level, logger name, and message from the `LoggingEvent`.
- Inherits the header/footer provider chain from `AbstractLayout`.

## 16. Usage Example

```cpp
#include "log4qt/simpletimelayout.h"
#include "log4qt/consoleappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new SimpleTimeLayout());
layout->activateOptions();

auto appender = AppenderSharedPtr(new ConsoleAppender(layout, ConsoleAppender::STDOUT_TARGET));
appender->activateOptions();

Logger::rootLogger()->addAppender(appender);
Logger::logger("MyApp")->info("Ready");
// -> "30.05.2026 14:05[main] INFO MyApp - Ready"
```
