# InitialisationHelper

## Class Overview

`InitialisationHelper` is the process-wide bootstrap singleton for Log4Qt. It performs the static initialisation tasks the library needs before logging can be configured:

- Captures the program's start time (used as the time origin for relative timestamps).
- Registers the library's custom types with the Qt meta-type system.
- Reads package-related settings from the system environment.
- Provides a unified `setting()` lookup that consults both environment variables and the application's `QSettings`.

The header also defines the `LOG4QT_IMPLEMENT_INSTANCE` macro that every Log4Qt singleton (including this class, `Factory`, and `ConfiguratorHelper`) uses to define its thread-safe `instance()` accessor.

## Project Structure and Dependencies

- Header: `src/log4qt/helpers/initialisationhelper.h`
- Source: `src/log4qt/helpers/initialisationhelper.cpp`
- Part of the `log4qt` library target (see `src/log4qt/CMakeLists.txt`, `log4qt_HEADERS_helpers`).

Dependencies:

- `QHash` / `QString` — storage for the environment-derived settings.
- `QMutex` (forward declared) — referenced by the singleton infrastructure.
- `QCoreApplication` (cpp) — gate for accessing `QSettings`.
- `QProcess` (cpp) — `systemEnvironment()` to read environment variables.
- `QSettings` (cpp) — application settings under the `Log4Qt` group.
- `helpers/datetime.h`, `helpers/logerror.h`, `loggingevent.h` (cpp) — the custom types registered with Qt's meta-type system.

## Class Hierarchy and Role

`InitialisationHelper` is a standalone (non-`QObject`) singleton. The constructor and destructor are private, copy/move are deleted via `Q_DISABLE_COPY_MOVE`, and the instance is created by `LOG4QT_IMPLEMENT_INSTANCE`. The whole public API is `static`. It is one of the foundational singletons that other helpers depend on for their `instance()` definition.

## Macros

#### LOG4QT_IMPLEMENT_INSTANCE(TYPE)

Defines `TYPE *TYPE::instance()` for a singleton class `TYPE`. The generated function returns a pointer to a function-local `static` instance that is heap-allocated on first use (`static auto *singelton(new TYPE);`). The C++ "magic static" guarantee makes the function thread-safe and lazily initialised. The instance is intentionally never deleted, giving it process lifetime. Used by `InitialisationHelper`, `Factory`, and `ConfiguratorHelper`.

## Public Methods

All public methods are `static`.

#### static QHash&lt;QString, QString&gt; environmentSettings()

Returns the settings discovered from the system environment during construction. The recognised environment variables and their setting keys are:

| Environment variable | Setting key |
|----------------------|-------------|
| `LOG4QT_DEBUG` | `Debug` |
| `LOG4QT_DEFAULTINITOVERRIDE` | `DefaultInitOverride` |
| `LOG4QT_CONFIGURATION` | `Configuration` |

Returns a copy of the internal hash.

#### static QString setting(const QString &key, const QString &defaultValue = QString())

Returns the value for setting `key`, or `defaultValue` if it is not defined. Resolution order:

1. If the key is present in `environmentSettings()`, its value is returned.
2. Otherwise, if a `QCoreApplication` instance exists, the value is read from `QSettings` in the `Log4Qt` group (trimmed).
3. Otherwise `defaultValue` is returned.

Recognised setting keys include `Debug` (controls the level of the internal log-log logger), `DefaultInitOverride` (skips the automatic initialisation procedure when set to anything other than `false`), `Configuration` (configuration file used at startup), and `ConfiguratorClass` (configurator class used at startup).

#### static qint64 startTime()

Returns the program start time as milliseconds since the Unix epoch (1970-01-01T00:00:00.000 UTC), captured when the singleton was constructed.

#### static InitialisationHelper *instance()

Returns the singleton instance, creating it on first use. Construction triggers type registration and environment-settings initialisation.

## Public Member Variables

This class exposes no public member variables; all state is accessed through the static methods above. (Internally it holds the captured start time, the environment-settings hash, and a static initialisation flag.)

## Ownership and Lifecycle

`InitialisationHelper` is a process singleton with process lifetime. Two creation paths exist:

- **Static initialisation**: the private static flag `mStaticInitialisation` is initialised by `staticInitialisation()`, which calls `instance()`. This forces construction during the program's static-init phase so type registration and start-time capture happen as early as possible.
- **Lazy**: any later `instance()` call also creates it if it does not yet exist.

The destructor asserts (`Q_ASSERT_X(false, ...)`) — it is never expected to run, reflecting that the singleton lives for the whole process and is deliberately leaked rather than torn down. Construction order: capture start time (member initialiser), then `doRegisterTypes()`, then `doInitialiseEnvironmentSettings()`.

## Thread Safety

All public functions are thread-safe. The `instance()` accessor is generated by `LOG4QT_IMPLEMENT_INSTANCE` and relies on the thread-safe initialisation of a function-local `static`. The environment settings are computed once at construction and thereafter read-only, so `environmentSettings()` and `setting()` return consistent data without additional locking. `setting()` constructs a fresh `QSettings` per call when falling through to application settings.

## External Communication

`InitialisationHelper` reads from two external sources, both inbound and read-only:

- The **process environment**, via `QProcess::systemEnvironment()`, parsed once at construction to populate `environmentSettings()`.
- The **application settings store**, via `QSettings` (the `Log4Qt` group), consulted on demand by `setting()` when a key is not provided by the environment and a `QCoreApplication` exists.

## Inter-Class Interactions

- Registers `Log4Qt::LogError`, `Log4Qt::Level`, and `Log4Qt::LoggingEvent` with the Qt meta-type system so they can be used in queued signal/slot connections and `QVariant`.
- `LogManager` consults `setting()` / `environmentSettings()` during its startup/initialisation procedure (e.g. to honour `DefaultInitOverride`, locate the `Configuration` file, and configure the internal logger via `Debug`).
- `startTime()` provides the origin used by relative-time pattern conversions and `DateTime` helpers.
- Its `LOG4QT_IMPLEMENT_INSTANCE` macro is reused by other singletons such as `Factory` and `ConfiguratorHelper`.

## Usage Example

```cpp
// Read a Log4Qt setting, honouring environment variables then QSettings.
const QString configFile =
    Log4Qt::InitialisationHelper::setting(QStringLiteral("Configuration"));

// Compute elapsed milliseconds since program start.
const qint64 elapsed =
    QDateTime::currentMSecsSinceEpoch() - Log4Qt::InitialisationHelper::startTime();

// Define a singleton using the provided macro (mysingleton.cpp):
// LOG4QT_IMPLEMENT_INSTANCE(MySingleton)
```
