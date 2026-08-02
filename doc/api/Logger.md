# Logger

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. It organises logging around a hierarchy of named *loggers*, each of which routes formatted *logging events* to one or more *appenders* (console, file, network, etc.) according to a configurable severity *level*.

`Log4Qt::Logger` is the central, user-facing class of the library. A logger is a named node in the logger hierarchy that an application uses to emit log messages at various severities (trace, debug, info, warn, error, fatal). When a message is logged, the logger decides — based on its effective level and the repository threshold — whether the message passes, and if so it builds a `LoggingEvent` and dispatches it to every attached appender, optionally walking up the parent chain (additivity).

Developers never construct a `Logger` directly. They retrieve a shared, never-destroyed instance by name via the static `Logger::logger(name)` factory (or through the `LOG4QT_DECLARE_STATIC_LOGGER` / `LOG4QT_DECLARE_QCLASS_LOGGER` convenience macros) and call one of its many logging methods.

## 2. Project Structure and Dependencies

`logger.h` is included by virtually every translation unit that wants to log. The header pulls in the building blocks the public API exposes by value or reference:

- `helpers/logerror.h` — `LogError`, an error-object overload accepted by every logging method.
- `helpers/classlogger.h` — `ClassLogger`, the lazy per-instance logger holder used by `LOG4QT_DECLARE_QCLASS_LOGGER`.
- `helpers/appenderattachable.h` — `AppenderAttachable`, the base providing the appender list and its read/write lock.
- `level.h` — `Level`, the severity wrapper used throughout the API.
- `logstream.h` — `LogStream`, the `<<`-style streaming helper returned by the no-argument logging methods.
- `loggingevent.h` — `LoggingEvent`, the immutable record dispatched to appenders.

The implementation (`logger.cpp`) additionally depends on internal types: `LogManager` (the global facade resolving loggers and the root logger), `LoggerRepository` / `Hierarchy` (the owning container), and `AppenderSkeleton`.

Build requirement: links against `Qt6::Core` (public) and `Qt6::Concurrent` (private). The class is exported through the `LOG4QT_EXPORT` macro for shared-library builds.

Project-internal types referenced in the public API: `Level`, `LogError`, `LogStream`, `LoggingEvent`, `MessageContext`, `LoggerRepository`, `Appender` (forward-declared), and `AppenderAttachable`.

## 3. Class Hierarchy and Role

`Logger` derives from two bases:

- `QObject` — gives `Logger` the meta-object system (signals, `Q_PROPERTY`, `Q_ENUM` of the level type via its property), and the `objectName` (set to the logger name). Note that loggers are constructed with a `nullptr` QObject parent; their lifetime is managed by the hierarchy, not by QObject parent-ownership.
- `AppenderAttachable` — provides the protected appender list (`mAppenders`) and its `QReadWriteLock` (`mAppenderGuard`), plus the public `addAppender`, `removeAppender`, `appenders`, `appender`, `isAttached`, and `removeAllAppenders` API for managing the appenders attached to this logger.

`Logger` is a node in the logger hierarchy: each instance holds a pointer to its parent logger (the root logger has none) and a pointer to the `LoggerRepository` that owns it. The class is friends with `Hierarchy`, which is the only type permitted to construct loggers.

The macro `LOG4QT_DECLARE_QCLASS_LOGGER` is applied inside the class, declaring a `logger()` accessor backed by a `ClassLogger` so that `Logger`'s own internal warnings (such as the unexpected-destruction message) are routed through a named logger.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `additivity` | `bool` | `additivity` | `setAdditivity` | `additivityChanged` | Controls whether events handled by this logger are also forwarded to the appenders of ancestor loggers. Default is `true`. When `false`, the parent chain is not walked. |
| `level` | `Log4Qt::Level` | `level` | `setLevel` | `levelChanged` | The level assigned directly to this logger. Default is `Level::NULL_INT`, meaning "no explicit level — inherit from the nearest ancestor with a level set". |
| `loggerRepository` | `LoggerRepository *` | `loggerRepository` | — | — | Read-only (`CONSTANT`). The repository that owns this logger. |
| `name` | `QString` | `name` | — | — | Read-only (`CONSTANT`). The dotted name of the logger (e.g. `"Log4Qt::Level"`). |
| `parentLogger` | `Logger *` | `parentLogger` | — | — | Read-only (`CONSTANT`). The parent logger in the hierarchy, or `nullptr` for the root logger. |

