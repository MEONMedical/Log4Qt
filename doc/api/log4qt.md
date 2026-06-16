# log4qt.h

## A. Overview

`log4qt.h` is the top-level umbrella and version header for the Log4Qt library, a C++ port of the Apache Log4j package built on the Qt framework. A developer reaches for this header to:

- Obtain the compile-time **version** of Log4Qt and compare it against a required version using `LOG4QT_VERSION` / `LOG4QT_VERSION_CHECK`.
- Enforce the **minimum toolchain requirements** (Qt version and compiler) at compile time. Including the header fails the build early with a clear diagnostic when Qt or the compiler is too old.
- Reference the library-wide `ErrorCode` enumeration that names every error condition the package can report.

The header also carries the package's narrative documentation (object ownership, initialization procedure, environment variables, and differences from Log4j) as Doxygen `\page` blocks. These describe library behaviour as a whole rather than declaring any runtime API, so they are summarized here but not reproduced verbatim.

Note that this header declares no functions and no classes. Its runtime contribution is the single `ErrorCode` enumeration; everything else is preprocessor machinery and documentation.

## B. Namespaces

| Namespace | Groups |
|-----------|--------|
| `Log4Qt` | Encloses all parts of the package. In this header it scopes the `ErrorCode` enumeration and the documented version macros. |

## C. Types and Type Aliases

| Name | Kind | Description |
|------|------|-------------|
| `Log4Qt::ErrorCode` | `enum : int` | Library-wide error identifiers reported by appenders, layouts, configurators, levels, and loggers. Backed by `int`. |

### Enum `Log4Qt::ErrorCode` values

| Value | Meaning |
|-------|---------|
| `Ok` | No error (0). |
| `AppenderActivateMissingLayoutError` | An appender was activated without a required layout. |
| `AppenderActivateMissingWriterError` | A writer-based appender was activated without a writer. |
| `AppenderActivateMissingFileError` | A file appender was activated without a file name. |
| `AppenderClosedError` | An operation was attempted on a closed appender. |
| `AppenderInvalidPatternError` | A pattern supplied to an appender failed to parse. |
| `AppenderNoOpenFileError` | A file operation was attempted with no open file. |
| `AppenderNotActivatedError` | An appender was used before its options were activated. |
| `AppenderOpeningFileError` | Opening the target file failed. |
| `AppenderRenamingFileError` | Renaming a (rolled) file failed. |
| `AppenderRemoveFileError` | Removing a file failed. |
| `AppenderUseInvalidPatternError` | An invalid pattern was encountered during use. |
| `AppenderUseMissingLayoutError` | The appender was used while missing a layout. |
| `AppenderUseMissingWriterError` | The appender was used while missing a writer. |
| `AppenderWritingFileError` | Writing to the file failed. |
| `LevelInvalidLevelString` | A string could not be converted to a valid `Level`. |
| `LayoutExpectedDigitError` | A layout/pattern option expected a digit. |
| `LayoutOptionIsNotIntegerError` | A layout option that must be an integer was not. |
| `LayoutIntegerIsNotPositiveError` | A layout integer option was not positive. |
| `LoggerInvalidLevelForRoot` | An invalid level was set on the root logger. |
| `ConfiguratorOpeningFileError` | The configurator failed to open a configuration file. |
| `ConfiguratorReadingFileError` | The configurator failed to read a configuration file. |
| `ConfiguratorInvalidSubstitutionError` | A variable substitution in the configuration was invalid. |
| `ConfiguratorInvalidOptionError` | An option value in the configuration was invalid. |
| `ConfiguratorMissingAppenderError` | A referenced appender was not defined. |
| `ConfiguratorUnknownAppenderClassError` | An appender class name was not recognised. |
| `ConfiguratorMissingLayoutError` | A required layout was not defined. |
| `ConfiguratorUnknownLayoutClassError` | A layout class name was not recognised. |
| `ConfiguratorPropertyError` | A property could not be applied to an object. |
| `ConfiguratorUnknownTypeError` | An unknown object type was requested. |
| `AppenderMissingDatabaseOrTableError` | A database appender lacked a database or table setting. |
| `AppenderExecSqlQueryError` | Executing an SQL query in a database appender failed. |
| `AppenderInvalidDatabaseLayoutError` | A database appender was given a non-database layout. |
| `AppenderTelnetServerNotRunning` | The telnet appender's server is not running. |
| `AppenderAsncDispatcherNotRunning` | The async appender's dispatcher is not running. |
| `AppenderAsyncQueueFull` | The async appender's queue is full. |
| `AppenderAsyncShutdownTimeout` | The async appender timed out during shutdown. |

