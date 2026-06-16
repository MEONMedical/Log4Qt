# LoggerObject

## 1. Class Overview

`LoggerObject` demonstrates the **per-instance class logger** pattern. The
`LOG4QT_DECLARE_QCLASS_LOGGER` macro adds a `logger()` accessor returning a `Log4Qt::Logger` named
after the class (`LoggerObject`). On each tick of a 10 ms `QTimer`, `onTimeout()` emits one
stream-style `debug` and one stream-style `error` message; after 10 iterations the object emits
`exit(0)` to stop the application.

Unlike the *basic* example's `LoggerObject`, this version uses only the stream API and does not
exercise the `l4q*` macros or Qt categorized logging — the point here is that the *same* class logger
is configured externally by the properties file (root level `ALL`) rather than in code.

## 2. Project Structure and Dependencies

| Dependency | Purpose |
| --- | --- |
| `log4qt/logger.h` | `Log4Qt::Logger`, `LOG4QT_DECLARE_QCLASS_LOGGER`. |
| `QObject` | Base class; signals/slots. |
| `QTimer` | 10 ms periodic trigger. |

Files: `loggerobject.h` (API), `loggerobject.cpp` (behavior).

## 3. Class Hierarchy and Role

Derives from `QObject`. Acts as the self-driving emitter that also terminates the application once
it has produced enough output.

## 7. Signals

#### void exit(int code)

Emitted once the internal counter reaches 10. Connected in `main()` to `QCoreApplication::exit`.

## 8. Public Slots & Q_INVOKABLE

None public. `onTimeout()` is a private slot driven by the internal `QTimer`; it logs a `debug` and
an `error` message and increments the counter.

## 9. Public Methods

#### explicit LoggerObject(QObject *parent = nullptr)

Creates a child `QTimer`, connects `timeout()` to `onTimeout()`, starts it at 10 ms, and initializes
the counter to 0.

#### Log4Qt::Logger *logger() const

Provided by `LOG4QT_DECLARE_QCLASS_LOGGER`. Returns the class-named `Logger` (`"LoggerObject"`).

## 11. Ownership and Lifecycle

Created with `new LoggerObject(&application)` and parented to the `QCoreApplication`; destroyed at
shutdown. The `QTimer` is parented to `this`.

## 12. Thread Safety

Not thread-safe as a `QObject`; runs in the main thread. The underlying `Log4Qt::Logger` is
thread-safe.

## 14. Inter-Class Interactions

Uses its own class `Logger` (`"LoggerObject"`). Its effective level comes from the **properties
file**: the root logger is set to `ALL`, so both the `debug` and `error` messages pass through to
the console, daily-file, and JSON appenders defined there.

## 16. Usage Example

```cpp
#include "loggerobject.h"
#include "log4qt/propertyconfigurator.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Logging is configured from a file; the class logger's level is governed by it.
    Log4Qt::PropertyConfigurator::configureAndWatch(
        QCoreApplication::applicationFilePath() + ".log4qt.properties");

    auto *demo = new LoggerObject(&app);
    QObject::connect(demo, &LoggerObject::exit, &app, &QCoreApplication::exit);

    return app.exec();
}
```
