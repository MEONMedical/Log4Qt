# OptionConverter

## 1. Class Overview

`OptionConverter` is a static-utility class that converts raw configuration *option strings* — typically read from a properties file or `QSettings` — into strongly typed C++ values. It is the bridge between Log4Qt's textual configuration format and the typed setters on appenders, layouts, and the logging framework.

Each conversion follows a consistent contract: on success the typed value is returned and the optional `bool *ok` output is set to `true`; on failure a `LogError` (code `ConfiguratorInvalidOptionError`, or `ConfiguratorInvalidSubstitutionError` for substitution problems) is logged through the class's static logger, `*ok` is set to `false`, and a documented fallback value is returned. Overloads that take a `defaultValue` instead of an `ok` pointer return that default on failure and suppress the caller's need to inspect `ok`.

`OptionConverter` also provides variable substitution (`findAndSubst`) and a small Java-to-C++ class-name translation helper.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/helpers/optionconverter.h`
- Source: `src/log4qt/helpers/optionconverter.cpp`

Header dependencies: `log4qt/log4qtdefs.h`, `log4qt/log4qtshared.h` (export macro), `log4qt/level.h` (`Level` return type), `QString`, `QStringConverter`. The header forward-declares `Log4Qt::Properties`.

Source dependencies: `helpers/logerror.h`, `helpers/properties.h`, `logger.h` (for `LOG4QT_DECLARE_STATIC_LOGGER`), and `consoleappender.h` (for the `ConsoleAppender::Target` constants returned by `toTarget()`).

## 3. Class Hierarchy and Role

`OptionConverter` is a non-instantiable static-utility class. Its only constructor is **private** and undefined-for-use, so the class behaves like a namespace of static functions. It does not derive from `QObject` and exposes no instances. It is tagged `Q_DECLARE_TYPEINFO(Log4Qt::OptionConverter, Q_COMPLEX_TYPE)`.

## 4. Q_PROPERTY Table

None.

## 5. Enumerations

None defined by this class. `toTarget()` returns `int` values drawn from `ConsoleAppender::Target` (`StdOut`, `StdErr`), and `toEncoding()` returns `QStringConverter::Encoding`.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

All methods are `static`.

### Variable substitution

#### static QString findAndSubst(const Properties &properties, const QString &key)

Looks up `key` in `properties` and recursively expands `${name}` references found in the value. Behavior:

- If the key does not exist (the looked-up value is a *null* string), the null string is returned unchanged — callers use this to distinguish "missing" from "empty".
- Text outside `${...}` is copied verbatim; each `${name}` is replaced by recursively resolving `name` against the same `properties`.
- If a referenced sub-key is not found **and** its name starts with `LOG4QT_`, the value is taken from the environment via `qgetenv()`.
- A `${` with no matching closing `}` logs a `ConfiguratorInvalidSubstitutionError` and returns the result accumulated so far.

### Class-name translation

#### static QString classNameJavaToCpp(const QString &className)

Returns the Java class name `className` as a C++ class name by replacing every `.` with `::`.

### Typed conversions

#### static bool toBoolean(const QString &option, bool *ok = nullptr)

Converts `option` (trimmed, lower-cased) to a boolean. `"true"`, `"enabled"`, and `"1"` yield `true`; `"false"`, `"disabled"`, and `"0"` yield `false`. On any other input it logs an error, sets `*ok` to `false`, and returns `false`.

#### static bool toBoolean(const QString &option, bool defaultValue)

As above, but returns `defaultValue` when `option` is not a recognized boolean string (no `LogError` inspection needed by the caller).

#### static qint64 toFileSize(const QString &option, bool *ok = nullptr)

Converts `option` (trimmed, lower-cased) to a byte count. The string is a non-negative integer with an optional unit suffix: `kb` (×1024), `mb` (×1024²), or `gb` (×1024³); no suffix means bytes. Conversion fails — logging an error, setting `*ok` to `false`, and returning `0` — if the numeric part does not parse, if the value is negative, or if there is trailing text after the unit.

#### static int toInt(const QString &option, bool *ok = nullptr)

Converts `option` (trimmed) to an `int` using `QString::toInt()`. On failure it logs an error, sets `*ok` to `false`, and returns `0`.

#### static Level toLevel(const QString &option, bool *ok = nullptr)

Converts `option` (upper-cased, trimmed) to a `Level` via `Level::fromString()`. On failure it logs an error, sets `*ok` to `false`, and returns the (null) level produced by `Level::fromString()` for the failed input.

#### static Level toLevel(const QString &option, Log4Qt::Level defaultValue)

As above, but returns `defaultValue` when the string is not a valid level.

#### static int toTarget(const QString &option, bool *ok = nullptr)

Converts `option` (trimmed, lower-cased) to a `ConsoleAppender` target. `"system.out"` / `"stdout_target"` map to `ConsoleAppender::StdOut`; `"system.err"` / `"stderr_target"` map to `ConsoleAppender::StdErr`. On any other input it logs an error, sets `*ok` to `false`, and returns `ConsoleAppender::StdOut`.

#### static QStringConverter::Encoding toEncoding(const QString &option, bool *ok = nullptr)

Converts `option` to a `QStringConverter::Encoding` via `QStringConverter::encodingForName()`. On an unrecognized name it logs an error, sets `*ok` to `false`, and returns `QStringConverter::System`.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`OptionConverter` is never instantiated (private constructor) and owns no state or resources. `findAndSubst` takes the `Properties` argument by `const` reference and does not retain it.

## 12. Thread Safety

All methods are pure static functions that operate only on their arguments and local variables; they hold no shared mutable state, so concurrent calls are safe provided the `Properties` object passed to `findAndSubst` is not being mutated by another thread at the same time. Error reporting goes through the framework logger, which is itself thread-safe.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

`OptionConverter` is consumed primarily by the configurators (for example `PropertyConfigurator`), which read option strings out of a `Properties` map and call the `to*` helpers to populate typed appender/layout settings. `findAndSubst` resolves `${...}` references against that same `Properties` map (and the environment for `LOG4QT_`-prefixed names). Errors are reported as `LogError` objects via `Logger::error()`, joining the same error path documented for `LogError`.

## 15. External Communication

`findAndSubst` reads process environment variables via `qgetenv()` for unresolved keys beginning with `LOG4QT_`. The class performs no file or network I/O of its own.

## 16. Usage Example

```cpp
using namespace Log4Qt;

Properties props;
props.load(&configFile);

// Resolve a value, expanding ${...} references and LOG4QT_* env vars:
const QString rawThreshold = OptionConverter::findAndSubst(props, u"log4j.threshold"_s);

// Typed conversions with explicit error checks:
bool ok = false;
const Level level = OptionConverter::toLevel(rawThreshold, &ok);
if (!ok)
    /* a LogError was already logged */;

// Convenience overloads with defaults:
const bool immediateFlush = OptionConverter::toBoolean(props.property(u"flush"_s), true);
const qint64 maxSize       = OptionConverter::toFileSize(u"10MB"_s);   // -> 10 * 1024 * 1024
const int target           = OptionConverter::toTarget(u"System.err"_s); // ConsoleAppender::StdErr
```
