# main.cpp — PropertyConfigurator Example Entry Point

## A. Overview

This example demonstrates **file-based configuration** of Log4Qt using `PropertyConfigurator`. The
entire logging pipeline — appenders, layouts, levels, per-logger settings, and the global
header/footer provider — is described in a log4j-style `.properties` file loaded at runtime. The
companion *basic* example builds the same kind of pipeline in code; here, `main()` mostly registers
application-defined extension classes and then hands control to the configurator.

`main()`'s role is to:

1. Reset the `LogManager` so its automatic startup configuration does not pre-load the properties
   file before the custom factories are in place.
2. Create the `QCoreApplication` and the demo logger objects.
3. Register two custom `HeaderFooterProvider` subclasses with the `Factory` so the configurator can
   instantiate them **by class name** and set their `serialNumber` `Q_PROPERTY`.
4. Load the configuration via `PropertyConfigurator::configureAndWatch(file)`.
5. Emit framing START/STOP banner messages and run the event loop.
6. Tear down logging and unregister the factories on exit.

## B. Qt Application Setup

A `QCoreApplication` is created on the stack (console application). No special attributes are set.
`QCoreApplication::applicationFilePath()` is used to derive the configuration file name
(`<exe-path>.log4qt.properties`).

A noteworthy ordering detail: `Log4Qt::LogManager::resetConfiguration()` is called **before** any
application configuration. Without it, `LogManager`'s `doStartup` would auto-load the properties
file before the custom provider factories are registered, producing a spurious header/footer pair at
startup.

## C. Command-Line Handling

`argc`/`argv` are forwarded to `QCoreApplication`; the example parses no arguments. The config file
path is derived from the executable path, not from the command line.

## D. Top-Level Object Creation

| Object | Type | Lifetime | Demonstrates |
| --- | --- | --- | --- |
| `object` | `LoggerObject` | Parented to `application` | Per-instance class logger via `LOG4QT_DECLARE_QCLASS_LOGGER`; stream-style `debug`/`error` on a 10 ms timer. Emits `exit(0)` after 10 ticks. |
| `object1` | `LoggerObjectPrio` | Parented to `application` | A second class logger; configured in the properties file with an explicit `ERROR` level, `additivity=false`, and its own appender refs. Marked `Q_UNUSED`. |
| `object2` | `LoggerStatic` | `QScopedPointer`, scoped to the event-loop block | Static class logger via `LOG4QT_DECLARE_STATIC_LOGGER`; logs from constructor/destructor. |

`LoggerObject::exit(int)` is connected to `QCoreApplication::exit` to stop the loop.

## E. Wiring and Connections

Logging is configured by **loading a file**. After registering the custom provider factories,
`setupRootLogger()` builds the path `applicationFilePath() + ".log4qt.properties"` and, if it exists,
calls `Log4Qt::PropertyConfigurator::configureAndWatch(configFile)`. `configureAndWatch` also watches
the file for changes and re-applies it at runtime.

**Factory registration** is the key extension mechanism shown here. Each lambda captures the serial
number (read from hardware in a real application) so every provider instance the configurator
creates already has its `serialNumber` set when `activateOptions()` opens the file and writes the
header:

```cpp
Log4Qt::Factory::registerHeaderFooterProvider(
    "SerialNumberHeaderProvider",
    [serialNumber]() -> Log4Qt::HeaderFooterProvider * {
        auto *p = new SerialNumberHeaderProvider;
        p->setSerialNumber(serialNumber);
        return p;
    });
```

### The configuration file

`propertyconfigurator.exe.log4qt.properties` (loaded next to the executable). Representative
excerpt:

