# TTCCLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j; *layouts* turn a `LoggingEvent` into text. `TTCCLayout` reproduces log4j's classic **TTCC** format — **T**ime, **T**hread, **C**ategory (logger), and nested **C**ontext — followed by the level and message. Each of the four elements can be toggled on or off, and the time element supports several named formats.

Reach for `TTCCLayout` when you want the familiar log4j TTCC line without hand-writing a pattern. Internally it builds an equivalent `PatternLayout`-style pattern from its toggles and delegates to a `PatternFormatter`.

## 2. Project Structure and Dependencies

- **Instantiated by**: Configurators via the factory and application code assigning a layout to an appender.
- **Depends on**: `PatternFormatter` (`helpers/patternformatter.h`) — `TTCCLayout` assembles a pattern string from its properties and lets the formatter do the per-event substitution.
- **Qt modules**: Qt Core (`QDateTime`).
- **Internal types**: `AbstractStringLayout` (base), `PatternFormatter`, `LoggingEvent`.

## 3. Class Hierarchy and Role

`TTCCLayout` → `AbstractStringLayout` → `AbstractLayout` → `QObject`. It inherits the meta-object system, layout contract, header/footer provider chain, and charset/byte path, and implements `format()` by delegating to an internally built `PatternFormatter`. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `categoryPrefixing` | `bool` | `categoryPrefixing` | `setCategoryPrefixing` | — | When `true` (default), the logger (category) name `%c` is included. Setting rebuilds the formatter. |
| `contextPrinting` | `bool` | `contextPrinting` | `setContextPrinting` | — | When `true` (default), the nested diagnostic context `%x` is included. Setting rebuilds the formatter. |
| `dateFormat` | `QString` | `dateFormat` | `setDateFormat` | — | Date format token placed inside `%d{…}`. Default `"RELATIVE"`. Accepts `NONE`, `ISO8601`, `ABSOLUTE`, `DATE`, `RELATIVE`, or a `QDateTime` format string. Setting rebuilds the formatter. |
| `threadPrinting` | `bool` | `threadPrinting` | `setThreadPrinting` | — | When `true` (default), the thread name `[%t]` is included. Setting rebuilds the formatter. |

## 5. Enumerations

### DateFormat (Q_ENUM)

Used by the `TTCCLayout(DateFormat, QObject *)` constructor and `setDateFormat(DateFormat)`; each value maps to a string token embedded in the `%d{…}` specifier.

| Value | Integer | Description |
|-------|---------|-------------|
| `None` | 0 | `"NONE"` — no date output. |
| `Iso8601` | 1 | `"ISO8601"` — formatted as `yyyy-MM-dd hh:mm:ss.zzz`. |
| `Absolute` | 2 | `"ABSOLUTE"` — formatted as `HH:mm:ss.zzz`. |
| `Date` | 3 | `"DATE"` — formatted as `MMM YYYY HH:mm:ss.zzz`. |
| `Relative` | 4 | `"RELATIVE"` (default) — milliseconds since program start. |

## 6. Public Member Variables

None (all data members private).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### TTCCLayout(QObject *parent = nullptr)
Constructs with all toggles `true` and `dateFormat` set to `RELATIVE`.

#### TTCCLayout(const QString &dateFormat, QObject *parent = nullptr)
Constructs with all toggles `true` and the given date format string.

#### TTCCLayout(DateFormat dateFormat, QObject *parent = nullptr)
Constructs with all toggles `true` and the date format selected by the enum.

#### bool categoryPrefixing() const
Returns whether the logger name is included. Defined inline.

#### bool contextPrinting() const
Returns whether the NDC is included. Defined inline.

#### QString dateFormat() const
Returns the current date format token. Defined inline.

#### bool threadPrinting() const
Returns whether the thread name is included. Defined inline.

#### void setCategoryPrefixing(bool categoryPrefixing)
Toggles logger-name output and rebuilds the formatter. Defined inline.

#### void setContextPrinting(bool contextPrinting)
Toggles NDC output and rebuilds the formatter. Defined inline.

#### void setDateFormat(const QString &dateFormat)
Sets the date format token (string) and rebuilds the formatter. Defined inline.

#### void setDateFormat(DateFormat dateFormat)
Sets the date format from the enum, mapping each value to its token string, then rebuilds the formatter.

#### void setThreadPrinting(bool threadPrinting)
Toggles thread-name output and rebuilds the formatter. Defined inline.

#### QString format(const LoggingEvent &event) [override]
Formats `event` by delegating to the internally built `PatternFormatter`. Asserts the formatter is non-null in debug builds. The assembled pattern is `%d{<dateFormat>}` followed by optional ` [%t]`, ` %-5p`, optional ` %c`, optional ` %x`, then ` - %m%n`.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. The only override of an inherited virtual is `format()` (from `AbstractLayout`).

## 11. Ownership and Lifecycle

A `QObject` accepting an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. The `PatternFormatter` is held by `std::unique_ptr` and destroyed with the layout (RAII); it is rebuilt whenever any property changes. Copy/move disabled.

## 12. Thread Safety

Single-threaded for configuration: each setter rebuilds the formatter without synchronisation. `format()` reads the formatter pointer; concurrent use is mediated by the owning appender's lock.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Delegates per-event formatting to `PatternFormatter`.
- Appenders call `format()` per event.
- Inherits the header/footer provider chain from `AbstractLayout`.
- Configurators set the four properties through the meta-object system.
