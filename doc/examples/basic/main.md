# main.cpp — Basic Example Entry Point

## A. Overview

This example demonstrates **programmatic configuration** of Log4Qt. Rather than reading a
configuration file, `main()` builds the logging pipeline in code: it creates layouts, a console
appender, a plain-text file appender, and a JSON file appender, attaches them to the root logger,
sets the root level, and routes Qt's own diagnostic messages (`qDebug`/`qWarning`/...) through
Log4Qt.

`main()`'s role is to:

1. Create the `QCoreApplication` and the demo logger objects.
2. Build and activate the root logger's appenders and layouts in code (`setupRootLogger`).
3. Emit framing START/STOP banner messages around the application's lifetime.
4. Run the event loop so the timer-driven demo objects produce log output.
5. Tear the logging system down cleanly on exit (`shutDownRootLogger`).

It also shows two custom `HeaderFooterProvider` subclasses that inject a device serial number into
file headers — one producing a plain-text header, one producing JSON header/footer objects.

## B. Qt Application Setup

A `QCoreApplication` is created on the stack (console application, no GUI). No special application
attributes are set. `QCoreApplication::applicationDirPath()` is later used to place the log files
next to the executable.

## C. Command-Line Handling

`argc`/`argv` are forwarded to `QCoreApplication` but no arguments are parsed by the example.

The example does configure Qt's categorized logging filter rules directly:

| Rule | Effect |
| --- | --- |
| `*.debug=false` | Disables `debug`-level output for all Qt logging categories. |
| `test.category1.debug=true` | Re-enables `debug` for the `test.category1` category used by `LoggerObject`. |

## D. Top-Level Object Creation

| Object | Type | Lifetime | Demonstrates |
| --- | --- | --- | --- |
| `object` | `LoggerObject` | Parented to `application` | Per-instance class logger via `LOG4QT_DECLARE_QCLASS_LOGGER`, stream and printf-style logging, `l4q*` macros, and Qt categorized logging interop. Emits `exit(0)` after 10 timer ticks. |
| `object1` | `LoggerObjectPrio` | Parented to `application` | A second class logger firing on a fast (1 ms) timer; in this example its level is governed by the root logger. Marked `Q_UNUSED`. |
| `object2` | `LoggerStatic` | `QScopedPointer`, scoped to the event-loop block | Static, non-`QObject` class logger via `LOG4QT_DECLARE_STATIC_LOGGER`; logs from constructor/destructor and prints the Log4Qt version. |

The `LoggerObject::exit(int)` signal is connected to `QCoreApplication::exit`, so the application
shuts down once `object` has logged ten iterations.

## E. Wiring and Connections

Logging is configured **entirely in code** by `setupRootLogger()` — this is the key contrast with
the `propertyconfigurator` example, which loads the same kind of pipeline from a `.properties`
file.

Steps performed by `setupRootLogger()`:

1. **Global header provider** — a `SerialNumberHeaderProvider` is created, given a serial number,
   and registered via `Log4Qt::AbstractLayout::setGlobalHeaderFooterProvider(...)`. Every file
   opened afterwards gets `Device S/N: SN-20260001` as its header line.
2. **Layout** — a `TTCCLayout` (time / thread / category / level) named `"My Layout"` is created and
   activated, then shared (`LayoutSharedPtr`).
3. **Console appender** — a `ConsoleAppender` targeting `StdOut`, using the shared layout, is named,
   activated, and added to the root logger.
4. **Plain file appender** — a `FileAppender` writing `basic.log` next to the executable
   (append mode), using the same shared layout.
5. **JSON file appender** — a `JsonLayout` is given its own `JsonSerialNumberHeaderProvider`
   (machine-readable JSON header/footer) and used by a `FileAppender` writing `basic.json`.
6. **Root level** — set to `Log4Qt::Level::INFO_INT`.
7. **Qt message handling** — `Log4Qt::LogManager::setHandleQtMessages(true)` so `qDebug`/`qWarning`
   etc. flow into Log4Qt.

`shutDownRootLogger()` reverses this: logs a closing message, calls `removeAllAppenders()`,
`loggerRepository()->shutdown()`, and clears the global header/footer provider.

A representative excerpt of the in-code pipeline:

```cpp
auto *layout = new Log4Qt::TTCCLayout();
layout->setName(QStringLiteral("My Layout"));
layout->activateOptions();
Log4Qt::LayoutSharedPtr layoutPtr(layout);

auto *consoleAppender =
    new Log4Qt::ConsoleAppender(layoutPtr, Log4Qt::ConsoleAppender::StdOut);
consoleAppender->setName(QStringLiteral("My Appender"));
consoleAppender->activateOptions();
logger->addAppender(Log4Qt::AppenderSharedPtr(consoleAppender));

logger->setLevel(Log4Qt::Level::INFO_INT);
Log4Qt::LogManager::setHandleQtMessages(true);
```

## F. Event Loop

Yes. After setup, `QCoreApplication::exec()` runs inside a scoped block that owns the
`LoggerStatic` instance. The timers in `LoggerObject` (10 ms) and `LoggerObjectPrio` (1 ms) drive
logging until `LoggerObject` emits `exit(0)`, which ends the loop. On block exit, `LoggerStatic` is
destroyed (logging its destructor message before shutdown).

## G. Dependencies

**Qt modules**

| Header | Provides |
| --- | --- |
| `QCoreApplication` | Console event loop, application paths. |
| `QDateTime` | Timestamps for the JSON header/footer. |
| `QScopedPointer` | Scoped ownership of the `LoggerStatic` instance. |
| `QString`, `QStringBuilder` | String construction. |
| `QFile` | Path helpers. |
| `QLoggingCategory` | Qt categorized-logging filter rules. |

**Log4Qt headers**

| Header | Provides |
| --- | --- |
| `log4qt/logger.h` | `Logger`, the logging macros. |
| `log4qt/propertyconfigurator.h` | (Included; not used for configuration in this example.) |
| `log4qt/loggerrepository.h` | Repository `shutdown()` on teardown. |
| `log4qt/consoleappender.h` | `ConsoleAppender`. |
| `log4qt/fileappender.h` | `FileAppender`. |
| `log4qt/ttcclayout.h` | `TTCCLayout`. |
| `log4qt/jsonlayout.h` | `JsonLayout`. |
| `log4qt/abstractlayout.h` | `AbstractLayout::setGlobalHeaderFooterProvider`. |
| `log4qt/logmanager.h` | `LogManager::setHandleQtMessages`, version info. |
| `log4qt/spi/headerfooterprovider.h` | `HeaderFooterProvider` base class for the custom providers. |

## Custom Helper Classes (defined in main.cpp)

#### class SerialNumberHeaderProvider : public Log4Qt::HeaderFooterProvider

Writes a single-line plain-text header `Device S/N: <serial>`. The serial number is injected via
`setSerialNumber(const QString &)` before the provider is registered globally.

#### class JsonSerialNumberHeaderProvider : public Log4Qt::HeaderFooterProvider

Overrides both `header()` and `footer()` to emit JSON objects
(`{"event":"start",...}` / `{"event":"end",...}`) carrying the serial number and an ISO-8601
timestamp captured at file open/close. Attached to the `JsonLayout` rather than registered globally.
