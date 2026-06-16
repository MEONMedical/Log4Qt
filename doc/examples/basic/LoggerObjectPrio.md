# LoggerObjectPrio

## 1. Class Overview

`LoggerObjectPrio` demonstrates a **second per-instance class logger** running at high frequency. As
with `LoggerObject`, it uses `LOG4QT_DECLARE_QCLASS_LOGGER` to obtain a `logger()` named after the
class (`LoggerObjectPrio`). Its `onTimeout()` slot emits one `debug` and one `error` message per
tick on a 1 ms timer.

The class name signals **priority/level** demonstration: which of its two messages actually appear
is decided by the effective level of its logger. In the *basic* example the logger inherits the
root level (`INFO`), so the `debug` line is filtered out and only `error` reaches the appenders. (In
the *propertyconfigurator* example the same class is given an explicit `ERROR` level and its own
appenders via the config file — the comparison highlights how level/additivity move from code to
configuration.)

## 2. Project Structure and Dependencies

| Dependency | Purpose |
| --- | --- |
| `log4qt/logger.h` | `Log4Qt::Logger`, `LOG4QT_DECLARE_QCLASS_LOGGER`. |
| `QObject` | Base class. |
| `QTimer` | 1 ms periodic trigger. |

Files: `loggerobjectprio.h` (API), `loggerobjectprio.cpp` (behavior).

## 3. Class Hierarchy and Role

Derives from `QObject`. Acts as a high-rate emitter that exercises level-based filtering. It does
not signal application exit; in `main()` the instance is created but marked `Q_UNUSED`.

## 8. Public Slots & Q_INVOKABLE

None public. `onTimeout()` is a private slot driven by the internal `QTimer`; it logs a `debug` and
an `error` message each tick.

## 9. Public Methods

#### explicit LoggerObjectPrio(QObject *parent = nullptr)

Creates a child `QTimer`, connects `timeout()` to `onTimeout()`, and starts it with a 1 ms interval.

#### Log4Qt::Logger *logger() const

Provided by `LOG4QT_DECLARE_QCLASS_LOGGER`. Returns the class-named `Logger` (`"LoggerObjectPrio"`).

## 11. Ownership and Lifecycle

Created with `new LoggerObjectPrio(&application)` and parented to the `QCoreApplication`; destroyed
at application shutdown. The `QTimer` is parented to `this`.

## 12. Thread Safety

Not thread-safe as a `QObject`; runs in the main thread. The underlying `Log4Qt::Logger` is
thread-safe.

## 14. Inter-Class Interactions

Uses its own class `Logger` (`"LoggerObjectPrio"`). In this example it has no explicit level, so its
effective level is inherited from the root logger configured in `setupRootLogger()` (`INFO`),
meaning `debug` output is suppressed and `error` output is delivered to the console, plain-text, and
JSON appenders.

## 16. Usage Example

```cpp
#include "loggerobjectprio.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // With the root level at INFO, only the error() line is emitted each tick.
    auto *prio = new LoggerObjectPrio(&app);
    Q_UNUSED(prio)

    return app.exec();
}
```
