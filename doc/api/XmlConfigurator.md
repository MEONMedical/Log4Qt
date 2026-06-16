# XmlConfigurator

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. *Configurators* read configuration data and build the runtime logger/appender/layout graph held by the `LoggerRepository` via the `LogManager` singleton.

`XmlConfigurator` configures the logging framework from an XML file written in a log4j2-style structured format. Like `JsonConfigurator`, it implements no configuration logic of its own: it parses the XML with a `QXmlStreamReader`, flattens nested elements and their attributes into the flat key/value `Properties` model used by `PropertyConfigurator`, and delegates the actual configuration to `PropertyConfigurator`. This reuses all existing parsing, factory, and type-conversion logic without duplication.

A developer reaches for `XmlConfigurator` when configuration is expressed as XML. During automatic startup the `LogManager` searches for `log4qt.xml` *after* `log4qt.json`, so `.properties` and `.json` files take priority; the `Configuration` setting also accepts `.xml` files.

## 2. Project Structure and Dependencies

Declared in `xmlconfigurator.h`, implemented in `xmlconfigurator.cpp`, part of the `log4qt` shared library.

Project-internal collaborators:

- `PropertyConfigurator` — receives the flattened `Properties` and performs the real configuration.
- `Properties` (from `helpers/properties.h`) — the flat key/value target of XML flattening.
- `ConfiguratorHelper` (from `helpers/configuratorhelper.h`) — manages the file watch and stores the last error list.
- `Logger` / `LOG4QT_DECLARE_STATIC_LOGGER` — used to report file and parse errors.
- `LoggerRepository` (forward-declared) — the repository being configured; defaults to the `LogManager` repository when null.

Build requirement: `Qt6::Core` (which provides `QXmlStreamReader`). The implementation uses `QFile` and `QXmlStreamReader`.

## 3. Class Hierarchy and Role

`XmlConfigurator` has no base class. It is a non-instantiable utility class — its constructor is explicitly `= delete`d, so it exposes only static functions and serves purely as a namespace for the XML entry points. The private static helpers `xmlToProperties()` and `flattenXmlElement()` implement parsing and recursive flattening.

## 9. Public Methods

#### static bool doConfigure(const QString &configFileName, LoggerRepository *loggerRepository = nullptr)

Reads `configFileName`, flattens it to a `Properties` object via `xmlToProperties()`, and (on success) hands it to a `PropertyConfigurator` to configure `loggerRepository`. If `loggerRepository` is null, the default `LogManager` repository is used. Returns `false` immediately if the file cannot be opened, contains no root element, or yields an XML stream error; otherwise returns the result reported by `PropertyConfigurator::doConfigure`.

#### static bool configure(const QString &configFilename)

Convenience wrapper that calls `doConfigure(configFilename)` against the default repository. This is also the function registered as the reload callback by `configureAndWatch()`.

#### static bool configureAndWatch(const QString &configFilename)

Configures the default repository from `configFilename` and then registers the file with `ConfiguratorHelper::setConfigurationFile()`, passing `configure` as the reload callback so the helper's `QFileSystemWatcher` reloads the XML on every change. Any existing watch is stopped first to avoid concurrent reconfiguration. An empty filename stops watching and returns `true` without configuring.

## 11. Ownership and Lifecycle

`XmlConfigurator` is never instantiated; all methods are static. The flattened `Properties` object is a local value, and the `QXmlStreamReader` reads directly from a stack `QFile`. All loggers, appenders, and layouts are created and owned by the delegated `PropertyConfigurator` and the logging graph (reference-counted shared pointers owned by the repository and its loggers). The caller owns none of the created logging objects.

## 12. Thread Safety

All functions are thread-safe, as stated in the header. The delegated `PropertyConfigurator` and the `ConfiguratorHelper` are internally synchronized. `configureAndWatch` stops any prior watch before starting a new one.

## 14. Inter-Class Interactions

- Delegates all configuration to `PropertyConfigurator::doConfigure(const Properties &, LoggerRepository *)`.
- Writes the flattened result into a `Properties` object.
- Registers its `configure` callback and the watched file with `ConfiguratorHelper`; the `ConfiguratorHelper::configurationFileChanged(QString, bool)` signal is what external observers connect to. `XmlConfigurator` emits no signals itself.
- Reports file/parse errors through the static logger, which surface via the configuration error list captured by `PropertyConfigurator` and `ConfiguratorHelper::configureError()`.

## 15. External Communication

`XmlConfigurator` reads configuration **from disk** (inbound only). `xmlToProperties()` opens the file with `QFile` in read-only text mode and parses it incrementally with a `QXmlStreamReader`. It advances to the document (root) element, then flattens the root's children at the top level.

Flattening rules:

- Nested elements produce dot-separated keys, with the element name prefixing its children: `<Console><PatternLayout/></Console>` under `appender` yields keys like `appender.console.layout.type`.
- XML attributes are flattened as child properties of their element: `<Logger name="MyApp" level="ERROR"/>` yields `logger.MyApp.name=MyApp` and `logger.MyApp.level=ERROR`.
- Non-whitespace text content of a leaf element (no child elements) is stored as the element's value.
- `${var}` substitution in attribute values and text content is performed later by `OptionConverter::findAndSubst()` inside `PropertyConfigurator`.

The resulting flat keys mirror the log4j2 schema (`appender.*`, `rootLogger.*`, `logger.*`). When used via `configureAndWatch`, the file is watched by a `QFileSystemWatcher` owned by `ConfiguratorHelper`, and changes trigger a reload using the static `configure` callback. No network, IPC, or external-process communication is involved.

## 16. Usage Example

```cpp
#include <log4qt/xmlconfigurator.h>
#include <log4qt/helpers/configuratorhelper.h>
#include <log4qt/logger.h>

using namespace Log4Qt;

int main(int argc, char *argv[])
{
    // One-shot configuration from an XML file.
    if (!XmlConfigurator::configure(u"log4qt.xml"_s))
    {
        const auto errors = ConfiguratorHelper::configureError();
        // inspect errors...
    }

    // Or: configure and live-reload on file changes.
    XmlConfigurator::configureAndWatch(u"log4qt.xml"_s);

    Logger::logger("MyApp")->info("Configured from XML");
    return 0;
}
```

A minimal `log4qt.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Configuration>
    <Appenders>
        <Console name="console">
            <PatternLayout conversionPattern="%-5p %c - %m%n" />
        </Console>
    </Appenders>
    <Loggers>
        <Root level="ALL">
            <AppenderRef ref="console" />
        </Root>
    </Loggers>
</Configuration>
```
