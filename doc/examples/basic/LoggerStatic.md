# LoggerStatic

## 1. Class Overview

`LoggerStatic` demonstrates the **static class logger** pattern for ordinary (non-`QObject`) C++
classes. The `LOG4QT_DECLARE_STATIC_LOGGER(logger, LoggerStatic)` macro defines a file-local static
`logger()` function that lazily creates and caches a `Log4Qt::Logger` named `"LoggerStatic"` (via
`Log4Qt::Logger::logger("LoggerStatic")`). This is the appropriate approach when a class is not a
`QObject` and therefore cannot use `LOG4QT_DECLARE_QCLASS_LOGGER`.

The class logs from its constructor and destructor and, in this *basic* example, also prints the
Log4Qt version string and version number — illustrating both lifecycle logging and querying library
metadata.

## 2. Project Structure and Dependencies

| Dependency | Purpose |
| --- | --- |
| `log4qt/logger.h` | `Log4Qt::Logger`, `LOG4QT_DECLARE_STATIC_LOGGER`. |
| `log4qt/logmanager.h` | `LogManager::version()` and `LogManager::versionNumber()`. |

Files: `loggerstatic.h` (API), `loggerstatic.cpp` (behavior, macro placement).

## 3. Class Hierarchy and Role

A plain C++ class (no base class, not a `QObject`). Its role is to show that Log4Qt logging works
outside the `QObject` hierarchy via a static logger, and to surface the running Log4Qt version at
startup.

## 9. Public Methods

#### LoggerStatic()

Logs a `trace` constructor message, then logs two `info` messages reporting
`Log4Qt::LogManager::version()` (string) and `Log4Qt::LogManager::versionNumber().toString()`.

#### ~LoggerStatic()

Logs a `trace` destructor message.

The copy constructor and copy assignment operator are explicitly deleted, so the type is
non-copyable.

## 11. Ownership and Lifecycle

In `main()` the instance is held by a `QScopedPointer<LoggerStatic>` inside the block that wraps
`QCoreApplication::exec()`. It is constructed before the event loop and destroyed when the block
exits — after the loop returns but before the root logger is shut down — so its destructor `trace`
message is still routed through the configured appenders.

## 12. Thread Safety

The class does no internal synchronization, but the static `logger()` accessor uses a function-local
`static` initialized once (thread-safe under C++11+), and `Log4Qt::Logger` itself is thread-safe.

## 14. Inter-Class Interactions

Uses the static `Logger` named `"LoggerStatic"`. With the root level at `INFO` (set in
`setupRootLogger()`), the `trace` messages from the constructor/destructor are suppressed while the
two `info` version messages are delivered to the console, plain-text, and JSON appenders.

## 16. Usage Example

```cpp
// loggerstatic.cpp
#include "loggerstatic.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"

LOG4QT_DECLARE_STATIC_LOGGER(logger, LoggerStatic)

LoggerStatic::LoggerStatic()
{
    logger()->trace() << "ctor Debug output";
    logger()->info()  << "Log4Qt Version String: " << Log4Qt::LogManager::version();
}
```
