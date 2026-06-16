# DateRolloverStrategy

## 1. Class Overview

`DateRolloverStrategy` performs date-based rotation of backup files. Instead of numbered slots, each rolled-over file carries a date stamp derived from a `QDateTime::toString()` format pattern. It supports three operating modes:

- **Suffix** naming (default): the active file is renamed by appending a date suffix. `app.log` becomes `app.log.2026-03-28`. The appender re-opens the same base filename.
- **Embedded** naming: the date is embedded between the basename and extension. `app.log` becomes `app_2026-03-28.log`. The appender opens the new dated filename returned by the strategy.
- **Dated active file** (`datedActiveFile == true`): the active file *always* carries the embedded date, from the very first startup. Each time period writes directly to its own dated file, so no rename happens on rollover and the `mode` property is ignored.

Two retention controls bound how many backups are kept: `maxBackups` (count limit) and `keepDays` (age limit). When either is set, obsolete-file cleanup runs **asynchronously** on a worker thread via `QtConcurrent::run`.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/daterolloverstrategy.h`
- Source: `src/log4qt/spi/daterolloverstrategy.cpp`

Direct dependencies:

- `RolloverStrategy` (`spi/rolloverstrategy.h`) — abstract base; provides `removeFile()` / `renameFile()`.
- `DateTime` (`helpers/datetime.h`) — supplies the current timestamp used to build date stamps.
- `QDateTime`, `QDir`, `QFile`, `QFileInfo`, `QRegularExpression` — filename building, directory scanning, and date extraction.
- `QFutureSynchronizer<void>` + `QtConcurrent` (`<QtConcurrentRun>`) — async cleanup of obsolete backups.
- `<algorithm>` — sorting backups by modification time during count-based pruning.

## 3. Class Hierarchy and Role

`QObject` → `RolloverStrategy` (abstract) → **`DateRolloverStrategy`**

A concrete `RolloverStrategy` implementing date-stamped rotation. Non-copyable, non-movable (`Q_DISABLE_COPY_MOVE`). It overrides `activateOptions()`, `initialFileName()`, and `rollover()`, and adds the `waitForCleanup()` method for synchronizing pending async cleanup.

## 4. Q_PROPERTY

| Property | Type | READ | WRITE | NOTIFY | Default | Description |
|----------|------|------|-------|--------|---------|-------------|
| `datePattern` | `QString` | `datePattern()` | `setDatePattern()` | — | `"'.'yyyy-MM-dd"` | `QDateTime::toString()` format used to build the date stamp. Quoted literals (e.g. the leading `'.'`) are emitted verbatim. |
| `mode` | `QString` | `modeString()` | `setModeString()` | — | `"Suffix"` | Naming mode as a string: `"Suffix"` or `"Embedded"` (case-insensitive on write). Maps to the `NamingMode` enum. |
| `maxBackups` | `int` | `maxBackups()` | `setMaxBackups()` | — | `0` | Maximum number of backup files to keep. `0` means unlimited. |
| `keepDays` | `int` | `keepDays()` | `setKeepDays()` | — | `0` | Days to retain backups; files whose parsed date is older than today minus `keepDays` are deleted. `0` means unlimited. |
| `datedActiveFile` | `bool` | `datedActiveFile()` | `setDatedActiveFile()` | — | `false` | When `true`, the active file always has the date embedded in its name from first startup; rollover performs no rename and `mode` is ignored. |

The `NamingMode mode()` / `setMode(NamingMode)` typed accessors are also available directly (not part of the property, which uses the string variants).

## 5. Enumerations

`enum class NamingMode : int` (`Q_ENUM`)

| Value | Integer | Meaning |
|-------|---------|---------|
| `Suffix` | `0` | Date stamp is appended to the full filename (`app.log` → `app.log.2026-03-28`); the appender re-opens the same base file. This is the default. |
| `Embedded` | `1` | Date stamp is inserted between basename and extension (`app.log` → `app_2026-03-28.log`); the appender opens the new dated name. |

## 6. Public Member Variables

None exposed. All state (`mDatePattern`, `mMode`, `mMaxBackups`, `mKeepDays`, `mDatedActiveFile`, `mActiveSuffix`, `mCleanupExecutors`) is private; access is via the methods below.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

#### explicit DateRolloverStrategy(QObject *parent = nullptr)
Constructs with `datePattern = "'.'yyyy-MM-dd"`, `mode = NamingMode::Suffix`, `maxBackups = 0`, `keepDays = 0`, `datedActiveFile = false`.

#### QString datePattern() const
Returns the date format pattern. `[[nodiscard]]`.

#### void setDatePattern(const QString &datePattern)
Sets the date format pattern.

#### NamingMode mode() const
Returns the naming mode as the typed enum. `[[nodiscard]]`.

#### void setMode(NamingMode mode)
Sets the naming mode from the typed enum.

#### QString modeString() const
Returns `"Embedded"` for `NamingMode::Embedded`, otherwise `"Suffix"`. `[[nodiscard]]`. Backs the `mode` property's READ.

#### void setModeString(const QString &mode)
Sets the mode from a string; `"Embedded"` (case-insensitive) selects `NamingMode::Embedded`, anything else selects `NamingMode::Suffix`. Backs the `mode` property's WRITE.

#### int maxBackups() const
Returns the maximum number of retained backups (`0` = unlimited). `[[nodiscard]]`.

#### void setMaxBackups(int maxBackups)
Sets the maximum number of retained backups.

#### int keepDays() const
Returns the retention age in days (`0` = unlimited). `[[nodiscard]]`.

#### void setKeepDays(int keepDays)
Sets the retention age in days.

#### bool datedActiveFile() const
Returns whether the active file always carries an embedded date. `[[nodiscard]]`.

#### void setDatedActiveFile(bool dated)
Enables/disables the dated-active-file mode.

#### void activateOptions() override
Calls the base implementation, then captures the current date stamp into the internal active suffix so the first rollover names its backup after the period it belongs to.

#### QString initialFileName(const QString &fileName) const override
When `datedActiveFile` is `false`, returns `fileName` unchanged. When `true`, returns the embedded-date name for the current time so the appender opens a dated file from startup.

#### QString rollover(const QString &fileName) override
Performs the date-based rollover and returns the path the appender should open next. Behaviour by mode:

- **Dated active file:** updates the internal active suffix, schedules async cleanup, and returns the embedded-date name for the new period (no rename).
- **Suffix mode:** builds the backup name from the *previous* period's active suffix, removes any pre-existing backup of that name, renames the base file to the backup name, schedules cleanup, and returns `fileName`.
- **Embedded mode:** updates the active suffix, schedules cleanup, and returns the new embedded-date filename (the previous active file is left in place under its own dated name).

Cleanup is scheduled only when `maxBackups > 0` or `keepDays > 0`.

#### void waitForCleanup()
Blocks until all pending asynchronous cleanup tasks (submitted by previous `rollover()` calls) have finished. Useful in tests or controlled shutdown to ensure deletions complete.

## 10. Protected Virtual Methods

No new protected virtuals. The class overrides the base-class virtuals `activateOptions()`, `initialFileName()`, and the pure-virtual `rollover()` (all documented above) and reuses the inherited `removeFile()` / `renameFile()` helpers.

A file-local (anonymous-namespace) helper `deleteObsoleteFiles()` performs the actual pruning on the worker thread: it scans the directory with a name filter derived from the mode, excludes the active file, deletes files older than the `keepDays` cutoff (date parsed from the filename via a `QRegularExpression`), and, if still over `maxBackups`, deletes the oldest by modification time.

## 11. Ownership and Lifecycle

Held by `RollingFileAppender` through a `RolloverStrategySharedPtr` (reference-counted `Log4QtSharedPtr`). Non-copyable, non-movable. The embedded `QFutureSynchronizer<void>` tracks outstanding cleanup futures; its destructor blocks for any still-running cleanup, so the strategy must outlive its scheduled cleanup work (or `waitForCleanup()` should be called first).

## 12. Thread Safety

The header documents all functions of this class as thread-safe. Rollover is serialized by the owning `RollingFileAppender`. Obsolete-file cleanup runs on a separate thread via `QtConcurrent::run`; the lambda captures values by copy so the worker does not race the strategy's members. `waitForCleanup()` and the `QFutureSynchronizer` destructor join those workers.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- `RollingFileAppender::activateOptions()` calls `activateOptions()` and `initialFileName()`, allowing a dated initial filename before the file is opened.
- `RollingFileAppender::rollOver()` calls `rollover(baseName)` after closing the active file and opens whatever path is returned.
- Uses `DateTime::currentDateTime()` for all timestamps so behaviour is consistent with the rest of the library's time handling.

## 15. External Communication

During rollover it renames the active file (Suffix mode) and, asynchronously, deletes obsolete backup files on the local filesystem according to the `maxBackups` / `keepDays` retention rules.

## 16. Usage Example

```cpp
#include "log4qt/rollingfileappender.h"
#include "log4qt/spi/daterolloverstrategy.h"
#include "log4qt/spi/timebasedtriggeringpolicy.h"

using namespace Log4Qt;

auto *appender = new RollingFileAppender;
appender->setFile(u"app.log"_s);

auto *date = new DateRolloverStrategy;
date->setMode(DateRolloverStrategy::NamingMode::Embedded); // app_2026-03-28.log
date->setDatePattern(u"_yyyy-MM-dd"_s);
date->setKeepDays(30);   // delete backups older than 30 days
date->setMaxBackups(60); // and never keep more than 60 files
appender->setRolloverStrategy(RolloverStrategySharedPtr(date));

// Roll at the start of each day.
appender->setTriggeringPolicy(
    TriggeringPolicySharedPtr(new TimeBasedTriggeringPolicy));

appender->activateOptions();
```
