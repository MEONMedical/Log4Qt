# PropertyConfigurator

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. *Configurators* read configuration data and build the runtime graph of loggers, appenders, and layouts held by the `LoggerRepository` (reached through the `LogManager` singleton).

`PropertyConfigurator` is the canonical configurator. It reads a log4j2-style `.properties` text file (or an equivalent `Properties` / `QSettings` source) and constructs the full logging configuration from it: global settings, appenders with their layouts, filters, triggering policies and rollover strategies, the root logger, and named loggers. It also transparently understands legacy log4j1-style (`log4j.*`) property files by translating them into the modern flat key format before parsing.

`PropertyConfigurator` is the engine underneath the other file-based configurators: both `JsonConfigurator` and `XmlConfigurator` flatten their input into a `Properties` object and delegate to it. A developer reaches for `PropertyConfigurator` to configure logging from a conventional `.properties` file, optionally watching that file for live reconfiguration.

## 2. Project Structure and Dependencies

Declared in `propertyconfigurator.h`, implemented in `propertyconfigurator.cpp`, part of the `log4qt` shared library.

Instantiated by `JsonConfigurator::doConfigure()` and `XmlConfigurator::doConfigure()`, and called directly by application code and by `LogManager` startup logic.

Project-internal collaborators:

- `Properties` (from `helpers/properties.h`) — an ordered key/value store with log4j-style loading from a `QIODevice` or `QSettings`, plus variable lookup. The flat property model is the configurator's native input.
- `Factory` (from `helpers/factory.h`) — class-name-keyed factory that creates `Appender`, `AbstractLayout`, `Filter`, `TriggeringPolicy`, `RolloverStrategy`, and `HeaderFooterProvider` instances, and applies named properties to a `QObject` via `setObjectProperty()`.
- `OptionConverter` (from `helpers/optionconverter.h`) — converts strings to `Level`/`bool` and performs `${var}` substitution via `findAndSubst()`.
- `ConfiguratorHelper` (from `helpers/configuratorhelper.h`) — stores the last error list and manages the `QFileSystemWatcher`-based file watch.
- `LogManager`, `LoggerRepository`, `Logger` — the target object graph being populated.
- `AppenderSkeleton`, `RollingFileAppender`, `AbstractLayout`, `ListAppender`, and the SPI types `TriggeringPolicy`, `RolloverStrategy`, `HeaderFooterProvider`.

Build requirement: `Qt6::Core`. The implementation uses `QFile`, `QSet`, `QHash`, and (via the header) forward-declares `QSettings`.

## 3. Class Hierarchy and Role

`PropertyConfigurator` has no base class. It has a public default constructor but copy and move are disabled via `Q_DISABLE_COPY_MOVE`. It exposes both **instance** methods (`doConfigure(...)`) and **static** convenience entry points (`configure(...)`, `configureAndWatch(...)`). The static methods create a temporary local instance and delegate to `doConfigure`. Instances carry per-run state (an appender registry and an error-capture appender), so each configuration pass uses a fresh object.

## 6. Public Member Variables

This class exposes no public member variables. Its state (`mpConfigureErrors`, `mAppenderRegistry`) is private.

## 9. Public Methods

### Instance methods

#### bool doConfigure(const Properties &properties, LoggerRepository *loggerRepository = nullptr)

Configures `loggerRepository` from an already-loaded `Properties` object. If `loggerRepository` is null, the default repository from `LogManager::loggerRepository()` is used. Brackets the work with error capture and returns `true` when no errors were recorded. This is the core entry point that the other overloads and the JSON/XML configurators feed into.

Processing order: legacy translation, then global settings, appenders, root logger, named loggers.

#### bool doConfigure(const QString &configFileName, LoggerRepository *loggerRepository = nullptr)

Opens and reads the named `.properties` file (read-only, text mode), loads it into a `Properties` object, and configures the repository. On a file-open or read failure it logs a `ConfiguratorOpeningFileError` / `ConfiguratorReadingFileError` and returns without configuring. Returns `true` when no errors were recorded.

#### bool doConfigure(const QSettings &settings, LoggerRepository *loggerRepository = nullptr)

Loads configuration keys from a `QSettings` object (via `Properties::load(const QSettings &)`) and configures the repository. Useful when configuration is stored in the platform settings store rather than a standalone file.

### Static entry points

#### static bool configure(const Properties &properties)

Convenience wrapper that creates a local `PropertyConfigurator` and calls `doConfigure(properties)` against the default repository.

#### static bool configure(const QString &configFilename)

Convenience wrapper that configures the default repository from the named `.properties` file. This is also the function registered as the reload callback by `configureAndWatch()`.

#### static bool configure(const QSettings &settings)

Convenience wrapper that configures the default repository from a `QSettings` object.

#### static bool configureAndWatch(const QString &configFilename)

Configures the default repository from `configFilename` and then registers the file with `ConfiguratorHelper::setConfigurationFile()`, passing `configure` as the reload callback. The helper watches the file via a `QFileSystemWatcher`; on every detected change the configuration is reloaded and `ConfiguratorHelper::configurationFileChanged()` is emitted.

Any existing watch is stopped first (by calling `setConfigurationFile()` with no arguments) to avoid concurrent reconfiguration. Passing an empty filename stops watching and returns `true` without configuring.

## 10. Protected Virtual Methods

None. `PropertyConfigurator` declares no virtual methods and is not designed for subclassing. All configuration steps below the public surface (`configureFromFile`, `configureFromProperties`, `configureAppenders`, `resolveAppenderReferences`, `configureRootLogger`, `configureLoggers`, `translateLegacyProperties`, `extractAliases`, `setProperties`, and the error-capture helpers) are private implementation detail.

