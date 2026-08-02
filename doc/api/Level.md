# Level

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. Every logging operation carries a *severity*, and loggers and filters compare severities to decide whether a message passes. `Log4Qt::Level` is the lightweight value type that wraps a single severity value and gives it a total ordering, a string representation, a syslog mapping, and serialization support.

A developer reaches for `Level` whenever they need to name a severity (e.g. `Level::DEBUG_INT`), set a logger's threshold, compare severities, or convert level strings to and from configuration files. It is a small, copyable, `constexpr`-friendly wrapper around an 8-bit enumerator — not a `QObject`.

## 2. Project Structure and Dependencies

`level.h` is included by `logger.h`, `loggingevent.h`, `logstream.h`, and throughout the appender/layout/filter code. It includes:

- `log4qt.h` — the library's shared macros (including `LOG4QT_EXPORT`).
- `<QString>`, `<QStringView>` — for `toString` / `fromString`.
- `<QMetaType>` — for `Q_DECLARE_METATYPE`, registering `Log4Qt::Level` with Qt's meta-type system so it can travel through `QVariant`, signals, and properties.

The implementation (`level.cpp`) additionally uses `<QDataStream>` for serialization and routes its own warnings through a static logger named `Log4Qt::Level`.

Build requirement: `Qt6::Core`.

## 3. Class Hierarchy and Role

`Level` has no base class — it is a plain value type, not a `QObject` and not a `Q_GADGET`. It holds a single `Value` enumerator and exposes comparison, conversion, and serialization operations. The trailing `Q_DECLARE_METATYPE(Log4Qt::Level)` and `Q_DECLARE_TYPEINFO(..., Q_PRIMITIVE_TYPE)` make it efficiently storable in Qt containers and `QVariant`, and the `Log4Qt::Level` type is used directly as a `Q_PROPERTY` type on `Logger`.

## 4. Q_PROPERTY Declarations

`Level` declares no properties of its own (it is not a `QObject`). It is, however, *used* as the type of the `level` property on `Logger`.

## 5. Enumerations

`Level::Value` is the underlying enumeration, backed by `quint8`. The integer values are deliberately spaced so new levels can be inserted between existing ones, and the ordering encodes increasing severity. Comparison operators rely on this ordering (`NULL_INT < ALL_INT < TRACE_INT < … < OFF_INT`), and serialization writes the value as an unsigned 8-bit integer.

| Value | Integer | Description |
|-------|---------|-------------|
| `NULL_INT` | 0 | "No level specified." A logger with this level inherits its effective level from the nearest ancestor that has one. Lowest in the ordering. |
| `ALL_INT` | 32 | Enables logging of all events; lower than every real message level so nothing is filtered out by threshold. |
| `TRACE_INT` | 64 | Finest-grained informational events, finer than debug. |
| `DEBUG_INT` | 96 | Fine-grained informational events useful for debugging. |
| `INFO_INT` | 128 | Coarse-grained informational messages about normal progress. |
| `WARN_INT` | 150 | Potentially harmful situations. |
| `ERROR_INT` | 182 | Error events that still allow the application to continue. |
| `FATAL_INT` | 214 | Severe errors that will presumably lead the application to abort. |
| `OFF_INT` | 255 | Highest possible rank; used to turn logging off entirely. Highest in the ordering. |

Used throughout the logging API: as the `level()` of a `LoggingEvent`, the threshold compared in `Logger::isEnabledFor`, and the argument to every `Logger` logging method and `LogStream`.

## 6. Public Member Variables

None. The single value (`mValue`) is private.

## 7. Signals

None — `Level` is not a `QObject`.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### constexpr Level(Value value = NULL_INT) noexcept

Constructs a `Level` from a `Value` enumerator, defaulting to `NULL_INT`. This is a non-`explicit`, `constexpr`, `noexcept` converting constructor, so a `Value` such as `Level::DEBUG_INT` converts implicitly to a `Level` — which is why the API can be called as `logger->setLevel(Level::DEBUG_INT)`.

