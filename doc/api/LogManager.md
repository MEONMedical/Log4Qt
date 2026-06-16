# LogManager

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. It organises logging around named `Logger` objects arranged in a dotted-name hierarchy, each of which forwards `LoggingEvent`s to attached `Appender`s formatted by `Layout`s.

`LogManager` is the global entry point and process-wide singleton of the library. It owns the default `LoggerRepository` (a `Hierarchy` instance) and exposes static convenience accessors for the special loggers used by the framework itself — `rootLogger()`, `logLogger()` (internal package logging) and `qtLogger()` (Qt message bridging). It also drives the library's automatic startup/configuration sequence, version reporting, optional bridging of Qt's own `qDebug()`/`qWarning()`/`qCritical()`/`qFatal()` messages into Log4Qt, and integration with `QLoggingCategory` filter rules and message patterns.

A developer rarely instantiates anything here — they call the static functions to obtain loggers, reconfigure logging, or enable Qt message handling.

## 2. Project Structure and Dependencies

`LogManager` is constructed lazily the first time any of its static functions is reached (commonly through the `Logger::logger()` / `LOG4QT_DECLARE_*_LOGGER` macros, which ultimately route through it). The library's configurators (`PropertyConfigurator`, `JsonConfigurator`, `XmlConfigurator`, `BasicConfigurator`) and the `Logger` lookup paths all funnel through the `LoggerRepository` it owns.

Internal collaborators (all from the Log4Qt library): `Hierarchy` (the concrete repository it creates), `Logger`, `Level`, `LoggingEvent`, `MessageContext`, `ConsoleAppender`, `TTCCLayout`, `DenyAllFilter`, `LevelRangeFilter`, `InitialisationHelper`, `OptionConverter`, `ConfiguratorHelper`, and the three configurator classes.

- **Qt module dependency:** Qt Core (`QString`, `QList`, `QMutex`/`QRecursiveMutex`, `QSettings`, `QCoreApplication`, `QFile`, `QFileInfo`, `QLoggingCategory`, `QVersionNumber`, `QtMessageHandler`).
- **Build requirement:** links `Qt6::Core` (`target_link_libraries(log4qt PUBLIC Qt${QT_VERSION_MAJOR}::Core)`); exported via the `LOG4QT_EXPORT` macro.

## 3. Class Hierarchy and Role

`LogManager` has no base class — it is a standalone, non-`QObject` singleton. Copy and move are deleted via `Q_DISABLE_COPY_MOVE(LogManager)`, and both the constructor and destructor are `private`, so instances can only be created through `instance()`. It is not part of the Qt meta-object system; it has no signals, slots, or properties.

## 4. Q_PROPERTY Declarations

None. `LogManager` is not a `QObject`.

## 5. Enumerations

None.

## 6. Public Member Variables

None. All state is private; access is through static functions.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

All public methods are `static`. Unless noted otherwise they implicitly construct the singleton on first use, and all are documented by the header as thread-safe.

#### static LogManager *instance()

Returns the singleton instance, constructing it on first call. Construction is guarded by a global mutex and publishes the pointer with release semantics before running the configuration steps (`doConfigureLogLogger()`, `welcome()`, `doStartup()`) so that re-entrant calls observe a fully built object. Registers `shutdown()` with `atexit()`. Most callers do not need this directly.

#### static LoggerRepository *loggerRepository()

Returns the default `LoggerRepository` (the owned `Hierarchy`). Used by configurators and by code that needs repository-level operations.

#### static Logger *logger(const QString &name)

Returns the `Logger` for the dotted/`::`-separated name, creating it (and any missing ancestors) in the repository if needed. The central logger-lookup entry point.

#### static Logger *rootLogger()

Returns the repository's root logger — the ultimate ancestor of every named logger.

#### static Logger *logLogger()

Returns the logger named `"Log4Qt"`, used for the library's own internal diagnostic messages. Equivalent to `logger("Log4Qt")`.

#### static Logger *qtLogger()

Returns the logger named `"Qt"`, used to emit messages captured from `qDebug()` and friends when Qt message handling is enabled. Equivalent to `logger("Qt")`.

#### static QList<Logger *> loggers()

Returns all loggers currently held by the repository.

#### static Level threshold()

Returns the repository-wide threshold level. Events below this level are suppressed for every logger.

#### static void setThreshold(Level level)

Sets the repository-wide threshold level.

#### static bool exists(const char *pName)

Returns true if a logger with the given name already exists in the repository, without creating it.

#### static bool handleQtMessages()

Returns whether bridging of `qDebug()`/`qWarning()`/`qCritical()`/`qFatal()` messages into Log4Qt is active. Reads an atomic flag with acquire semantics. Default is false.

#### static void setHandleQtMessages(bool handleQtMessages)

Enables or disables routing of Qt's own log messages into Log4Qt. When enabled, a Qt message handler is installed that logs through `qtLogger()`, mapping `QtDebugMsg`→DEBUG, `QtInfoMsg`→INFO, `QtWarningMsg`→WARN, `QtCriticalMsg`→ERROR, `QtFatalMsg`→FATAL, and other types→TRACE. Fatal messages reproduce the standard Qt abort behaviour. Disabling restores the previously installed handler.

