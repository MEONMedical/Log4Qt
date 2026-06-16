# BasicConfigurator

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging library. It builds a runtime graph of *loggers*, *appenders* (output sinks), and *layouts* (formatters), all owned by a central `LoggerRepository` reachable through the `LogManager` singleton. *Configurators* are the classes that populate that graph.

`BasicConfigurator` is the simplest configurator. Rather than reading an external configuration file, it installs a minimal, hard-coded default configuration in code: it attaches a `ConsoleAppender` with a `PatternLayout` to the root logger so that any application can produce sensible console output with a single call. A developer reaches for `BasicConfigurator` when no external configuration file is wanted — for example in tests, small tools, or as a fallback before a richer configuration is loaded.

## 2. Project Structure and Dependencies

`BasicConfigurator` is declared in `basicconfigurator.h` and implemented in `basicconfigurator.cpp`. It is part of the `log4qt` shared library target.

The implementation depends on these project-internal types:

- `Appender` (forward-declared in the header) — base interface for all output sinks. The single-argument `configure()` overload takes ownership of an externally created `Appender`.
- `ConsoleAppender` — writes formatted log output to `stdout`/`stderr`.
- `ListAppender` (from `varia/listappender.h`) — an in-memory appender used here to capture configuration-time errors.
- `PatternLayout` — formats logging events; `BasicConfigurator` uses the predefined `TtccPattern`.
- `LogManager` — provides access to the root logger and the internal `logLogger`.
- `ConfiguratorHelper` (from `helpers/configuratorhelper.h`) — stores the configuration error list reported by the last operation.

Build requirement: links against `Qt6::Core` (the `log4qt` library's public Qt dependency). The implementation includes `QCoreApplication` and `QThread`.

## 3. Class Hierarchy and Role

`BasicConfigurator` has no base class. It is a non-instantiable utility class exposing only static functions. Copy and move are disabled via `Q_DISABLE_COPY_MOVE(BasicConfigurator)`, and it has no public constructor, so it is never instantiated — it acts purely as a namespace for its static configuration entry points.

## 9. Public Methods

#### static bool configure()

Installs the default configuration. Creates a `ConsoleAppender` writing to `stdout` with a `PatternLayout` using `PatternLayout::TtccPattern`, names both objects, calls `activateOptions()` on them, and adds the appender to the root logger.

While configuring, it temporarily attaches a threshold-`ERROR` `ListAppender` named `"BasicConfigurator"` to the internal log logger to capture any error events raised during setup. After configuration it detaches that appender and publishes the captured errors via `ConfiguratorHelper::setConfigureError()`.

Returns `true` if no errors were captured (the captured error list is empty), `false` otherwise. Inspect `ConfiguratorHelper::configureError()` for details on failure.

#### static void configure(Appender *pAppender)

Adds the caller-supplied `pAppender` directly to the root logger. This is the building-block entry point for code that has already created and configured its own appender. Ownership of `pAppender` transfers to the logging framework (see Ownership and Lifecycle).

#### static void resetConfiguration()

Resets the entire logging configuration by delegating to `LogManager::resetConfiguration()`. Removes all appenders from all loggers and returns the repository to its default, unconfigured state. Call this before re-configuring, or during shutdown.

## 11. Ownership and Lifecycle

`BasicConfigurator` is never instantiated; all members are static.

The objects it creates are owned by the logging framework through reference-counted shared pointers:

- The default `ConsoleAppender` is wrapped in an `AppenderSharedPtr` and added to the root logger. The repository keeps it alive; the caller does not delete it.
- The `PatternLayout` is held by a `LayoutSharedPtr` and owned by the appender.
- The temporary `ListAppender` used for error capture is held in a local `AppenderSharedPtr` and released when `configure()` returns.

For `configure(Appender *pAppender)`, the raw pointer is immediately wrapped in an `AppenderSharedPtr` and handed to the root logger. The caller must **not** delete `pAppender` afterwards and must not wrap it in another owning pointer — ownership is transferred to the framework.

## 12. Thread Safety

All functions are thread-safe, as documented in the header. Internally they operate on the thread-safe `LogManager` loggers and the mutex-guarded `ConfiguratorHelper`. They may be called from any thread.

## 14. Inter-Class Interactions

- Reads the root logger and the internal log logger from `LogManager`.
- Publishes configuration errors to the shared `ConfiguratorHelper` singleton, which other code can query via `configureError()`.
- Delegates `resetConfiguration()` to `LogManager`.
- Produces a `ConsoleAppender` + `PatternLayout` pair attached to the root logger, feeding the same logger/appender graph that the file-based configurators populate.

Unlike `ConfiguratorHelper`-driven configurators, `BasicConfigurator` does not register a file watch and emits no signals itself.

## 16. Usage Example

```cpp
#include <log4qt/basicconfigurator.h>
#include <log4qt/logger.h>
#include <log4qt/consoleappender.h>
#include <log4qt/patternlayout.h>

using namespace Log4Qt;

int main(int argc, char *argv[])
{
    // Option A: install the built-in console default.
    BasicConfigurator::configure();

    // Option B: supply a custom appender; ownership passes to the framework.
    auto *layout = new PatternLayout(PatternLayout::TtccPattern);
    layout->activateOptions();
    auto *appender = new ConsoleAppender(LayoutSharedPtr(layout),
                                         ConsoleAppender::StdErr);
    appender->activateOptions();
    BasicConfigurator::configure(appender);

    Logger::logger("MyApp")->info("Logging is configured");

    // On shutdown:
    BasicConfigurator::resetConfiguration();
    return 0;
}
```
