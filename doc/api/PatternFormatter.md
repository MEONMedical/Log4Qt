# PatternFormatter

## 1. Class Overview

`PatternFormatter` is the engine that turns a *conversion-pattern string* into formatted log output for a `LoggingEvent`. It is the formatting core shared by `PatternLayout` and `TTCCLayout`.

On construction the pattern string is parsed once into an ordered chain of `PatternConverter` objects, each responsible for one piece of the output — a literal, a timestamp, the logger name, the level, the message, an MDC/NDC entry, a `QObject` property, or location information. At format time `format()` simply walks the chain, appending each converter's contribution to a single result string. Parsing once and reusing the converter chain keeps per-event formatting cheap.

The formatter also tracks whether the pattern uses any *location-sensitive* specifier (`%F`, `%L`, `%M`, `%l`) so that layouts can advertise `requiresLocation()` and appenders can avoid the cost of capturing source location when it is not needed.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/helpers/patternformatter.h`
- Source: `src/log4qt/helpers/patternformatter.cpp`

Header dependencies: `log4qt/log4qtdefs.h`, `log4qt/log4qtshared.h` (export macro), `QList`, `QString`, `<memory>`, `<vector>`. The header forward-declares `FormattingInfo`, `PatternConverter`, `LoggingEvent`, and `QObject`.

Source dependencies: `helpers/datetime.h` (`DateTime::formatMsecs`), `helpers/logerror.h`, `abstractlayout.h` (`AbstractLayout::endOfLine()`), `logger.h`, `loggingevent.h`, `logmanager.h`, plus `QString` and `QStringBuilder`. The converter hierarchy (`PatternConverter` and its subclasses) and the `FormattingInfo` helper are defined privately in the `.cpp` and are not part of the public API.

## 3. Class Hierarchy and Role

`PatternFormatter` is a plain (non-`QObject`) class. It declares a `virtual` destructor but is otherwise not intended as a polymorphic base, and it is non-copyable and non-movable (`Q_DISABLE_COPY_MOVE`). The non-movable guarantee is load-bearing: `%P{key}` converters store a pointer to the formatter's `mPropertySource` member, and that address must remain stable for the formatter's lifetime. The type is tagged `Q_DECLARE_TYPEINFO(Log4Qt::PatternFormatter, Q_COMPLEX_TYPE)`.

Internally the formatter owns a `std::vector<std::unique_ptr<PatternConverter>>`. `PatternConverter` is an abstract base whose concrete subclasses each handle a category of conversion character.

## 4. Q_PROPERTY Table

None.

## 5. Enumerations

None are public. (The private `BasicPatternConverter::Type` enum in the `.cpp` enumerates the basic conversion kinds: message, NDC, level, thread, filename, function name, line number, location, and category name.)

## 6. Public Member Variables

None. All data members are private.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

#### explicit PatternFormatter(const QString &pattern)

Creates a formatter and immediately parses `pattern` into the converter chain. Parsing is forgiving: an invalid conversion character is logged as a warning and emitted as a literal, and an unexpected end of pattern is logged and the trailing text emitted as a literal. Recoverable option errors are logged as `LogError`s (codes `LayoutExpectedDigitError`, `LayoutOptionIsNotIntegerError`, `LayoutIntegerIsNotPositiveError`).

#### virtual ~PatternFormatter()

Destroys the formatter and all owned `PatternConverter` objects.

#### QString format(const LoggingEvent &loggingEvent) const

Formats `loggingEvent` by running it through every converter in the chain, in order, and returns the assembled string. The result buffer is pre-reserved to reduce reallocations.

#### bool requiresLocation() const

Returns `true` if the pattern contains at least one location-sensitive conversion character (`%F`, `%L`, `%M`, or `%l`). Layouts delegate their own `requiresLocation()` to this so appenders can decide whether capturing source location is worthwhile.

#### void setPropertySource(const QObject *source)

Sets the `QObject` whose properties are read by `%P{key}` converters. At format time a `%P{key}` converter calls `source->property(key.toLatin1()).toString()`, resolving both static `Q_PROPERTY` members and dynamic properties set via `QObject::setProperty()`. Passing `nullptr` disables `%P{key}` (those converters then emit empty strings). Because converters hold a pointer *to* the formatter's property-source member rather than a copy, this may be called at any time — including after parsing — and takes effect on the next `format()` call.

### Supported conversion specifiers

The recognized conversion characters are `c d m p r t x X F M L l P`. The character `C` is recognized but ignored (consumed without producing output).

| Specifier | Output | Notes |
|-----------|--------|-------|
| `%c` | Logger (category) name | Optional integer precision `%c{n}` keeps the last *n* `::`-separated name segments. |
| `%d` | Event timestamp | Optional `%d{format}`. Default (no option) is `ISO8601`. Recognized keywords: `locale`/`locale:short`, `locale:long`, `locale:narrow` map to the matching `QLocale` date-time format; otherwise the option is used as a date-time format string. Formatting is done by `DateTime::formatMsecs`. |
| `%m` | Logging message | |
| `%p` | Level (priority) name | |
| `%r` | Relative time | Renders the timestamp with the special `RELATIVE` format. |
| `%t` | Thread name | |
| `%x` | NDC (nested diagnostic context) | |
| `%X` | MDC value | Requires a key: `%X{key}` looks the key up in the event's MDC. |
| `%P` | `QObject` property | Requires a key: `%P{key}`. Resolved against the object set by `setPropertySource()`. A bare `%P` with no `{key}` logs a warning. |
| `%F` | Source file name | Location-sensitive; sets `requiresLocation()`. |
| `%M` | Function name | Location-sensitive. |
| `%L` | Source line number | Location-sensitive. |
| `%l` | Full location | `file:line - function`. Location-sensitive. |
| `%n` | Line separator | Emitted as `AbstractLayout::endOfLine()`. Handled during parsing as a literal. |
| `%%` | Literal `%` | |
| `%C` | (ignored) | Consumed and produces no output. |

### Format modifiers

A specifier may carry an optional *format modifier* between `%` and the conversion character, of the form `[-][minWidth][.maxWidth]`:

- A leading `-` left-justifies within the minimum width (default is right-justification).
- `minWidth` pads the field with spaces up to that width if the value is shorter.
- `.maxWidth` truncates an over-long value; the extra characters are removed from the **beginning** of the value, not the end.

When neither a minimum nor a maximum is specified, the converter writes directly to the output with no padding/truncation pass.

## 10. Protected Virtual Methods

None on `PatternFormatter` itself. (Its private collaborator `PatternConverter`, defined in the `.cpp`, has a protected pure-virtual `convert(QString &, const LoggingEvent &)` that each concrete converter overrides; this is an implementation detail, not part of the public API.)

## 11. Ownership and Lifecycle

`PatternFormatter` owns its converter chain through `std::unique_ptr`, so all converters are destroyed with the formatter. It does **not** own the `QObject` passed to `setPropertySource()` — that object must outlive any `format()` call that resolves a `%P{key}` specifier; if it is destroyed, the caller should reset the source to `nullptr`. Because the type is `Q_DISABLE_COPY_MOVE`, a formatter has a fixed address for its lifetime, which is what makes the property-source pointer-to-member arrangement safe. `PatternLayout` typically owns its `PatternFormatter` via `std::unique_ptr` and rebuilds it whenever the pattern changes.

## 12. Thread Safety

After construction the converter chain is immutable, and `format()` is `const` and writes only to a local result string, so multiple threads may call `format()` concurrently on the same instance. The exceptions are the mutating operations: `setPropertySource()` writes `mPropertySource` without synchronization, and `%P{key}` converters read it (and call into the source `QObject`) at format time. Changing the property source, mutating the source object, or destroying the formatter concurrently with `format()` is not safe and requires external coordination.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

`PatternFormatter` is the formatting engine behind `PatternLayout` (and `TTCCLayout`): those layouts construct a `PatternFormatter` from their pattern, forward `format()` to it, and delegate `requiresLocation()` to it. The formatter reads almost every field of `LoggingEvent` — message, level, thread name, NDC, MDC, timestamp, logger/category name, and the source-location `context()`. For the logger-name converter it consults `LogManager` to special-case the Qt category logger. Timestamp formatting is delegated to `DateTime`. Internal parse/option errors are reported through `LogError` and the framework logger.

## 15. External Communication

None. `PatternFormatter` performs no file or network I/O. Through `%P{key}` it reads properties from a caller-supplied `QObject`, and through `%d` it reads the system locale, but it opens no external resources.

## 16. Usage Example

```cpp
using namespace Log4Qt;

// Build a formatter once from a conversion pattern:
PatternFormatter formatter(
    u"%d{ISO8601} [%t] %-5p %c{2} - %m%n"_s);

// Optionally expose a QObject's properties to %P{key}:
formatter.setPropertySource(myContextObject);

// Format each event (e.g. from inside an appender/layout):
const QString line = formatter.format(event);

// Let the appender skip location capture when the pattern doesn't need it:
if (formatter.requiresLocation())
    /* ensure source location is captured for the event */;
```