## 5. Enumerations

`Logger` declares no `Q_ENUM` or `Q_FLAG` of its own. The level values it accepts come from the `Log4Qt::Level` class (documented separately).

## 6. Public Member Variables

`Logger` exposes no public member variables. The appender list and its lock are protected members inherited from `AppenderAttachable`.

## 7. Signals

#### void additivityChanged(bool additivity)

Emitted by `setAdditivity` when the additivity flag actually changes value. A connected slot can react to runtime reconfiguration of event propagation.

#### void levelChanged(Log4Qt::Level level)

Emitted by `setLevel` when the assigned level actually changes value. Configurators and monitoring code connect to this to observe level changes.

## 8. Public Slots and Q_INVOKABLE Methods

`Logger` declares no `public slots` and no `Q_INVOKABLE` methods. (QML access to logging is provided by the separate `QmlLogger` class.)

## 9. Public Methods

### Property accessors

#### bool additivity() const

Returns the additivity flag. Thread-safe (atomic load).

#### Level level() const

Returns the level assigned directly to this logger (may be `Level::NULL_INT`). Thread-safe (atomic load).

#### LoggerRepository *loggerRepository() const

Returns the repository that owns this logger. The returned pointer is not owned by the caller.

#### QString name() const

Returns the logger's dotted name.

#### Logger *parentLogger() const

Returns the parent logger, or `nullptr` for the root logger. The returned pointer is not owned by the caller.

#### void setAdditivity(bool additivity)

Sets the additivity flag and emits `additivityChanged` if it changed. Thread-safe (atomic exchange).

### Level resolution and enablement

#### Level effectiveLevel() const

Returns the level actually in effect for this logger: starting at this logger, it walks up the parent chain until it finds a logger whose assigned level is not `Level::NULL_INT`. Asserts that the root logger has a non-null level. Acquires the appender read lock while walking the chain.

#### bool isEnabledFor(Level level) const

Returns `true` if a message at `level` would be logged. A logger is enabled for a level when the repository is not globally disabled for that level **and** the logger's effective level is less than or equal to `level` (i.e. `level` is at least as severe as the threshold). This is the single gate every logging method consults before building an event.

#### bool isTraceEnabled() const · bool isDebugEnabled() const · bool isInfoEnabled() const · bool isWarnEnabled() const · bool isErrorEnabled() const · bool isFatalEnabled() const

Convenience predicates, each equivalent to `isEnabledFor` for the corresponding level. Call these to guard expensive message construction when not using the variadic overloads.

### Logger retrieval (static factory)

#### static Logger *logger(const QString &name)

Returns the logger with the given dotted name, creating it (and any missing ancestors) on first request via `LogManager`. The returned pointer is owned by the repository and must never be deleted by the caller.

#### static Logger *logger(const char *name)

Convenience overload accepting a C string; forwards to `LogManager::logger`.

#### static Logger *rootLogger()

Returns the root of the logger hierarchy. The root always has a non-null level (defaults are enforced).

### Logging methods

For each severity there is a family of overloads with identical shape. The per-level methods are listed once below; substitute the level name for `<level>` ∈ {`trace`, `debug`, `info`, `warn`, `error`, `fatal`}.

#### LogStream <level>() const

Returns a `LogStream` bound to this logger and the level. Streaming values into it with `operator<<` accumulates a message that is logged when the temporary `LogStream` is destroyed. If the level is disabled, the stream discards everything cheaply.

#### void <level>(const QString &message) const

