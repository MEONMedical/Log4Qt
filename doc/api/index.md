# Log4Qt Core API — Reference Documentation

Log4Qt is a Qt port of Apache log4j: a logging framework where named **Loggers** gate messages by **Level**, package them as **LoggingEvents**, and dispatch them to attached **Appenders**, each of which uses a **Layout** to render the event for its destination. **Configurators** build this runtime graph from programmatic defaults or external config files, and the **LogManager** singleton ties everything together. This index covers the public classes in `src/log4qt/`.

## Configurators

*Configurators* read configuration data (programmatic defaults or an external file) and build the runtime graph of loggers, appenders, and layouts held by the `LoggerRepository` via the `LogManager` singleton. The file-based configurators flatten their input into a common `Properties` model and delegate the actual work to `PropertyConfigurator`.

| Class | Input source | Description |
|-------|--------------|-------------|
| [BasicConfigurator](BasicConfigurator.md) | Programmatic (none) | Installs a minimal hard-coded console default, or attaches a caller-supplied appender to the root logger. |
| [PropertyConfigurator](PropertyConfigurator.md) | `.properties` file / `Properties` / `QSettings` | Canonical configurator; parses log4j2-style (and legacy log4j1) flat keys into the full logger/appender/layout/filter/policy graph. The engine behind the JSON and XML configurators. |
| [JsonConfigurator](JsonConfigurator.md) | `.json` file | Flattens a JSON document into `Properties` and delegates to `PropertyConfigurator`. |
| [XmlConfigurator](XmlConfigurator.md) | `.xml` file | Flattens XML elements/attributes into `Properties` and delegates to `PropertyConfigurator`. |

All four classes are thread-safe and reside in the `Log4Qt` namespace within the `log4qt` shared library (`Qt6::Core`). `PropertyConfigurator`, `JsonConfigurator`, and `XmlConfigurator` support a `configureAndWatch()` variant that registers the file with `ConfiguratorHelper`, which watches it via a `QFileSystemWatcher` and reloads on change, emitting `ConfiguratorHelper::configurationFileChanged()`.

## Core Logging API

The central classes a developer interacts with when emitting log messages.

| Class | Source | Description |
|-------|--------|-------------|
| [Logger](Logger.md) | `logger.h` / `logger.cpp` | The primary user-facing API: a named node in the logger hierarchy that gates messages by level and dispatches `LoggingEvent`s to attached appenders. Retrieved via `Logger::logger(name)`. |
| [Level](Level.md) | `level.h` / `level.cpp` | Lightweight, ordered value type wrapping a severity (`TRACE` … `FATAL`, plus `NULL`/`ALL`/`OFF`), with string and syslog conversions and serialization. |
| [LoggingEvent](LoggingEvent.md) | `loggingevent.h` / `loggingevent.cpp` | The implicitly-shared record of a single logging occurrence (message, level, logger, timestamp, NDC/MDC, thread, source location) passed to appenders and layouts. |
| [LogStream](LogStream.md) | `logstream.h` / `logstream.cpp` | `ostream`-style helper returned by `Logger`'s no-argument level methods; accumulates `<<`-streamed values and logs them on destruction. |

## Management, Repository & Diagnostic Context

The global entry point, the logger storage/parenting layer, the per-thread diagnostic contexts, and the QML binding.

| Class | Source | Description |
|-------|--------|-------------|
| [LogManager](LogManager.md) | `logmanager.h` / `logmanager.cpp` | Process-wide singleton entry point. Owns the default logger repository, exposes the special loggers (root/log/Qt), and drives startup configuration, version reporting, and Qt-message bridging. |
| [LoggerRepository](LoggerRepository.md) | `loggerrepository.h` / `loggerrepository.cpp` | Abstract interface for a logger repository: logger lookup, root logger, threshold, reset and shutdown. |
| [Hierarchy](Hierarchy.md) | `hierarchy.h` / `hierarchy.cpp` | Concrete `LoggerRepository` storing loggers in a name-keyed hash and parenting them by dotted name. Owns all `Logger` instances. |
| [MDC](MDC.md) | `mdc.h` / `mdc.cpp` | Mapped Diagnostic Context — per-thread key/value map injected into log output. |
| [NDC](NDC.md) | `ndc.h` / `ndc.cpp` | Nested Diagnostic Context — per-thread stack of context strings injected into log output. |
| [QmlLogger](QmlLogger.md) | `qmllogger.h` / `qmllogger.cpp` | QML wrapper exposing logging to QML as the `Logger` element in module `org.log4qt`. Built only with `BUILD_WITH_QML_LOGGING`. |

