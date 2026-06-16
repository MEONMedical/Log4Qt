# DailyRollingFileAppender

## 1. Class Overview

`DailyRollingFileAppender` extends `RollingFileAppender` to produce a separate log file per day. The active file name embeds the current date (formatted with a configurable `datePattern`), and a new file is started whenever the calendar date changes. Optionally, log files older than `keepDays` days are deleted automatically.

Internally it does not implement date logic itself; instead it configures a `DateRolloverStrategy` (in **Embedded** naming mode) as the inherited rollover strategy and triggers a rollover on the first event of each new day. The date is therefore embedded directly into the active file name (e.g. `app_2026_05_30.log`) rather than applied as a suffix on rotation.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/dailyrollingfileappender.h`
- Source: `src/log4qt/dailyrollingfileappender.cpp`

Direct dependencies:

- `RollingFileAppender` (`rollingfileappender.h`) — base class; supplies `rollOver()`, the strategy plumbing, and the `FileAppender` file machinery.
- `DateRolloverStrategy` (`spi/daterolloverstrategy.h`) — performs the date-based file naming and old-file cleanup.
- `DateTime` helper (`helpers/datetime.h`) — provides the current date used for the day-change check.
- `LoggingEvent` — inspected on append.

## 3. Class Hierarchy and Role

`QObject` → … → `FileAppender` → `RollingFileAppender` → **`DailyRollingFileAppender`**

It specialises `RollingFileAppender` for the common "one file per day" case, hiding the triggering-policy/strategy composition behind the simple `datePattern` and `keepDays` properties. The day-change detection is performed directly in `append()` rather than through a `TriggeringPolicy`; the inherited rollover strategy is forced to a `DateRolloverStrategy`.

## 4. Q_PROPERTY

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `datePattern` | `QString` | `datePattern()` | `setDatePattern()` | — | Date pattern embedded into the file name, in `QDateTime::toString()` format. Default `"_yyyy_MM_dd"`. An empty value passed to the constructor falls back to the default. |
| `keepDays` | `int` | `keepDays()` | `setKeepDays()` | — | Number of days that old log files are kept on disk. A positive value enables automatic deletion; the obsolete-file check runs once per day. Default `0` (keep all files). |

Inherited: `skipFooterOnStartup` (from `RollingFileAppender`); `appendFile`, `bufferedIo`, `file` (from `FileAppender`).

## 5. Enumerations

None declared on this class. (The naming mode used internally is `DateRolloverStrategy::NamingMode`, fixed to `Embedded`.)

## 6. Public Member Variables

None. State (`mDatePattern`, `mLastDate`, `mKeepDays`, `mOriginalFilename`) is private.

## 7. Signals

None declared beyond those inherited.

## 8. Public Slots & Q_INVOKABLE

None declared. Configuration is via the property setters below.

## 9. Public Methods

#### DailyRollingFileAppender(QObject *parent = nullptr)
Constructs an appender with the default date pattern `"_yyyy_MM_dd"` and `keepDays = 0`.

#### DailyRollingFileAppender(const LayoutSharedPtr &layout, const QString &fileName, const QString &datePattern = QString(), int keepDays = 0, QObject *parent = nullptr)
Constructs an appender with the given layout and file name. An empty `datePattern` falls back to the default `"_yyyy_MM_dd"`; `keepDays` sets the retention window.

#### ~DailyRollingFileAppender()
Destructor. If the active strategy is a `DateRolloverStrategy`, calls its `waitForCleanup()` so that pending asynchronous old-file deletions complete *before* the strategy is destroyed. This is necessary because `deleteLater()` defers `QObject` destruction and would not synchronously drain the strategy's `QFutureSynchronizer`.

#### QString datePattern() const
Returns the configured date pattern. Thread-safe.

#### void setDatePattern(const QString &datePattern)
Sets the date pattern. Thread-safe. Takes full effect at the next `activateOptions()`.

#### int keepDays() const
Returns the retention window in days. Thread-safe.

#### void setKeepDays(int keepDays)
Sets the retention window. Thread-safe.

#### void activateOptions()
Captures the original (un-dated) base file name into `mOriginalFilename` on first activation, then constructs and installs a fresh `DateRolloverStrategy` in **Embedded** mode configured with the current `datePattern` and `keepDays`. It records today's date in `mLastDate`, closes any open file, sets the active file to the strategy's dated name (`strategy->rollover(mOriginalFilename)`), waits for any cleanup, and finally chains to `FileAppender::activateOptions()` to open the dated file. Thread-safe. Overrides `RollingFileAppender::activateOptions()`.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event)
Compares the current date (`DateTime::currentDateTime().date()`) with `mLastDate`. On a date change it updates `mLastDate`, resets the active file to `mOriginalFilename` (so the strategy receives the un-dated base name), and calls the inherited `rollOver()` to roll into the new day's file. It then writes the event via `FileAppender::append()`. Overrides `RollingFileAppender::append()`.

Note: unlike the base `RollingFileAppender::append()`, this override drives rollover from the in-line date check rather than from a `TriggeringPolicy`.

## 11. Ownership and Lifecycle

The `DateRolloverStrategy` is owned through the inherited `RolloverStrategySharedPtr` (set via `setRolloverStrategy()` in `activateOptions()`); ownership is shared by reference count. Because the strategy schedules asynchronous cleanup tasks (tracked by a `QFutureSynchronizer`), the destructor explicitly calls `waitForCleanup()` while the strategy is still alive to avoid losing or racing those tasks. `mOriginalFilename` preserves the user-configured base name across rollovers so each roll starts from the un-dated path.

Note: the original filename is captured only once (guarded by `mOriginalFilename.isEmpty()`); changing the inherited `file` property after the first `activateOptions()` does not change the recorded base name.

## 12. Thread Safety

All public functions are thread-safe. `datePattern()`/`setDatePattern()`, `keepDays()`/`setKeepDays()`, and `activateOptions()` lock the inherited recursive `mObjectGuard` mutex. `append()` runs inside the locked `doAppend()` path; its call to the inherited `rollOver()` relies on the recursive mutex to permit the nested close/open. The destructor's `waitForCleanup()` blocks until async deletions finish.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- **DateRolloverStrategy (`spi/`)**: the engine behind this appender. In `Embedded` mode it builds the dated active file name and, when `keepDays`/`maxBackups` apply, deletes obsolete dated files (asynchronously). This appender forces `Embedded` mode and forwards `datePattern` and `keepDays` to it.
- **RollingFileAppender (base)**: supplies `rollOver()` (close → strategy `rollover()` → reopen) and the strategy storage. `DailyRollingFileAppender` reuses that mechanism but supplies its own day-change trigger instead of a `TriggeringPolicy`.
- **Contrast with size-based rolling**: a plain `RollingFileAppender` rolls on a `TriggeringPolicy` (e.g. `SizeBasedTriggeringPolicy` for `maxFileSize`) using a `DefaultRolloverStrategy` that rotates numbered backups (`.1`..`.N`, the classic `maxBackupIndex`). `DailyRollingFileAppender` instead rolls on the calendar date and names files by embedding the date, with `keepDays`-based age cleanup rather than a fixed backup-index window.

## 15. External Communication

File I/O is inherited from `FileAppender`. Each day's events go to a distinct date-named file; rollover closes the previous day's file and opens the new one. Old-file deletion is performed by the `DateRolloverStrategy` (potentially on background threads), drained at destruction. No network or IPC.

## 16. Usage Example

```cpp
#include "log4qt/dailyrollingfileappender.h"
#include "log4qt/patternlayout.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(u"%d{ISO8601} [%t] %-5p %c - %m%n"_s));
layout->activateOptions();

// Writes app_2026_05_30.log today, app_2026_05_31.log tomorrow, etc.
// Files older than 14 days are deleted automatically.
auto *appender = new DailyRollingFileAppender(
        layout, u"app.log"_s, u"_yyyy_MM_dd"_s, /*keepDays*/ 14);
appender->setAppendFile(true);
appender->activateOptions();
// ... attach to a Logger and log; a new file starts on each date change.
```