Logs `message` at the level if `isEnabledFor` passes, building a `LoggingEvent` and dispatching it.

#### void <level>(const LogError &logError) const

Logs the string form of `logError` (via `LogError::toString()`) at the level if enabled.

#### template<typename ...Ts> void <level>(const QString &message, Ts &&...ts) const

Argument-substituting overload. When the level is enabled, each `ts` is folded into `message` via successive `QString::arg(...)` calls (replacing `%1`, `%2`, …) before logging. Cheap when disabled because argument substitution is skipped entirely.

### Generic level logging

#### LogStream log(Level level) const

Returns a `LogStream` bound to this logger and an arbitrary `level`.

#### void log(Level level, const QString &message) const

Logs `message` at the given `level` if enabled.

#### void log(Level level, const LogError &logError) const

Logs `logError.toString()` at the given `level` if enabled.

#### void log(const LoggingEvent &logEvent) const

Logs a pre-built event if `isEnabledFor(logEvent.level())` passes. Useful for re-dispatching deserialized or relayed events.

#### template<typename ...Ts> void log(Level level, const QString &message, Ts &&...ts) const

Argument-substituting generic-level overload, analogous to the per-level variadic methods.

### Location-aware logging

#### void logWithLocation(Level level, const char *file, int line, const char *function, const QString &message) const

Logs `message` at `level`, attaching source-location information (file, line, function) to the resulting `LoggingEvent` via a `MessageContext`. Returns without logging unless `isEnabledFor(level)` passes, so the configured logger level is honoured even when the call is not guarded by the caller (the `l4q*` macros guard anyway, making the check a cheap redundant test on the hot path). The `file` and `function` pointers must point to strings with static storage duration (e.g. `__FILE__`, `Q_FUNC_INFO`).

#### template<typename ...Ts> void logWithLocation(Level level, const char *file, int line, const char *function, const QString &message, Ts &&...ts) const

Variadic location-aware overload. Checks `isEnabledFor(level)` before performing the `arg` substitution, so the argument formatting is skipped for disabled levels, then delegates to the non-variadic form (which checks again).

#### void logWithLocation(Level level, const QString &message, const std::source_location &loc = std::source_location::current()) const

Available when the compiler provides `<source_location>` (`__cpp_lib_source_location`). Captures the call site automatically and logs with that context, again only if `isEnabledFor(level)` passes.

### Dispatch

#### void callAppenders(const LoggingEvent &event) const

Dispatches `event` to every appender attached to this logger (under a read lock), then, if `additivity()` is true and a parent exists, recurses into the parent's `callAppenders`. This is the propagation mechanism; it is normally invoked internally by the logging methods but is public so events can be injected directly. Per the source note, use a `MainThreadAppender` if events produced on worker threads must be written from the main thread.

## 10. Protected Methods

#### Logger(LoggerRepository *loggerRepository, Level level, const QString &name, Logger *parent = nullptr)

Constructor. Protected because only `Hierarchy` (a friend) may create loggers. Asserts a non-null repository and sets the QObject `objectName` to the logger name.

#### ~Logger() override

Destructor. Loggers are not expected to be destroyed during normal operation; the destructor emits a warning through the class logger if it runs.

#### virtual void setLevel(Level level)

Sets the level assigned to this logger, emitting `levelChanged` if it changed (atomic exchange). If this is the root logger (no parent) and `level` is `Level::NULL_INT`, the call is rejected with a warning and `DEBUG_INT` is substituted, preserving the invariant that the root always has a usable level. Declared `virtual` so subclasses can extend level assignment.

#### void forcedLog(Level level, const QString &message) const

Builds a `LoggingEvent` from `level` and `message` and dispatches it via `callAppenders`, without consulting `isEnabledFor`. Used internally by the logging methods after they have confirmed the level is enabled.

#### void forcedLog(const LoggingEvent &logEvent) const

Dispatches a pre-built event via `callAppenders` unconditionally.

## 11. Ownership and Lifecycle