## D. Constants

The version numbers are supplied by the build system (CMake) via compile definitions and combined here into comparable forms.

| Name | Type / Value | Description |
|------|--------------|-------------|
| `LOG4QT_VERSION_MAJOR` | macro / int (from build) | Major version component, defined by CMake from `PROJECT_VERSION_MAJOR`. |
| `LOG4QT_VERSION_MINOR` | macro / int (from build) | Minor version component, defined by CMake from `PROJECT_VERSION_MINOR`. |
| `LOG4QT_VERSION_PATCH` | macro / int (from build) | Patch version component, defined by CMake from `PROJECT_VERSION_PATCH`. |
| `LOG4QT_VERSION_STR` | macro / string (from build) | Version as a string literal, e.g. `"2.1.0"`, defined by CMake from `PROJECT_VERSION`. |
| `LOG4QT_VERSION` | macro / int | The current version encoded as `(major << 16) | (minor << 8) | patch`. |

## E. Functions / Macros

#### LOG4QT_VERSION_CHECK(major, minor, patch)

Combines the three version components into a single comparable integer, computed as `((major << 16) | (minor << 8) | (patch))`. Use it to express a version threshold in a preprocessor comparison, for example guarding code on the Log4Qt version available at compile time. Preconditions: each argument fits in 8 bits. Thread-safety: not applicable (preprocessor only).

#### LOG4QT_VERSION

Expands to the encoded numeric version of the Log4Qt headers being compiled against, equal to `LOG4QT_VERSION_CHECK(LOG4QT_VERSION_MAJOR, LOG4QT_VERSION_MINOR, LOG4QT_VERSION_PATCH)`. Use it on the left-hand side of a comparison with `LOG4QT_VERSION_CHECK(...)` to conditionally compile code for a minimum library version. The complementary runtime accessors are `LogManager::version()` and `LogManager::versionString()`.

### Compile-time requirement guards

The header issues a hard `#error` (failing the build) when the toolchain does not meet the library's baseline. These are not callable; they fire automatically on inclusion.

| Guard condition | Diagnostic |
|-----------------|------------|
| `QT_VERSION < QT_VERSION_CHECK(6, 5, 0)` | Requires Qt 6.5.0 or higher. |
| MSVC with `_MSC_VER < 1900` | Requires MSVC 14 (VS2015) or higher for the C++11 features used. |
| GCC (non-Clang) older than 4.8 | Requires GCC 4.8 or higher. |
| Clang older than 3.3 | Requires Clang 3.3 or higher. |

## F. Dependencies

| Include | Provides |
|---------|----------|
| `log4qtshared.h` | The `LOG4QT_EXPORT` visibility macro used across the public API. |
| `<qglobal.h>` | `QT_VERSION`, `QT_VERSION_CHECK`, compiler-detection macros (`Q_CC_MSVC`, `Q_CC_GNU`, `Q_CC_CLANG`) used by the requirement guards. |

## G. Usage Example

```cpp
#include <log4qt/log4qt.h>

// Compile-time gate: only build this branch against Log4Qt 2.1.0 or newer.
#if (LOG4QT_VERSION >= LOG4QT_VERSION_CHECK(2, 1, 0))
    // Use an API introduced in 2.1.0.
#endif

void report(Log4Qt::ErrorCode code)
{
    if (code != Log4Qt::Ok) {
        // Handle the specific failure, e.g. AppenderOpeningFileError.
    }
}
```