#### int syslogEquivalent() const

Maps the level to the corresponding syslog severity number: `NULL_INT`/`ALL_INT`/`TRACE_INT`/`DEBUG_INT` → 7 (debug), `INFO_INT` → 6, `WARN_INT` → 4, `ERROR_INT` → 3, `FATAL_INT`/`OFF_INT` → 0 (emergency). Used by appenders that forward to a syslog facility.

#### constexpr int toInt() const noexcept

Returns the underlying integer value (0–255). Useful for ordering logic and serialization.

#### constexpr auto operator<=>(const Level &other) const noexcept

Defaulted C++20 three-way comparison. Generates all relational operators (`<`, `<=`, `>`, `>=`, `==`, `!=`) from the integer ordering of the underlying value, so higher-severity levels compare greater.

#### QString toString() const

Returns the canonical name of the level (`"NULL"`, `"ALL"`, `"TRACE"`, `"DEBUG"`, `"INFO"`, `"WARN"`, `"ERROR"`, `"FATAL"`, `"OFF"`). The names are plain literals and are **not** localized: they are part of the log format — layouts write them via `%p`, log tooling parses them, and `fromString()` has to accept them again — so they must not vary with the locale. The returned `QString`s reference static literal data, so no allocation happens and `toString()` stays usable while the process is shutting down.

#### static Level fromString(QStringView level, bool *ok = nullptr)

Parses a level name into a `Level`, accepting the canonical tokens returned by `toString()`. On success, sets `*ok` to `true` (if provided) and returns the matching level. On an unrecognized string it logs a warning, sets `*ok` to `false`, and returns `Level::NULL_INT`.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`Level` is a value type with automatic storage and trivial copy/move semantics (`Q_PRIMITIVE_TYPE`). There is no heap allocation and no ownership concern — instances are copied freely, stored in containers and `QVariant`, and passed by value.

`toString()` needs no cache and holds no static state: it returns `QString`s over static literal data, which is also what makes it safe to call while the process is shutting down (Log4Qt logs during static destruction — the `atexit`-registered `LogManager::shutdown()` formats a shutdown event).

## 12. Thread Safety

All functions are thread-safe, as stated in the header. Instances are immutable after construction except via assignment, and neither `toString()` nor `fromString()` touches shared mutable state.

## 13. QML Exposure

Not directly registered for QML.

## 14. Inter-Class Interactions

- **`Logger`** exposes a `Level`-typed `level` property, compares levels in `isEnabledFor`/`effectiveLevel`, and emits `levelChanged(Log4Qt::Level)`.
- **`LoggingEvent`** stores and reports a `Level` as the event severity, and serializes it via the `Level` data-stream operators.
- **Configurators / filters / layouts** use `fromString` to read level names from configuration and `toString` to render them.

## 15. External Communication

#### QDataStream &operator<<(QDataStream &out, Log4Qt::Level level)

Free function (friend, available unless `QT_NO_DATASTREAM`). Serializes a level as a single `quint8`. Used when streaming `LoggingEvent`s across process or storage boundaries.

#### QDataStream &operator>>(QDataStream &in, Level &level)

Free function (friend). Reads a `quint8` back into a `Level`. The class is otherwise self-contained and performs no network or device I/O.

## 16. Usage Example

```cpp
#include "log4qt/level.h"

using Log4Qt::Level;

// Implicit construction from the enumerator.
Level threshold = Level::WARN_INT;

// Ordering: WARN is more severe than INFO.
bool passes = (Level(Level::ERROR_INT) >= threshold);   // true

// Human-readable name and syslog mapping.
QString name = threshold.toString();          // "WARN"
int sysSev   = threshold.syslogEquivalent();  // 4

// Parse from configuration text.
bool ok = false;
Level parsed = Level::fromString(u"DEBUG", &ok);  // ok == true, parsed == DEBUG_INT
```