Loggers are owned by the logger hierarchy (`Hierarchy` / `LoggerRepository`), reached through `LogManager`. They are constructed lazily on first request and are **never destroyed** for the lifetime of the repository. Consequences:

- Callers must **never** `delete` a pointer returned by `Logger::logger(...)` or `rootLogger()`. Doing so is a bug; the destructor even logs a warning about unexpected destruction.
- Because a logger pointer is stable for the process lifetime, it is safe and idiomatic to cache it — exactly what `LOG4QT_DECLARE_STATIC_LOGGER` and `LOG4QT_DECLARE_QCLASS_LOGGER` do.
- The constructor passes `nullptr` as the QObject parent, so QObject parent-ownership does not apply; the hierarchy manages lifetime explicitly.
- Pointer members `loggerRepository()` and `parentLogger()` are borrowed references owned by the hierarchy, not by the logger.
- Attached appenders are held as `AppenderSharedPtr` (shared ownership) in the inherited `AppenderAttachable` list.

## 12. Thread Safety

All public functions of `Logger` are thread-safe, as stated in the header. Specifically:

- `level` and `additivity` are stored in `std::atomic` members; reads and writes use acquire/release semantics.
- The appender list is guarded by the inherited `QReadWriteLock` (`mAppenderGuard`); `callAppenders` and `effectiveLevel` take a read lock.
- The static `logger(...)` factory and `LOG4QT_DECLARE_*_LOGGER` accessors are thread-safe (the macros use a function-local static for safe lazy initialisation).

Appenders perform the actual output; to marshal log writes onto the main thread from worker threads, attach a `MainThreadAppender`.

## 13. QML Exposure

`Logger` itself is not registered for QML. QML logging is provided by the separate `QmlLogger` class (registered as `Logger` in module `org.log4qt`, built only when `BUILD_WITH_QML_LOGGING` is enabled).

## 14. Inter-Class Interactions

- **→ Appenders (via `AppenderAttachable`):** `callAppenders` invokes `doAppend` on each attached `Appender`; appenders are managed through the inherited attach/detach API.
- **→ `LoggingEvent`:** every logging path constructs a `LoggingEvent` (directly or in `forcedLog`) and hands it to appenders. Location-aware logging populates the event's `MessageContext`.
- **→ `LogStream`:** the no-argument level methods return a `LogStream` that calls back into `log(level, message)` when it is destroyed.
- **→ `LoggerRepository` / `LogManager` / `Hierarchy`:** `isEnabledFor` queries `LoggerRepository::isDisabled`; the static factory and root-logger accessors delegate to `LogManager`; instances are created only by `Hierarchy`.
- **Signals:** `levelChanged` and `additivityChanged` are emitted on reconfiguration; configurators and observers connect to them.

## 15. External Communication

None. `Logger` itself performs no network, IPC, or device I/O; any external transport is the responsibility of specific appenders.

## 16. Usage Example

```cpp
#include "log4qt/logger.h"
#include "log4qt/level.h"
#include "log4qt/basicconfigurator.h"

using Log4Qt::Logger;
using Log4Qt::Level;

class Counter
{
    LOG4QT_DECLARE_STATIC_LOGGER(logger, Counter)  // place in the .cpp instead, typically
public:
    explicit Counter(int preset)
    {
        if (preset < 0)
            logger()->warn(u"Invalid negative preset %1. Using 0."_s, preset);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Send everything to the console with a default layout.
    Log4Qt::BasicConfigurator::configure();

    Logger *log = Logger::logger(QStringLiteral("example.main"));
    log->setLevel(Level::DEBUG_INT);

    // Direct message logging with argument substitution.
    log->info(u"Starting up with %1 worker(s)"_s, 4);

    // Predicate-guarded expensive logging.
    if (log->isDebugEnabled())
        log->debug(u"Detailed state dump follows..."_s);

    // ostream-style streaming via LogStream.
    log->warn() << "Disk usage at " << 92 << "% on volume " << "C:";

    return app.exec();
}
```
