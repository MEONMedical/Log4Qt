# Basic Example — Documentation Index

This example shows how to configure Log4Qt **programmatically**, with no external configuration
file. `main()` builds the logging pipeline in code (`BasicConfigurator`-style manual setup): it
creates a `TTCCLayout`, attaches a `ConsoleAppender`, a plain-text `FileAppender`, and a JSON
`FileAppender` to the root logger, sets the root level to `INFO`, and enables routing of Qt's own
messages into Log4Qt. Custom `HeaderFooterProvider` subclasses inject a device serial number into
each log file header.

Contrast this with the sibling **propertyconfigurator** example, which loads the equivalent pipeline
from a log4j-style `.properties` file at runtime.

## Files

| Document | Description |
| --- | --- |
| [main.md](main.md) | Application entry point; builds the root logger, layouts, and appenders in code and runs the event loop. |
| [LoggerObject.md](LoggerObject.md) | Per-instance class logger (`LOG4QT_DECLARE_QCLASS_LOGGER`); shows stream, printf-style, and `l4q*` macro logging plus Qt categorized-logging interop. |
| [LoggerObjectPrio.md](LoggerObjectPrio.md) | A second class logger demonstrating level/priority filtering on a fast timer. |
| [LoggerStatic.md](LoggerStatic.md) | Static class logger for non-`QObject` classes (`LOG4QT_DECLARE_STATIC_LOGGER`); logs lifecycle and library version. |

## Building and Running

The example is built as part of the Log4Qt CMake project:

- Target: `basic` (see `CMakeLists.txt`); links against the `log4qt` library.
- Output: placed in `${CMAKE_BINARY_DIR}/bin`.

Run the resulting `basic` executable. It writes `basic.log` and `basic.json` next to the executable
and mirrors output to the console. The program logs a START banner, runs for a short time as the
demo objects fire on their timers, then logs a STOP banner and exits automatically (after
`LoggerObject` completes 10 iterations).

## Logging Concept Demonstrated

Manual, in-code Log4Qt configuration: constructing layouts and appenders, attaching them to the root
logger, setting levels, bridging Qt messages, and three different ways for application classes to
obtain and use a `Logger`.
