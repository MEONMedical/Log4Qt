# PatternLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j. `PatternLayout` is the most flexible text layout: it formats each `LoggingEvent` according to a *conversion pattern* string, much like log4j's `PatternLayout` or `printf`. The pattern is a mix of literal text and `%`-prefixed conversion specifiers that are replaced with fields of the event (message, level, logger name, timestamp, thread, NDC/MDC, caller location, …).

Reach for `PatternLayout` whenever you want full control over the single-line (or multi-line) textual form of each log record. It also supports separate header and footer patterns evaluated at file-open and file-close time.

### Conversion specifiers

| Specifier | Meaning |
|-----------|---------|
| `%c{section_count}` | Logger name; optional `section_count` keeps that many trailing `::`-delimited sections |
| `%d{format_string}` | Date/time; optional brace-delimited format passed to `QDateTime::toString()` (also accepts `NONE`, `ISO8601`, `ABSOLUTE`, `DATE`, `RELATIVE`) |
| `%m` | Log message |
| `%p` | Level name |
| `%r` | Milliseconds since application start |
| `%t` | Thread name |
| `%x` | NDC (nested diagnostic context) |
| `%X` | MDC (mapped diagnostic context) |
| `%F` | Source file name (location-sensitive) |
| `%M` | Method/function name (location-sensitive) |
| `%L` | Line number (location-sensitive) |
| `%l` | Full caller location (location-sensitive) |
| `%P{key}` | Application property resolved via the pattern formatter's property source |
| `%n` | Platform end-of-line |

Field-width / justification modifiers (e.g. `%-5p`) are supported by the underlying `PatternFormatter`.

## 2. Project Structure and Dependencies

- **Instantiated by**: Configurators (`PropertyConfigurator`, `XmlConfigurator`, `JsonConfigurator`) via the factory; application code that assigns a layout to an appender.
- **Depends on**: `PatternFormatter` (`helpers/patternformatter.h`) does the actual parsing and per-event substitution; `PatternLayout` owns one for the body pattern and optionally one each for the header and footer patterns.
- **Qt modules**: Qt Core.
- **Internal types**: `AbstractStringLayout` (base), `PatternFormatter`, `LoggingEvent`, `HeaderFooterProvider` (consulted by the overridden `header()`/`footer()`).

## 3. Class Hierarchy and Role

`PatternLayout` → `AbstractStringLayout` → `AbstractLayout` → `QObject`. `QObject` provides the meta-object machinery; `AbstractLayout` the formatting contract and header/footer provider chain; `AbstractStringLayout` the charset and byte-encoding path. `PatternLayout` implements `format()` by delegating to its `PatternFormatter`, and overrides `header()`, `footer()`, and `requiresLocation()`. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `conversionPattern` | `QString` | `conversionPattern` | `setConversionPattern` | — | The pattern applied to each event body. Default `"%m%n"`. Setting it rebuilds the internal `PatternFormatter`. |
| `headerPattern` | `QString` | `headerPattern` | `setHeaderPattern` | — | Optional pattern formatted once at file-open time. Useful specifiers: `%d`, `%r`. Overrides the static `header` string when both are set. |
| `footerPattern` | `QString` | `footerPattern` | `setFooterPattern` | — | Optional pattern formatted once at file-close time. Symmetric to `headerPattern`. |

## 5. Enumerations

### ConversionPattern (Q_ENUM)

Used by the `PatternLayout(ConversionPattern, QObject *)` constructor and `setConversionPattern(ConversionPattern)` to select a predefined pattern.

| Value | Integer | Description |
|-------|---------|-------------|
| `DefaultPattern` | 0 | Sets the pattern to `"%m%n"` (message + newline). |
| `TtccPattern` | 1 | Sets the pattern to `"%r [%t] %p %c %x - %m%n"` (the classic log4j TTCC form). |

## 6. Public Member Variables

None (all members private).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### PatternLayout(QObject *parent = nullptr)
Constructs a layout with the default pattern `"%m%n"`.

#### PatternLayout(const QString &pattern, QObject *parent = nullptr)
Constructs a layout with the given conversion pattern.

#### PatternLayout(ConversionPattern conversionPattern, QObject *parent = nullptr)
Constructs a layout with a predefined pattern selected by the enum.

#### QString conversionPattern() const
Returns the current body pattern. Defined inline.

#### void setConversionPattern(const QString &pattern)
Sets the body pattern and rebuilds the internal `PatternFormatter`. Defined inline.

#### void setConversionPattern(ConversionPattern conversionPattern)
Sets the body pattern from the `ConversionPattern` enum (`DefaultPattern` or `TtccPattern`).

#### QString headerPattern() const
Returns the header pattern. Defined inline.

#### void setHeaderPattern(const QString &pattern)
Sets the header pattern. An empty string clears the header formatter; otherwise a dedicated `PatternFormatter` is built for it.

#### QString footerPattern() const
Returns the footer pattern. Defined inline.

#### void setFooterPattern(const QString &pattern)
Sets the footer pattern, with the same empty-clears semantics as `setHeaderPattern()`.

#### QString header() const [override]
Returns the effective header. Priority: per-layout provider (if non-empty), then the header pattern formatted against a default-constructed `LoggingEvent`, then the base-class resolution (static `header` string, then global provider).

#### QString footer() const [override]
Returns the effective footer using the same priority chain as `header()`.

#### QString format(const LoggingEvent &event) [override]
Formats `event` by delegating to the body `PatternFormatter`. Asserts in debug builds that the formatter is non-null.

#### bool requiresLocation() const [override]
Returns `true` if the current body pattern contains at least one location-sensitive specifier (`%F`, `%L`, `%M`, `%l`), as determined by the `PatternFormatter`.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. The overrides of inherited virtuals are `format()` (from `AbstractLayout`), `header()`, `footer()`, and `requiresLocation()`. Subclassing `PatternLayout` is uncommon; the design point is configuration via the pattern string.

## 11. Ownership and Lifecycle

A `QObject` taking an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. The three `PatternFormatter` instances (body, header, footer) are held by `std::unique_ptr` and destroyed with the layout (RAII). Copy/move disabled.

## 12. Thread Safety

Single-threaded for configuration: setting any pattern rebuilds a formatter and is not synchronised. `format()` reads the body formatter and is safe to call concurrently only insofar as the appender serialises access; treat one `PatternLayout` instance as used under its appender's lock. The header/footer global-provider lookups inherit the base class's read-locked behaviour.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Delegates all per-event work to `PatternFormatter`.
- Appenders call `format()` per event and may call `requiresLocation()` to decide on capturing source location.
- File-based appenders call `header()`/`footer()` at file open/close; both consult the `HeaderFooterProvider` chain inherited from `AbstractLayout`.
- Configurators set `conversionPattern`, `headerPattern`, `footerPattern` through the property system.

## 16. Usage Example

```cpp
#include "log4qt/patternlayout.h"
#include "log4qt/consoleappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(u"%d{ISO8601} [%t] %-5p %c - %m%n"_s));
layout->setHeaderPattern(u"=== log opened %d{ISO8601} ==="_s);
layout->activateOptions();

auto appender = AppenderSharedPtr(new ConsoleAppender(layout, ConsoleAppender::STDOUT_TARGET));
appender->activateOptions();

Logger::rootLogger()->addAppender(appender);
Logger::logger("MyApp")->info("Application started");
```