#### static bool watchThisFile()

Returns whether the active configuration file is watched for changes (via `ConfiguratorHelper` / a file watcher) so that edits trigger reconfiguration. Default is false.

#### static void setWatchThisFile(bool watchThisFile)

Enables or disables watching of the configuration file picked up during startup.

#### static QString filterRules()

Returns the `QLoggingCategory` filter rules previously set via `setFilterRules()`.

#### static void setFilterRules(const QString &rules)

Stores the rules and applies them to `QLoggingCategory::setFilterRules()`, controlling the `qc*` categorised logging macros.

#### static QString messagePattern()

Returns the message pattern previously set via `setMessagePattern()`.

#### static void setMessagePattern(const QString &pattern)

Stores the pattern and applies it via `qSetMessagePattern()`, controlling formatting of categorised Qt logging output.

#### static void configureLogLogger()

Configures the internal `logLogger()` to its default behaviour: non-additive, with a stdout `ConsoleAppender` for events up to INFO and a stderr `ConsoleAppender` for WARN and above, both using a `TTCCLayout`. The level is taken from the `Debug` environment/application setting (falling back to DEBUG if the value is not a valid level, or ERROR if absent). Existing appenders are not removed.

#### static void startup()

Runs the package's default initialisation procedure: respects the `DefaultInitOverride` setting; otherwise searches, in order, the `Configuration` setting, application `QSettings` (`Log4Qt/Properties` group), executable-name-based config files, the executable directory, and the working directory for `log4qt.properties` / `log4qt.json` / `log4qt.xml`, configuring from the first match. If file watching is enabled, registers the chosen file with `ConfiguratorHelper`.

#### static void resetConfiguration()

Disables Qt message handling, resets every logger in the repository to defaults (removing all appenders), and re-applies the default internal logging via `configureLogLogger()`.

#### static void shutdown()

Shuts the repository down (resets its configuration). Registered as an `atexit` handler during `instance()` construction.

#### static const char *version()

Returns the Log4Qt runtime version string (`LOG4QT_VERSION_STR`), which may differ from the compile-time version the application was built against.

#### static QVersionNumber versionNumber()

Returns the runtime version as a `QVersionNumber` built from the major/minor/patch components.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`LogManager` is a process-wide singleton with private constructor/destructor. The single instance is created on demand by `instance()` and is never deleted explicitly during normal operation — the destructor logs a warning ("Unexpected destruction of LogManager") because reaching it indicates an unintended teardown.

The manager owns its `LoggerRepository` (`new Hierarchy()` in the constructor). That repository in turn owns every `Logger` it creates. Because the singleton is intentionally leaked, `shutdown()` is registered with `atexit()` to reset the repository at process exit rather than destroy the manager. Callers never manage the lifetime of loggers or the repository; they hold raw pointers returned by the static accessors.

## 12. Thread Safety

All public functions are thread-safe. The implementation uses several mechanisms: a global `QMutex` (`singleton_guard`) protects double-checked construction in `instance()`; the `mInstance` pointer is an `std::atomic` accessed with acquire/release ordering; `mHandleQtMessages` and `mWatchThisFile` are `std::atomic<bool>`; and a `mutable QRecursiveMutex` (`mObjectGuard`) guards the string state (`mFilterRules`, `mMessagePattern`) and the setter operations. The owned `Hierarchy` provides its own internal synchronisation for logger lookups.

## 13. QML Exposure

None. `LogManager` is not registered for QML.

## 14. Inter-Class Interactions

- Creates and owns a `Hierarchy` as the default `LoggerRepository`; delegates `logger()`, `rootLogger()`, `loggers()`, `threshold()`, `setThreshold()`, `exists()`, `resetConfiguration()` and `shutdown()` to it.
- Drives `PropertyConfigurator`, `JsonConfigurator` and `XmlConfigurator` during `startup()` based on discovered configuration sources, and registers the chosen file with `ConfiguratorHelper` when watching is enabled.
- Builds the internal logging pipeline from `ConsoleAppender`, `TTCCLayout`, `DenyAllFilter` and `LevelRangeFilter` in `configureLogLogger()`.
- Reads environment/application settings through `InitialisationHelper` and converts level strings via `OptionConverter`.
- Bridges Qt's logging system: installs a `QtMessageHandler`, emits captured messages as `LoggingEvent`s through `qtLogger()`, and configures `QLoggingCategory` filter rules and the global message pattern.
- Uses `QSettings` to discover application-embedded configuration during startup.

## 15. External Communication

None. `LogManager` does not itself open network connections, sockets, pipes, or talk to external processes; any output destinations are determined by the appenders configured on its loggers.

## 16. Usage Example

```cpp
#include "log4qt/logmanager.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Route qDebug()/qWarning()/etc. through Log4Qt.
    LogManager::setHandleQtMessages(true);

    // Obtain a named logger and emit an event.
    Logger *logger = LogManager::logger(QStringLiteral("MyApp.Module"));
    logger->info(QStringLiteral("Application started (Log4Qt %1)").arg(QString::fromLatin1(LogManager::version())));

    // Tighten the repository-wide threshold.
    LogManager::setThreshold(Level(Level::INFO_INT));

    return app.exec();
}
```
