# LoggerObjectPrio

## 1. Class Overview

`LoggerObjectPrio` demonstrates a **per-instance class logger whose level and routing are set from
the configuration file**. The `LOG4QT_DECLARE_QCLASS_LOGGER` macro gives it a `logger()` named after
the class (`LoggerObjectPrio`). On each tick of a 1 ms `QTimer`, `onTimeout()` emits one `debug` and
one `error` message.

This is the clearest illustration of **priority/level configuration via PropertyConfigurator**. The
properties file declares a named logger for this class:

| Property | Value | Effect |
| --- | --- | --- |
| `logger.LoggerObjectPrio.level` | `ERROR` | Only `error` and higher are logged; the per-tick `debug` is filtered. |
| `logger.LoggerObjectPrio.additivity` | `false` | Output does **not** propagate to the root logger's appenders. |
| `logger.LoggerObjectPrio.appenderRef.console.ref` | `console` | Routed to the console appender. |
| `logger.LoggerObjectPrio.appenderRef.daily.ref` | `daily` | Routed to the daily-file appender. |

So this logger writes `error` messages only to its own console + daily appenders, bypassing the JSON
appender attached to the root.

## 2. Project Structure and Dependencies

| Dependency | Purpose |
| --- | --- |
| `log4qt/logger.h` | `Log4Qt::Logger`, `LOG4QT_DECLARE_QCLASS_LOGGER`. |
| `QObject` | Base class. |
| `QTimer` | 1 ms periodic trigger. |

Files: `loggerobjectprio.h` (API), `loggerobjectprio.cpp` (behavior).

## 3. Class Hierarchy and Role

Derives from `QObject`. A high-rate emitter used to demonstrate level filtering and non-additive
appender routing driven entirely by configuration. Created in `main()` but marked `Q_UNUSED`.

## 8. Public Slots & Q_INVOKABLE

None public. `onTimeout()` is a private slot driven by the internal `QTimer`; it logs a `debug` and
an `error` message each tick.

## 9. Public Methods

#### explicit LoggerObjectPrio(QObject *parent = nullptr)

Creates a child `QTimer`, connects `timeout()` to `onTimeout()`, and starts it at 1 ms.

#### Log4Qt::Logger *logger() const

Provided by `LOG4QT_DECLARE_QCLASS_LOGGER`. Returns the class-named `Logger` (`"LoggerObjectPrio"`).

## 11. Ownership and Lifecycle

Created with `new LoggerObjectPrio(&application)` and parented to the `QCoreApplication`; destroyed
at shutdown. The `QTimer` is parented to `this`.

## 12. Thread Safety

Not thread-safe as a `QObject`; runs in the main thread. The underlying `Log4Qt::Logger` is
thread-safe.

## 14. Inter-Class Interactions

Uses its own class `Logger` (`"LoggerObjectPrio"`), which the properties file matches by name. The
`ERROR` level suppresses the per-tick `debug`, and `additivity=false` keeps its `error` output off
the root logger's JSON appender — it appears only on the console and daily file.

## 16. Usage Example

```cpp
#include "loggerobjectprio.h"
#include <QCoreApplication>

// Configuration file (propertyconfigurator.exe.log4qt.properties):
//   logger.LoggerObjectPrio.level=ERROR
//   logger.LoggerObjectPrio.additivity=false
//   logger.LoggerObjectPrio.appenderRef.console.ref=console
//   logger.LoggerObjectPrio.appenderRef.daily.ref=daily

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    auto *prio = new LoggerObjectPrio(&app);   // logs only error(), to console + daily
    Q_UNUSED(prio)
    return app.exec();
}
```