### Appender-to-appender references

`configureFromProperties()` runs `resolveAppenderReferences()` between
`configureAppenders()` and the logger passes. Appenders are created *and
activated* one at a time as they are parsed, so an appender that refers to
another one by name cannot resolve it during its own activation — the target
may not exist yet. This step runs once the registry holds every appender of the
configuration and wires those references up, the same way `appenderRef` is
resolved for loggers.

Currently the only such reference is `AsyncAppender::errorRef`: for each
`AsyncAppender` with a non-empty `errorRef` the named appender is looked up in
`mAppenderRegistry` and installed via `setErrorAppender()`. A reference that
matches no appender is reported as a warning (not a configuration error, so
`doConfigure()` still succeeds). Because JSON and XML configuration is flattened
and delegated to this class, the behaviour is identical in all three formats.

## 11. Ownership and Lifecycle

`PropertyConfigurator` is a short-lived value object: instantiate, call a `doConfigure` overload, discard. The static `configure`/`configureAndWatch` methods manage that lifecycle internally.

Objects created during configuration are owned by reference-counted shared pointers and ultimately by the logging graph:

- Appenders are created via `Factory::createAppender()`, held in `AppenderSharedPtr`, registered in the per-run `mAppenderRegistry` keyed by name, and attached to loggers by `appenderRef`. The repository and loggers keep them alive; `mAppenderRegistry` is cleared at the end of each `configureFromProperties` pass.
- Layouts (`LayoutSharedPtr`) are owned by their appender. `HeaderFooterProvider` instances are moved into the layout (or set as the global provider on `AbstractLayout`).
- Filters (`FilterSharedPtr`) are added to the appender when it is an `AppenderSkeleton`.
- Triggering policies and rollover strategies are attached only to a `RollingFileAppender`; a warning is logged if specified for another appender type.
- The error-capture `ListAppender` (`mpConfigureErrors`) is created in `startCaptureErrors()`, attached to the internal log logger, and removed in `stopCaptureErrors()`.

When an `appenderRef`/`rootLogger.appenderRef` set is present, the target logger's existing appenders are removed first (`removeAllAppenders()`) so the file is authoritative.

An appender referenced by another appender (`AsyncAppender::errorRef`) is kept alive by the referring appender's own `AppenderSharedPtr`, so it survives the registry being cleared even if no logger references it.

## 12. Thread Safety

All public functions are thread-safe, as stated in the header. The `LogManager` loggers and `ConfiguratorHelper` they touch are internally synchronized. Each configuration run uses a distinct instance (or a local instance inside the static methods), so per-run state is not shared across threads. `configureAndWatch` deliberately stops any prior watch before starting a new one to prevent overlapping reconfiguration.

## 14. Inter-Class Interactions

- Uses `Factory` to instantiate appenders, layouts, filters, policies, strategies, and header/footer providers by class name, and to set their properties from string values.
- Uses `OptionConverter` for `${var}` substitution and for converting strings to `Level` and `bool`.
- Populates the `LoggerRepository` (root logger and named loggers) obtained from `LogManager`.
- Applies global settings to `LogManager`: `reset`, `status` (log4qt internal log level), `threshold`, `handleQtMessages`, `watchThisFile`, `filterRules`, `messagePattern`, and the global `HeaderFooterProvider`.
- Publishes the captured error list to `ConfiguratorHelper` and, via `configureAndWatch`, registers the reload callback there. The `ConfiguratorHelper::configurationFileChanged(QString, bool)` signal is what external observers connect to — `PropertyConfigurator` itself emits no signals.

## 15. External Communication

`PropertyConfigurator` reads configuration **from disk** (inbound only). The `doConfigure(const QString &)` and the `configure`/`configureAndWatch` filename overloads open a `.properties` text file via `QFile` in read-only mode. The expected format is log4j2-style flat keys (with automatic translation of legacy `log4j.*` files):

- Global: `reset`, `status`, `threshold`, `handleQtMessages`, `watchThisFile`, `filterRules`, `messagePattern`, `headerFooterProvider.type` and its properties.
- Appenders: `appender.<alias>.type`, `.name`, `.layout.type`, layout properties, `filter.<alias>.*`, `policy.<alias>.*`, `strategy.*`, plus arbitrary appender properties.
- Loggers: `rootLogger.level`, `rootLogger.appenderRef.<n>.ref`, `logger.<alias>.name|level|additivity`, and `logger.<alias>.appenderRef.<n>.ref`.

When configured through `configureAndWatch`, the file is watched by a `QFileSystemWatcher` (owned by `ConfiguratorHelper`); a change triggers a reload using the static `configure` callback. There is no network, IPC, or external-process communication.

## 16. Usage Example

```cpp
#include <log4qt/propertyconfigurator.h>
#include <log4qt/helpers/configuratorhelper.h>
#include <log4qt/logger.h>

using namespace Log4Qt;

int main(int argc, char *argv[])
{
    // One-shot configuration from a .properties file.
    bool ok = PropertyConfigurator::configure(u"log4qt.properties"_s);
    if (!ok)
    {
        const auto errors = ConfiguratorHelper::configureError();
        // inspect errors...
    }

    // Or: configure and live-reload on file changes.
    PropertyConfigurator::configureAndWatch(u"log4qt.properties"_s);
    QObject::connect(ConfiguratorHelper::instance(),
                     &ConfiguratorHelper::configurationFileChanged,
                     [](const QString &file, bool error) {
                         qInfo() << "Reloaded" << file << "error:" << error;
                     });

    Logger::logger("MyApp")->info("Configured");
    return 0;
}
```
