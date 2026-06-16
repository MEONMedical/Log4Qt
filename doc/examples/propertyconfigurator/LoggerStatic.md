# LoggerStatic

## 1. Class Overview

`LoggerStatic` demonstrates the **static class logger** pattern for ordinary (non-`QObject`) C++
classes. The `LOG4QT_DECLARE_STATIC_LOGGER(logger, LoggerStatic)` macro defines a file-local static
`logger()` function that lazily creates and caches a `Log4Qt::Logger` named `"LoggerStatic"` (via
`Log4Qt::Logger::logger("LoggerStatic")`). This is the right approach when a class cannot use
`LOG4QT_DECLARE_QCLASS_LOGGER` because it is not a `QObject`.

The class logs a `trace` message from both its constructor and destructor.

## 2. Project Structure and Dependencies

| Dependency | Purpose |
| --- | --- |
| `log4qt/logger.h` | `Log4Qt::Logger`, `LOG4QT_DECLARE_STATIC_LOGGER`. |

Files: `loggerstatic.h` (API), `loggerstatic.cpp` (behavior, macro placement).

## 3. Class Hierarchy and Role

A plain C++ class (no base class, not a `QObject`). Its role is to show Log4Qt logging from outside
the `QObject` hierarchy via a static logger, including lifecycle (constructor/destructor) logging.

## 9. Public Methods

#### LoggerStatic()

Logs a `trace` constructor message via the static `logger()`.

#### ~LoggerStatic()

Logs a `trace` destructor message.

The copy constructor and copy assignment operator are explicitly deleted, so the type is
non-copyable.

## 11. Ownership and Lifecycle

In `main()` the instance is held by a `QScopedPointer<LoggerStatic>` inside the block wrapping
`QCoreApplication::exec()`. It is constructed before the event loop and destroyed when the block
exits — after the loop returns but before the root logger is shut down — so its destructor `trace`
message is still routed through the configured appenders.

## 12. Thread Safety

The class does no internal synchronization, but the static `logger()` accessor uses a function-local
`static` initialized once (thread-safe under C++11+), and `Log4Qt::Logger` itself is thread-safe.

## 14. Inter-Class Interactions

Uses the static `Logger` named `"LoggerStatic"`. Because the properties file sets the root level to
`ALL`, the `trace` messages from the constructor and destructor are delivered to the console,
daily-file, and JSON appenders configured there.

## 16. Usage Example

```cpp
// loggerstatic.cpp
#include "loggerstatic.h"
#include "log4qt/logger.h"

LOG4QT_DECLARE_STATIC_LOGGER(logger, LoggerStatic)

LoggerStatic::LoggerStatic()
{
    logger()->trace() << "ctor Debug output";
}

LoggerStatic::~LoggerStatic()
{
    logger()->trace() << "dtor Debug output";
}
```