```properties
reset=true
status=WARN
handleQtMessages=true
watchThisFile=false

# Global header provider — application class registered via the Factory in main.cpp
headerFooterProvider.type=SerialNumberHeaderProvider

appender.console.type=Console
appender.console.target=STDOUT_TARGET
appender.console.layout.type=TTCCLayout

appender.daily.type=DailyFile
appender.daily.file=${logpath}/propertyconfigurator.log
appender.daily.datePattern=_yyyy_MM_dd
appender.daily.keepDays=90

appender.json.type=DailyFile
appender.json.file=${logpath}/propertyconfigurator.json
appender.json.layout.type=JsonLayout
appender.json.layout.headerFooterProvider.type=JsonSerialNumberHeaderProvider
appender.json.layout.headerFooterProvider.headerPattern={"event":"start","serialNumber":"%P{serialNumber}","time":"%d{yyyy-MM-ddTHH:mm:ss}"}

rootLogger.level=ALL
rootLogger.appenderRef.console.ref=console
rootLogger.appenderRef.daily.ref=daily
rootLogger.appenderRef.json.ref=json

logger.LoggerObjectPrio.name=LoggerObjectPrio
logger.LoggerObjectPrio.level=ERROR
logger.LoggerObjectPrio.additivity=false
logger.LoggerObjectPrio.appenderRef.daily.ref=daily
logger.LoggerObjectPrio.appenderRef.console.ref=console
```

What the file configures:

| Setting | Meaning |
| --- | --- |
| `reset` / `status` / `threshold` | Reset prior config, set internal status to `WARN`, no global threshold. |
| `handleQtMessages=true` | Route Qt's `qDebug`/`qWarning`/... into Log4Qt. |
| `headerFooterProvider.type` | Global header provider, resolved by class name through the registered factory. |
| `appender.console` | `ConsoleAppender` on `STDOUT`, `TTCCLayout`, threshold `ALL`. |
| `appender.daily` | `DailyFile` appender rolling daily, keeping 90 days. |
| `appender.json` | `DailyFile` appender with `JsonLayout`; its header/footer come from the `JsonSerialNumberHeaderProvider`, whose patterns read `%P{serialNumber}` and the open/close timestamp. |
| `rootLogger` | Level `ALL`, attached to the three appenders. |
| `logger.LoggerObjectPrio` | Named logger with level `ERROR`, `additivity=false`, and its own console + daily appender refs. |

On shutdown, `shutDownRootLogger()` logs a closing message, removes appenders, shuts the repository
down, clears the global provider, and **unregisters both factories**.

## F. Event Loop

Yes. `QCoreApplication::exec()` runs inside a scoped block owning the `LoggerStatic` instance. The
demo objects' timers drive logging until `LoggerObject` emits `exit(0)`.

## G. Dependencies

**Qt modules**

| Header | Provides |
| --- | --- |
| `QCoreApplication` | Console event loop, application paths. |
| `QScopedPointer` | Scoped ownership of `LoggerStatic`. |
| `QString`, `QStringBuilder` | String construction. |
| `QFile` | `QFile::exists` check on the config path. |

**Log4Qt headers**

| Header | Provides |
| --- | --- |
| `log4qt/logger.h` | `Logger`, logging macros. |
| `log4qt/propertyconfigurator.h` | `PropertyConfigurator::configureAndWatch`. |
| `log4qt/helpers/factory.h` | `Factory::registerHeaderFooterProvider` / `unregisterHeaderFooterProvider`. |
| `log4qt/logmanager.h` | `LogManager::resetConfiguration`. |
| `log4qt/loggerrepository.h` | Repository `shutdown()` on teardown. |
| `log4qt/abstractlayout.h` | `AbstractLayout::setGlobalHeaderFooterProvider` (cleared on teardown). |
| `log4qt/spi/headerfooterprovider.h` | `HeaderFooterProvider` / `PatternHeaderFooterProvider` base classes. |

## Custom Helper Classes (defined in main.cpp)

#### class SerialNumberHeaderProvider : public Log4Qt::HeaderFooterProvider

A `Q_OBJECT` with a `serialNumber` `Q_PROPERTY` (`READ serialNumber` / `WRITE setSerialNumber`).
Overrides `header()` to emit `Device S/N: <serial>`. Because it is a `Q_OBJECT` with a settable
property, the `PropertyConfigurator` can configure it by name; the captured serial is applied by the
factory lambda.

#### class JsonSerialNumberHeaderProvider : public Log4Qt::PatternHeaderFooterProvider

A `Q_OBJECT` with a `serialNumber` `Q_PROPERTY`. It inherits `PatternHeaderFooterProvider`, so its
header/footer text is produced by the `PatternFormatter` from the `headerPattern`/`footerPattern`
set in the properties file. `%P{serialNumber}` reads the property at format time and `%d{...}`
captures the file-open/close timestamp.
