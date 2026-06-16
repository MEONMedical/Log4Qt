# JsonConfigurator

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. *Configurators* read configuration data and build the runtime logger/appender/layout graph held by the `LoggerRepository` via the `LogManager` singleton.

`JsonConfigurator` configures the logging framework from a JSON file written in a log4j2-style structured format. It does not implement configuration logic of its own: it parses the JSON document, flattens its nested object structure into the same flat key/value `Properties` model used by `PropertyConfigurator`, and delegates the actual configuration to `PropertyConfigurator`. This reuses all existing parsing, factory, and type-conversion logic without duplication.

A developer reaches for `JsonConfigurator` when configuration is more comfortably expressed as JSON than as flat `.properties` lines. During automatic startup the `LogManager` searches for `log4qt.json` *after* `log4qt.properties`, so `.properties` files take priority; the `Configuration` setting also accepts `.json` files.

## 2. Project Structure and Dependencies

Declared in `jsonconfigurator.h`, implemented in `jsonconfigurator.cpp`, part of the `log4qt` shared library.

Project-internal collaborators:

- `PropertyConfigurator` — receives the flattened `Properties` and performs the real configuration.
- `Properties` (from `helpers/properties.h`) — the flat key/value target of JSON flattening.
- `ConfiguratorHelper` (from `helpers/configuratorhelper.h`) — manages the file watch and stores the last error list.
- `Logger` / `LOG4QT_DECLARE_STATIC_LOGGER` — used to report file and parse errors.
- `LoggerRepository` (forward-declared) — the repository being configured; defaults to the `LogManager` repository when null.

Build requirement: `Qt6::Core`. The implementation uses `QFile`, `QJsonDocument`, `QJsonObject`, `QJsonValue`, and `QJsonParseError`.

## 3. Class Hierarchy and Role

`JsonConfigurator` has no base class. It is a non-instantiable utility class — its constructor is explicitly `= delete`d, so it exposes only static functions and serves purely as a namespace for the JSON entry points. The private static helpers `jsonToProperties()` and `flattenJsonObject()` implement parsing and recursive flattening.

## 9. Public Methods

#### static bool doConfigure(const QString &configFileName, LoggerRepository *loggerRepository = nullptr)

Reads `configFileName`, flattens it to a `Properties` object via `jsonToProperties()`, and (on success) hands it to a `PropertyConfigurator` to configure `loggerRepository`. If `loggerRepository` is null, the default `LogManager` repository is used. Returns `false` immediately if the file cannot be opened, read, or parsed, or if the JSON root is not an object; otherwise returns the result reported by `PropertyConfigurator::doConfigure`.

#### static bool configure(const QString &configFilename)

Convenience wrapper that calls `doConfigure(configFilename)` against the default repository. This is also the function registered as the reload callback by `configureAndWatch()`.

#### static bool configureAndWatch(const QString &configFilename)

Configures the default repository from `configFilename` and then registers the file with `ConfiguratorHelper::setConfigurationFile()`, passing `configure` as the reload callback so the helper's `QFileSystemWatcher` reloads the JSON on every change. Any existing watch is stopped first to avoid concurrent reconfiguration. An empty filename stops watching and returns `true` without configuring.

## 11. Ownership and Lifecycle

`JsonConfigurator` is never instantiated; all methods are static. The flattened `Properties` object is a local value. All loggers, appenders, and layouts are created and owned by the delegated `PropertyConfigurator` and the logging graph (reference-counted shared pointers owned by the repository and its loggers). The caller owns none of the created logging objects.

## 12. Thread Safety

All functions are thread-safe, as stated in the header. The delegated `PropertyConfigurator` and the `ConfiguratorHelper` are internally synchronized. `configureAndWatch` stops any prior watch before starting a new one.

## 14. Inter-Class Interactions

- Delegates all configuration to `PropertyConfigurator::doConfigure(const Properties &, LoggerRepository *)`.
- Writes the flattened result into a `Properties` object.
- Registers its `configure` callback and the watched file with `ConfiguratorHelper`; the `ConfiguratorHelper::configurationFileChanged(QString, bool)` signal is what external observers connect to. `JsonConfigurator` emits no signals itself.
- Reports file/parse errors through the static logger, which surface via the configuration error list captured by `PropertyConfigurator` and `ConfiguratorHelper::configureError()`.

## 15. External Communication

`JsonConfigurator` reads configuration **from disk** (inbound only). `jsonToProperties()` opens the file with `QFile` in read-only text mode and parses it with `QJsonDocument::fromJson()`. The root must be a JSON object.

Flattening rules (header-documented):

- Nested objects produce dot-separated keys: `{"a":{"b":"c"}}` becomes `a.b=c`.
- JSON booleans and numbers are stringified (`true` → `"true"`, integral doubles → integer text, `42` → `"42"`).
- JSON `null` produces an empty-string value.
- `${var}` substitution is performed later by `OptionConverter::findAndSubst()` inside `PropertyConfigurator`.

The resulting flat keys mirror the log4j2 schema (`appender.*`, `rootLogger.*`, `logger.*`). When used via `configureAndWatch`, the file is watched by a `QFileSystemWatcher` owned by `ConfiguratorHelper`, and changes trigger a reload using the static `configure` callback. No network, IPC, or external-process communication is involved.

## 16. Usage Example

```cpp
#include <log4qt/jsonconfigurator.h>
#include <log4qt/helpers/configuratorhelper.h>
#include <log4qt/logger.h>

using namespace Log4Qt;

int main(int argc, char *argv[])
{
    // One-shot configuration from a JSON file.
    if (!JsonConfigurator::configure(u"log4qt.json"_s))
    {
        const auto errors = ConfiguratorHelper::configureError();
        // inspect errors...
    }

    // Or: configure and live-reload on file changes.
    JsonConfigurator::configureAndWatch(u"log4qt.json"_s);

    Logger::logger("MyApp")->info("Configured from JSON");
    return 0;
}
```

A minimal `log4qt.json`:

```json
{
    "appender": {
        "console": {
            "type": "Console",
            "name": "console",
            "layout": {
                "type": "PatternLayout",
                "conversionPattern": "%-5p %c - %m%n"
            }
        }
    },
    "rootLogger": {
        "level": "ALL",
        "appenderRef": { "0": { "ref": "console" } }
    }
}
```
