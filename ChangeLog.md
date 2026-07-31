# Change Log

All notable changes to this project will be documented in this file.
----
## [v2.0.0] - ??
### Added
- JsonConfigurator for configuring logging from JSON files (`log4qt.json`).
  The JSON structure maps directly to flat property keys via dot-separated
  nesting. Automatic initialization searches for `log4qt.json` alongside
  `log4qt.properties` (`.properties` takes priority).
- XmlConfigurator for configuring logging from XML files (`log4qt.xml`).
  Nested elements produce dot-separated keys; XML attributes are flattened
  as child properties. Automatic initialization searches for `log4qt.xml`
  after `log4qt.json` (`.properties` > `.json` > `.xml` priority).
- Short factory aliases for all built-in components (e.g. `Console`,
  `File`, `RollingFile`, `PatternLayout`, `LevelMatch`) alongside the
  existing `org.apache.log4j.*` and `Log4Qt::*` names.
- **TriggeringPolicy / RolloverStrategy architecture** for RollingFileAppender,
  inspired by log4j2. Rolling is now decoupled into pluggable policies (WHEN
  to roll) and strategies (HOW to roll):
  - `SizeBasedTriggeringPolicy` -- triggers when file size exceeds a threshold.
  - `TimeBasedTriggeringPolicy` -- triggers based on a date/time pattern
    (minutely through monthly).
  - `CronTriggeringPolicy` -- triggers on a Quartz-style cron schedule
    (6-field format: seconds minutes hours day-of-month month day-of-week).
  - `OnStartupTriggeringPolicy` -- triggers once at startup if the log file
    already exists and is non-empty.
  - `CompositeTriggeringPolicy` -- OR-combines multiple policies; created
    automatically when multiple policies are added to an appender.
  - `DefaultRolloverStrategy` -- fixed-window numbered backup rotation
    (configurable min/max index).
  - `DateRolloverStrategy` -- date-based rotation with two naming modes:
    `Suffix` (e.g. `app.log.2026-03-28`) and `Embedded` (e.g.
    `app_2026-03-28.log`). Supports a configurable `maxBackups` limit and
    a `DateTimeProvider` callback for test mockability. Backup files are
    named after the period they belong to, and obsolete files are cleaned
    up asynchronously. A `datedActiveFile` property makes the active log
    file itself carry the current date from the first startup, so each
    period writes directly to its own dated file without any rename on
    rollover.
  - `RolloverStrategy::initialFileName()` -- new virtual hook that lets a
    strategy choose the initial active filename before the file is opened
    (e.g. to inject today's date). `RollingFileAppender` remembers the
    configured base filename and always passes it to the strategy, so
    rollovers never see an already-transformed name.
  - New `CronExpression` helper class for parsing and evaluating Quartz-style
    cron expressions.
  - Policies and strategies are configured via
    `appender.X.policy.<alias>.type` and `appender.X.strategy.type` keys.
  - Factory registration with short aliases (`SizeBased`, `TimeBased`,
    `Cron`, `OnStartup`, `Default`, `Date`).
- Dedicated PropertyConfigurator unit test suite (`tests/propertytest`).
- Dedicated policy/strategy unit test suite (`tests/policytest`) with 91
  test cases covering all triggering policies, rollover strategies, cron
  expression parsing, factory registration, and configurator integration.
- **Layout hierarchy** refactored: `Layout` renamed to `AbstractLayout`;
  new `AbstractStringLayout` intermediate base for all text-producing layouts,
  adding `charset`, `contentType()`, `formatTo()`, and `threadLocalBuffer()`.
  `virtual bool requiresLocation()` added to `AbstractLayout` (default `false`);
  `PatternLayout` returns `true` when the pattern contains `%F`, `%L`, `%M`,
  or `%l`.
- **`JsonLayout`** — new layout producing NDJSON (one JSON object per line).
  Nine boolean properties control which fields are emitted (`includeTimestamp`,
  `includeLevel`, `includeLogger`, `includeThread`, `includeMessage`,
  `includeNdc`, `includeMdc`, `includeLocation`, `prettyPrint`). Timestamp is
  Unix epoch milliseconds. `contentType()` returns
  `"application/json; charset=UTF-8"`.
- **`RandomAccessFileAppender`** — high-throughput file appender using a
  user-space write buffer (default 256 KB) and direct `QFile::write()`,
  bypassing `QTextStream`. Integrates with `AbstractStringLayout::formatTo()`
  to avoid intermediate `QString` allocations. Writes layout header/footer on
  file open/close.
- **`AsyncAppender`** rewritten with `BoundedBlockingQueue` and configurable
  queue-full policies: `Block` (default), `Discard` (drops events below a
  configurable threshold), and `Synchronous` (falls back to caller-thread
  dispatch). Adds `errorAppender`, `discardedCount`, `shutdownTimeout`, and
  `batchComplete()` signal. Worker thread named `Log4Qt-Async-<name>`.
- **`AppenderSkeleton::doAppend()` split-lock** — layout formatting runs
  outside `mObjectGuard`; only the final write is serialised. New protected
  virtual `preAppend()` hook and `forwardEvent()` static helper for
  intentional cross-appender dispatch (used by `AsyncAppender`).
- **`DateTime::formatMsecs()`** — static method with thread-local cache for
  named formats (`ISO8601`, `ABSOLUTE`, `DATE`). Repeated calls within the
  same millisecond cost only a `qint64` comparison.
- Dedicated unit test suites for `AsyncAppender` (`tests/asyncappendertest`)
  and `JsonLayout` (`tests/jsonlayouttest`).
- All CTest/build target names now carry a `tst_` prefix.
- **`HeaderFooterProvider` SPI** (`spi/headerfooterprovider.h`) — pluggable
  interface for dynamic log-file headers and footers. Applications subclass
  `HeaderFooterProvider` and register it with the `Factory` to inject runtime
  content (e.g. device serial numbers) without touching layout classes.

### Changed
- **Breaking:** Minimum required Qt version raised to Qt 6.5. Qt 5 is no longer
  supported. Use branch 1.6 / release 1.6.x for Qt 5.7+ support.
  All pre-6.5 `#if QT_VERSION` code paths have been removed.
- **Breaking:** PropertyConfigurator now uses a Log4j2-style configuration
  format. The `log4j.` prefix is removed, appenders use explicit `type` keys,
  and loggers use appender references instead of inline declarations:
  - `log4j.rootLogger=ALL, console` &rarr; `rootLogger.level=ALL` +
    `rootLogger.appenderRef.0.ref=console`
  - `log4j.appender.X=org.apache.log4j.ConsoleAppender` &rarr;
    `appender.X.type=Console`
  - `log4j.appender.X.layout=org.apache.log4j.TTCCLayout` &rarr;
    `appender.X.layout.type=TTCCLayout`
  - `log4j.logger.MyApp=ERROR, console` &rarr; `logger.MyApp.name=MyApp` +
    `logger.MyApp.level=ERROR` + `logger.MyApp.appenderRef.0.ref=console`
  - `log4j.additivity.MyApp=false` &rarr; `logger.MyApp.additivity=false`
  - Global settings: `log4j.reset` &rarr; `reset`, `log4j.Debug` &rarr;
    `status`, `log4j.threshold` &rarr; `threshold`, etc.
- Appender filters now configured via `appender.X.filter.F.type=LevelMatch`
  with properties under the same prefix.
- **Breaking:** RollingFileAppender no longer has `maxFileSize`,
  `maximumFileSize`, or `maxBackupIndex` properties. Use the new
  TriggeringPolicy and RolloverStrategy configuration instead:
  - `appender.X.policy.<alias>.type=SizeBasedTriggeringPolicy` with
    `maxFileSize=10MB` replaces the old `maxFileSize` property.
  - `appender.X.strategy.type=DefaultRolloverStrategy` with
    `maxIndex=7` replaces the old `maxBackupIndex` property.

### Improvements
- Modernized codebase to C++20; applied `const` correctness and `constexpr`
  improvements throughout.
- `SizeBasedTriggeringPolicy` uses `QIODevice::pos()` instead of `size()` for
  accurate in-progress file size tracking.
- `TriggeringPolicy::isTriggeringEvent()` now receives a `QIODevice *` directly.
- `TimeBasedTriggeringPolicy` date-pattern parsing replaces the former
  `Frequency` enum with direct pattern analysis.

### Changed (continued)
- **Breaking:** `DailyFileAppender` renamed to `DailyRollingFileAppender` and
  refactored to inherit from `RollingFileAppender`. It now delegates rollover
  to `DateRolloverStrategy(Embedded)` instead of implementing its own file
  naming logic. The `IDateRetriever` testability interface is preserved via
  `DateTimeProvider`. The factory short alias `DailyFile` is unchanged.
- **Breaking:** Removed the old `DailyRollingFileAppender` class (suffix-based
  daily rotation). Use `RollingFileAppender` with `TimeBasedTriggeringPolicy`
  and `DateRolloverStrategy(Suffix)` instead.

### Fixed
- Use-after-free crash at worker-thread exit: the per-thread name cache in
  `LoggingEvent` observed the `QThread`'s `objectName` via a `thread_local`
  `QPropertyNotifier`, whose destructor ran at OS-thread exit — after the
  `QThread` object may already have been deleted through the canonical
  `connect(thread, finished, thread, deleteLater)` cleanup pattern. The cache
  is now invalidated through a `QObject::objectNameChanged` connection with a
  shared atomic dirty flag, so thread-local teardown never touches the
  `QThread`. Affected any code that logs from worker threads cleaned up with
  `deleteLater`.

### Deprecated / Removed
- Removed qmake support
- Removed binary logger

## [v1.6.0] - ??

### Improvements
- qmake: Make database, telnet appender and qml logger optional depending on CONFIG
- cmake: Make database, telnet appender and qml logger optional depending on option
- Use std::as_const instead of deprecated qAsConst for c++17 and greater
- Use QMetaType instead of QVariant type
- if console is blocked by debugger use OutputDebugString (win)
- Support for Qt6
- Minimum required Qt version is 5.12
- Add case sensitivity option in Stringmatchfilter
- cmake: replace BUILD_STATIC_LOG4CXX_LIB with standard BUILD_SHARED_LIBS

### Fixed
- TTCCLayout crash fix (#69)
- Fixes for Qt6.7
- Replace deprecated Qt 5.15

## [v1.5.1] - 14.02.2020
### Improvements
- Automatically delete files written by DailyRollingFileAppender (formerly DailyFileAppender) after a configurable period of time.

## [v1.5.0] - 25.06.2018
### Improvements
- RollingFileAppender: Changed the behavior on application restart and if
                       appendFile is set to false to avoid data loss. The
                       existing log files are rolled instead of overwritten.
                       This behavior is different to the log4/log4cpp implementation.
- Modernize c++ and qt usage wit clang-tidy and clazy

### Fixed
- Fixed "DATE" format string, it will be formatted as MMM YYYY HH:mm:ss.zzz
- Minimum required Qt version is 5.7
- Move color console appender to windows only (is only implemented for windows),
  unified windows os preprocessor switches

## [v1.4.2] - 2017-09-06
### Improvements
- Build: Use moc file includes in order to improve build performance
- Allow java - loggernames in the form 'Foo.bar' within Hierarchy::createLogger()

### Fixed
- Work around for the QFileSystemWatcher shortcome handling changes in files which
  are modified/written into a temp file, on commit the orig file is deleted and
  the temp file renamed/moved to orig file name


## [v1.4.1] - 2017-06-17
### Improvements
- Added qbs support

### Fixed
- Fixed cmake build
- Fix Patternformatter maxLength behaviour to match the log4j documentation -
  if the data item is longer than the maximum field, then the extra characters are removed from the beginning
  of the data item and not from the end.

## [v1.4.0] - 2017-05-04

### Added
- Environment variables starting with LOG4QT_ can now be used within a log4qt.properties file ${LOG4QT_...}
- Added cmake support
- Added suport for location information (%F, %L, %M, %l)
- Added support for QLoggingCategories

### Improvements
- Support static builds
- detect minimum required compiler version (log4qt depends on magic statics c++11)
- Cleanup includes in headers
- Make database and telnet appender optional dependig on Qt module sql and network

### Fixed
- Make DatabaseAppender compile

## [v1.3.0] - 2016-09-13

### Added
- Changed directory layout
- Wrapper logger object for logging from Qml (QmlLogger)
- Appender for windows debug console (OutputDebugString)
- continuous integration windows - appveyor ci config files
- Changelog
- New global boolean property for *.properties files
    log4j.watchThisFile=[true]|[false]
    If set to true an file system watcher is installed for the *.properties file
    and the loggers are automatically reconfigured if this file changes.

### Improvements
- Replaced old-c-style casts
- Searching for *.properties files are extended:
  1. If <application binaryname.exe.log4qt.properties exists this file is used (Windows only)
  2. If <application binaryname>.log4qt.properties exists this file is used
  3. If "log4qt.properties" exists in the executables path this file is used
  4. If "log4qt.properties" exists in the current working directory this file is used (the default before)

### Fixed
- Minimum required Qt version is 5.3
- Reenabled an fixed unit test suite

## [v1.2.0] - 2016-05-03

### Added
- continuous integration linux - travis ci config files

### Improvment
- Replaced object ownership via LogObject and LogObjectPtr with QSharedPointer
- Removed pre Qt5 code
- Code improvements (use c++11 feature, e.g. nulptr, range base for loop, override, ...)
- Replaced msecSinceEpoch methods of DateTime with QDateTime methodes which exists since Qt4.7
- Replaced custom DataTime Formater with QDateTime::toString() - lost additional expression week

### Fixed
- converted inlcude file from utf-16 to ascii
- DatePatternConverter: fixed scope of toString() call

## [v1.1.0] - 2015-12-22

### Added
- Binary logger
- XMLLayout to support apache chainsaw