## Appender Hierarchy

Appenders take fully constructed `LoggingEvent`s and deliver them to a destination — console, file, etc. — applying a layout, a filter chain, and a threshold level along the way. Inheritance chain: `QObject` → `Appender` → `AppenderSkeleton` → `WriterAppender` → `ConsoleAppender` / `FileAppender`, with `ColorConsoleAppender` extending `ConsoleAppender`.

| Class | Role |
|-------|------|
| [Appender](Appender.md) | Abstract `QObject` base defining the appender contract: name, layout, filter chain, `doAppend()`, `close()`. |
| [AppenderSkeleton](AppenderSkeleton.md) | Concrete base implementing layout, filter chain, threshold, active/closed state, and the five-phase, thread-safe `doAppend()` lifecycle. |
| [WriterAppender](WriterAppender.md) | Writes formatted events to a `QTextStream`; adds encoding, immediate-flush, and header/footer handling. |
| [ConsoleAppender](ConsoleAppender.md) | Writes to `stdout` or `stderr`; on Windows falls back to `OutputDebugString` when no console is attached. |
| [ColorConsoleAppender](ColorConsoleAppender.md) | Windows-only `ConsoleAppender` that translates ANSI colour escapes into Win32 console attributes. |
| [FileAppender](FileAppender.md) | Writes to a file; adds append/truncate, buffered I/O, directory creation, and file-rotation primitives. |

### Rolling File Appenders

`FileAppender` subclasses (and one `AppenderSkeleton` subclass) that rotate the log file on size, time, or other triggers.

| Class | Role |
|-------|------|
| [RollingFileAppender](RollingFileAppender.md) | Rotates the active file based on composable `TriggeringPolicy` objects, renaming/pruning backups via a `RolloverStrategy`. Classic `maxFileSize` / `maxBackupIndex` model. |
| [DailyRollingFileAppender](DailyRollingFileAppender.md) | `RollingFileAppender` variant that rolls over on a date pattern (e.g. daily), detecting the day change inline in `append()`. |
| [RandomAccessFileAppender](RandomAccessFileAppender.md) | Extends `AppenderSkeleton` directly; uses a self-managed `QByteArray` buffer and random-access `QFile` writes rather than a `QTextStream`. |

## Layouts

A *layout* converts a `LoggingEvent` into the representation an appender writes (a string, JSON, XML, or an SQL record). The core contract is `QString format(const LoggingEvent &event)`, defined on the root base class and implemented by every concrete layout.

| Class | Source | Description |
|-------|--------|-------------|
| [AbstractLayout](AbstractLayout.md) | `abstractlayout.h` / `.cpp` | Root of the layout hierarchy; defines the abstract `format()` contract, header/footer strings, the per-layout and global `HeaderFooterProvider` chain, `activateOptions()`, and `requiresLocation()`. |
| [AbstractStringLayout](AbstractStringLayout.md) | `abstractstringlayout.h` / `.cpp` | Base for text-producing layouts; adds the `charset` property, the direct byte-encoding `formatTo()` path, and the `thread_local` scratch buffer. |
| [PatternLayout](PatternLayout.md) | `patternlayout.h` / `.cpp` | Formats events with a configurable conversion pattern (`%m`, `%d`, `%p`, `%c`, ...); supports header/footer patterns. |
| [SimpleLayout](SimpleLayout.md) | `simplelayout.h` / `.cpp` | Minimal `LEVEL - message` output, with optional level suppression. |
| [SimpleTimeLayout](SimpleTimeLayout.md) | `simpletimelayout.h` / `.cpp` | Fixed one-line output with timestamp, thread, level, logger, and message. |
| [TTCCLayout](TTCCLayout.md) | `ttcclayout.h` / `.cpp` | Classic log4j TTCC format (Time, Thread, Category, nested Context) with per-element toggles. |
| [JsonLayout](JsonLayout.md) | `jsonlayout.h` / `.cpp` | One JSON object per event (NDJSON / JSON Lines), with selectable fields. |
| [XMLLayout](XMLLayout.md) | `xmllayout.h` / `.cpp` | log4j-compatible `log4j:event` XML fragments. |
| [DatabaseLayout](DatabaseLayout.md) | `databaselayout.h` / `.cpp` | Maps event fields onto named SQL columns and produces a `QSqlRecord` for the database appender. Compiled only when database logging support is enabled. |

