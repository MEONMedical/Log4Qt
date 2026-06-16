# LoggerObject

## 1. Class Overview

`LoggerObject` demonstrates the **per-instance class logger** pattern. The
`LOG4QT_DECLARE_QCLASS_LOGGER` macro adds a `logger()` accessor that returns a `Log4Qt::Logger`
named after the class (`LoggerObject`). The class shows several distinct logging styles in one
place:

- Stream-style logging: `logger()->debug() << ...`, `logger()->error() << ...`
- Argument/printf-style logging: `logger()->debug(QStringLiteral("test"))`
- The convenience `l4q*` macros (`l4qError`, `l4qDebug`) with both message strings, parameter
  substitution, and stream syntax.
- Interop with Qt categorized logging (`qCCritical` on a `Q_LOGGING_CATEGORY`), routed into Log4Qt
  via the `LogManager::setHandleQtMessages(true)` enabled in `main()`.

A `QTimer` drives `onTimeout()` every 10 ms; after 10 iterations the object emits `exit(0)` to end
the application.

## 2. Project Structure and Dependencies

| Dependency | Purpose |
| --- | --- |
| `log4qt/logger.h` | `Log4Qt::Logger`, `LOG4QT_DECLARE_QCLASS_LOGGER`, `l4q*` macros. |
| `QObject` | Base class; enables signals/slots. |
| `QTimer` | Periodic trigger for the demo logging. |
| `QLoggingCategory` | Declares the `category1` (`"test.category1"`) category. |

Files: `loggerobject.h` (API), `loggerobject.cpp` (behavior).

## 3. Class Hierarchy and Role

`LoggerObject` derives from `QObject`. Its role is a self-driving demo emitter: it logs on a timer
and signals the application to quit once it has produced enough output.

## 7. Signals

#### void exit(int code)

Emitted once the internal counter reaches 10. Connected in `main()` to `QCoreApplication::exit`,
terminating the event loop.

## 8. Public Slots & Q_INVOKABLE

None public. `onTimeout()` is a private slot invoked by the internal `QTimer`. It emits the logging
calls listed in the overview and increments the counter.

## 9. Public Methods

#### explicit LoggerObject(QObject *parent = nullptr)

Constructs the object, creates a child `QTimer`, connects its `timeout()` signal to `onTimeout()`,
and starts it with a 10 ms interval. Initializes the iteration counter to 0.

#### Log4Qt::Logger *logger() const

Provided by `LOG4QT_DECLARE_QCLASS_LOGGER`. Returns the class-named `Logger` (cached on first call
via an internal `Log4Qt::ClassLogger`).

## 11. Ownership and Lifecycle

Created with `new LoggerObject(&application)` and parented to the `QCoreApplication`, so Qt destroys
it at application shutdown. The internal `QTimer` is parented to `this` and destroyed with the
object. Copy and move are disabled (`Q_DISABLE_COPY_MOVE`).

## 12. Thread Safety

Not thread-safe as a `QObject`; it lives and runs in the thread that owns its timer (the main
thread). The underlying `Log4Qt::Logger` itself is documented as thread-safe.

## 14. Inter-Class Interactions

Uses its own class `Logger` (`"LoggerObject"`) obtained through `logger()`. Because `main()` sets
the root logger level to `INFO`, `debug` calls are suppressed while `error` calls pass through to
the console, plain-text, and JSON appenders configured there. The `qCCritical(category1, ...)` call
is bridged into Log4Qt by the global Qt message handler enabled in `main()`.

## 16. Usage Example

```cpp
#include "loggerobject.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Root logger must be configured first (see setupRootLogger in main.cpp).
    auto *demo = new LoggerObject(&app);
    QObject::connect(demo, &LoggerObject::exit, &app, &QCoreApplication::exit);

    return app.exec();   // demo logs every 10 ms, then quits after 10 ticks
}
```
