# PropertyConfigurator Example — Documentation Index

This example shows how to configure Log4Qt from an **external log4j-style `.properties` file** using
`PropertyConfigurator`. Instead of building appenders and layouts in code, `main()` registers two
application-defined `HeaderFooterProvider` extension classes with the `Factory`, then calls
`PropertyConfigurator::configureAndWatch(...)` to load the pipeline from
`propertyconfigurator.exe.log4qt.properties` (placed next to the executable). The configurator
resolves appenders, layouts, levels, per-logger settings, and the global header provider by name —
including the custom providers, configured via their `serialNumber` `Q_PROPERTY`.

Contrast this with the sibling **basic** example, which constructs the equivalent pipeline
programmatically in C++.

## Files

| Document | Description |
| --- | --- |
| [main.md](main.md) | Application entry point; registers provider factories, loads the `.properties` file, and runs the event loop. |
| [LoggerObject.md](LoggerObject.md) | Per-instance class logger (`LOG4QT_DECLARE_QCLASS_LOGGER`); stream-style `debug`/`error` whose level is governed by the file (root `ALL`). |
| [LoggerObjectPrio.md](LoggerObjectPrio.md) | Class logger whose level (`ERROR`), `additivity=false`, and appender routing are set by a named-logger block in the file. |
| [LoggerStatic.md](LoggerStatic.md) | Static class logger for non-`QObject` classes (`LOG4QT_DECLARE_STATIC_LOGGER`); logs lifecycle. |

## Building and Running

The example is built as part of the Log4Qt CMake project:

- Target: `propertyconfigurator` (see `CMakeLists.txt`); links against the `log4qt` library.
- Output: placed in `${CMAKE_BINARY_DIR}/bin`.

The configuration file must sit next to the executable with the name
`<executable>.log4qt.properties` (the example ships `propertyconfigurator.exe.log4qt.properties`).
On launch the program loads and watches that file, writes `propertyconfigurator.log` and
`propertyconfigurator.json` (daily-rolling), mirrors output to the console, runs briefly as the demo
objects fire, then logs a STOP banner and exits automatically.

Because `configureAndWatch` is used, editing the properties file while the program runs re-applies
the configuration live.

## Logging Concept Demonstrated

File-driven Log4Qt configuration with `PropertyConfigurator`: appenders, layouts, levels, named
loggers with custom level/additivity/routing, and registering application-defined extension classes
(`HeaderFooterProvider`) with the `Factory` so the configurator can instantiate and configure them
by class name.