## Auxiliary & Specialized Appenders

Additional `AppenderSkeleton` subclasses for asynchronous dispatch, thread marshalling, and alternative sinks (SQL, network, OS log, debugger, Qt signal). None are registered for QML.

| Class | Role |
|-------|------|
| [AsyncAppender](AsyncAppender.md) | Wraps other appenders; queues events on a bounded blocking queue and dispatches them from a dedicated worker thread, with configurable queue-full policies (`Block`/`Discard`/`Synchronous`). Also inherits `AppenderAttachable`. |
| [MainThreadAppender](MainThreadAppender.md) | Marshals events to the main/GUI thread via `QCoreApplication::postEvent()` before forwarding to attached appenders. Also inherits `AppenderAttachable`. |
| [DatabaseAppender](DatabaseAppender.md) | Inserts each event as a row into a SQL table via Qt SQL, driven by a `DatabaseLayout` column mapping. |
| [TelnetAppender](TelnetAppender.md) | Runs a `QTcpServer` (Qt Network) and streams formatted log lines to all connected inbound TCP/telnet clients. |
| [SystemLogAppender](SystemLogAppender.md) | Writes to the OS-native log facility — Windows Event Log or Unix syslog — mapping levels to native severities. |
| [WDCAppender](WDCAppender.md) | Writes to the Windows debugger output channel via `OutputDebugString` (no-op on non-Windows). |
| [SignalAppender](SignalAppender.md) | Emits the formatted message as a Qt signal (`appended(const QString &)`) for in-app handlers. |

## Shared Headers and Macros

Library-wide umbrella header, build/visibility macros, and helper types used throughout Log4Qt.

| File | Description |
|------|-------------|
| [log4qt.h](log4qt.md) | Umbrella/version header: `LOG4QT_VERSION`, version constants and check macro, the `ErrorCode` enum, and compile-time Qt/compiler requirement guards. |
| [log4qtshared.h](log4qtshared.md) | The `LOG4QT_EXPORT` symbol-visibility macro, expanding for static, exporting, and importing builds. (The `LOG4QT_DECLARE_*_LOGGER` convenience macros live in `logger.h`.) |
| [log4qtsharedptr.h](log4qtsharedptr.md) | `Log4QtSharedPtr<T>` — a `QObject` shared pointer that disposes via `deleteLater()`. |

## Triggering Policies and Rollover Strategies (`spi/`)

The service-provider interface (`spi/`) that drives rolling file appenders. A **triggering policy** decides *when* to roll the file over; a **rollover strategy** decides *how* to rename, prune, or stamp the files when a roll happens.

| Class | Role |
|-------|------|
| [TriggeringPolicy](TriggeringPolicy.md) | Abstract base: `bool isTriggeringEvent(...)` plus the `isStartupTrigger()` hook. Held by `RollingFileAppender` via a shared pointer. |
| [SizeBasedTriggeringPolicy](SizeBasedTriggeringPolicy.md) | Triggers when the active file passes a byte threshold (`maximumFileSize` / string `maxFileSize`, default 10 MB). |
| [TimeBasedTriggeringPolicy](TimeBasedTriggeringPolicy.md) | Triggers on date/time boundaries derived from a date pattern, with `interval`, `modulate`, and `maxRandomDelay`. |
| [CronTriggeringPolicy](CronTriggeringPolicy.md) | Triggers on a Quartz-style cron `schedule`, delegating to the `CronExpression` helper. |
| [OnStartupTriggeringPolicy](OnStartupTriggeringPolicy.md) | Rolls once at activation if an existing non-empty file is present. |
| [CompositeTriggeringPolicy](CompositeTriggeringPolicy.md) | OR-combines several child policies; auto-built when more than one policy is attached. |
| [RolloverStrategy](RolloverStrategy.md) | Abstract base: `initialFileName()` plus the pure-virtual `rollover()` contract and protected file-move/remove helpers. |
| [DefaultRolloverStrategy](DefaultRolloverStrategy.md) | Numbered fixed-window rotation between `minIndex` and `maxIndex` (delete-shift-rename). |
| [DateRolloverStrategy](DateRolloverStrategy.md) | Date-stamped rotation with a `NamingMode` (suffix/embedded), `maxBackups`/`keepDays`, and async cleanup. |

## Filters and Header/Footer Providers (`spi/`)

| Class | Role |
|-------|------|
| [Filter](Filter.md) | Abstract base of the appender filter chain; `decide()` returns the `Decision` enum (`Accept`/`Deny`/`Neutral`). `AppenderSkeleton` walks the chain during `doAppend()`. |
| [HeaderFooterProvider](HeaderFooterProvider.md) | Abstract provider supplying dynamic header/footer text to layouts; includes the concrete pattern-based provider. |

## Helpers (`helpers/`)

Infrastructure used throughout the library — object registration, configuration parsing, formatting, concurrency, and diagnostics.

| Class | Role |
|-------|------|
| [Factory](Factory.md) | Singleton registry that creates appenders, layouts, filters, triggering policies, rollover strategies, and header/footer providers by class name, and sets their properties from config. |
| [InitialisationHelper](InitialisationHelper.md) | Process-wide bootstrap singleton: environment/`QSettings` setting resolution, start time, and meta-type registration. |
| [ConfiguratorHelper](ConfiguratorHelper.md) | Holds the active configure callback and watches the config file via `QFileSystemWatcher`, emitting `configurationFileChanged()` on change. |
| [AppenderAttachable](AppenderAttachable.md) | Mix-in giving an object a thread-safe set of attached appenders (used by `Logger` and `AsyncAppender`). |
| [ClassLogger](ClassLogger.md) | Lazily-resolved per-class `Logger` cache backing the `LOG4QT_DECLARE_QCLASS_LOGGER` macro. |
| [PatternFormatter](PatternFormatter.md) | Compiles a conversion-pattern string into tokens and formats `LoggingEvent`s; the engine behind `PatternLayout` and `TTCCLayout`. |
| [OptionConverter](OptionConverter.md) | Converts configuration string options into typed values (bool, int, file size, level, target, encoding) and performs `${...}` substitution. |
| [Properties](Properties.md) | log4j-style string property map with `QIODevice`/`QSettings` loading and a default-fallback chain. |
| [LogError](LogError.md) | Structured error value (message, code, args, causing error) with a thread-local last-error slot, used by Log4Qt's internal error reporting. |
| [DateTime](DateTime.md) | `QDateTime`-based timestamp formatting helper with named formats and thread-local caching. |
| [CronExpression](CronExpression.md) | Parses and evaluates Quartz-style 6-field cron expressions; computes the next fire time. |
| [AsyncWorker](AsyncWorker.md) | `QThread` worker that drains the async queue and dispatches events to `AsyncAppender`'s attached appenders. |
| [BoundedBlockingQueue](BoundedBlockingQueue.md) | Header-only thread-safe bounded producer/consumer queue (blocks on full/empty) backing `AsyncAppender`. |

## Varia — Utility Appenders and Filters (`varia/`)

Simple appenders and the concrete filter implementations.

| Class | Role |
|-------|------|
| [DebugAppender](DebugAppender.md) | Writes to the platform debug channel (`OutputDebugStringW` on Windows, `stderr` elsewhere). |
| [NullAppender](NullAppender.md) | Discards all events; a sink for disabling output or benchmarking. |
| [ListAppender](ListAppender.md) | Accumulates events in an in-memory list (testing/inspection), bounded by `maxCount`. |
| [DenyAllFilter](DenyAllFilter.md) | Always returns `Deny`; the terminator of an allow-list filter chain. |
| [LevelMatchFilter](LevelMatchFilter.md) | Matches one exact `Level` (`levelToMatch` + `acceptOnMatch`). |
| [LevelRangeFilter](LevelRangeFilter.md) | Matches an inclusive `[levelMin, levelMax]` level band. |
| [StringMatchFilter](StringMatchFilter.md) | Matches a substring in the message, with configurable case sensitivity. |
