# RollingFileAppender

## 1. Class Overview

`RollingFileAppender` extends `FileAppender` to roll over (rotate) the active log file based on configurable **triggering policies** and **rollover strategies**. It separates the two orthogonal concerns of log rotation:

- A **TriggeringPolicy** decides *when* a rollover should occur — for example when the file exceeds a size limit, at a time interval, or once at application startup.
- A **RolloverStrategy** decides *how* the rollover is performed — for example numbered backup-file rotation or date-based renaming.

If no strategy is set when options are activated, a `DefaultRolloverStrategy` (fixed-window numbered rotation) is installed automatically. Multiple triggering policies can be combined with OR semantics through `addTriggeringPolicy()`.

This design is inspired by log4j2, where `RollingFileAppender` is likewise composed from a triggering policy and a rollover strategy.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/rollingfileappender.h`
- Source: `src/log4qt/rollingfileappender.cpp`

Direct dependencies:

- `FileAppender` (`fileappender.h`) — base class providing the `QTextStream`-backed file output, `file`/`appendFile`/`bufferedIo` properties, and `openFile()`/`closeFile()`.
- `TriggeringPolicy` (`spi/triggeringpolicy.h`) and `RolloverStrategy` (`spi/rolloverstrategy.h`) — the two pluggable SPI base classes.
- `CompositeTriggeringPolicy` (`spi/compositetriggeringpolicy.h`) — used internally to combine multiple policies.
- `DefaultRolloverStrategy` (`spi/defaultrolloverstrategy.h`) — installed as the fallback strategy.
- `LoggingEvent`, `AbstractLayout` — used during append and rollover logging.

## 3. Class Hierarchy and Role

`QObject` → `Appender`/`AppenderSkeleton` → `WriterAppender` → `FileAppender` → **`RollingFileAppender`**

`RollingFileAppender` is itself the base class of `DailyRollingFileAppender`. Its role is to intercept the append path: after delegating the actual write to `FileAppender::append()`, it consults the triggering policy and, when triggered, invokes `rollOver()`, which closes the file, asks the strategy to rotate, and reopens.

## 4. Q_PROPERTY

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `skipFooterOnStartup` | `bool` | `skipFooterOnStartup()` | `setSkipFooterOnStartup()` | — | When `true` and a triggering policy fires on startup, the layout footer is *not* written to the previous log file before rolling over. Useful when the footer is a structural delimiter (e.g. `"]"` for `JsonLayout`) that should only appear in normally-closed files. Default `false`. |

Inherited from `FileAppender`: `appendFile`, `bufferedIo`, `file`.

## 5. Enumerations

None. `RollingFileAppender` declares no `Q_ENUM`. (The numbered/date rotation enumerations live on the strategy classes — see `DateRolloverStrategy::NamingMode`.)

## 6. Public Member Variables

None. All state (`mTriggeringPolicy`, `mRolloverStrategy`, `mBaseFileName`, `mActiveFileName`, `mSkipFooterOnStartup`) is private and accessed through the methods below. `mBaseFileName` is the configured, untransformed name that rollovers operate on; `mActiveFileName` is the name the appender itself last installed (the strategy's initial name, or the result of the most recent rollover) and exists to tell a strategy-transformed `file()` apart from one the user reconfigured.

## 7. Signals

None declared beyond those inherited from the appender hierarchy.

## 8. Public Slots & Q_INVOKABLE

None declared. Configuration is performed through the ordinary setters below and through the inherited property system.

## 9. Public Methods

#### void setTriggeringPolicy(const TriggeringPolicySharedPtr &policy)
Replaces the current triggering policy with `policy`. Thread-safe (guarded by the object mutex).

#### void addTriggeringPolicy(const TriggeringPolicySharedPtr &policy)
Adds a triggering policy using OR semantics. If no policy is set, `policy` becomes the active policy. If the active policy is already a `CompositeTriggeringPolicy`, `policy` is added to it. Otherwise the existing policy and `policy` are wrapped together in a newly created `CompositeTriggeringPolicy`. Thread-safe.

#### TriggeringPolicySharedPtr triggeringPolicy() const
Returns the currently configured triggering policy (possibly a composite). Thread-safe.

#### void setRolloverStrategy(const RolloverStrategySharedPtr &strategy)
Replaces the current rollover strategy with `strategy`. Thread-safe.

#### RolloverStrategySharedPtr rolloverStrategy() const
Returns the currently configured rollover strategy. May be null before `activateOptions()` installs the default. Thread-safe.

#### bool skipFooterOnStartup() const
Returns the `skipFooterOnStartup` property value.

#### void setSkipFooterOnStartup(bool skip)
Sets the `skipFooterOnStartup` property.

#### void activateOptions()
Finalises configuration and opens the file. In order it:

1. Installs a `DefaultRolloverStrategy` if no strategy was set.
2. Activates the triggering policy (if any) and the strategy.
3. Records the configured base filename in `mBaseFileName`, so rollovers always operate on the original name and a strategy never sees an already-transformed filename. On **re-activation** the base name is only re-read from `file()` when the user actually changed it (`file() != mActiveFileName`) — otherwise `file()` still holds the strategy-transformed active name from the previous activation or rollover, and deriving the base from it would stack the transformation (`app.2026-08-01.2026-08-01.log`).
4. Asks the strategy for an `initialFileName()` (e.g. a date-embedded name) and switches to it *before* opening, so the correct name is used from the very first startup; the result is remembered as `mActiveFileName`.
5. Evaluates `isStartupTrigger()` on the policy **before** `FileAppender::activateOptions()` opens the file (because opening may truncate it).
6. Calls `FileAppender::activateOptions()`. When a startup rollover is pending, the first open is forced into **append** mode regardless of the configured `appendFile` — the previous run's file still has to be archived by `rollOver()`, and truncating it here would destroy exactly the content being archived. The configured value is restored immediately afterwards.
7. If a startup rollover was requested, optionally suppresses the next footer (per `skipFooterOnStartup`) and calls `rollOver()`.

Overrides `FileAppender::activateOptions()`. Thread-safe.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event)
Writes the event through `FileAppender::append()`, then asks the triggering policy whether this event should trigger a rollover via `isTriggeringEvent(writer()->device(), event)`. If so, calls `rollOver()`. The policy reads the file position from the active device (a cached value, no syscall) for size-based decisions. Overrides `FileAppender::append()`.

#### void rollOver()
Performs the rollover. It logs the strategy class name at debug level, closes the file (`closeFile()`), computes the base name (`mBaseFileName`, or the current `file()` if empty), invokes `mRolloverStrategy->rollover(baseName)` to obtain the next file path, switches to that path if it differs from the current one, records it as `mActiveFileName`, and reopens via `FileAppender::openFile()`.

The reopen deliberately overrides `appendFile` when the file to be opened **still exists**: either a rename/remove failed during the rollover (e.g. another process holds the file open) or the strategy intentionally reuses a dated file for the current period. In both cases the content has not been archived, so the file is opened in append mode instead of being truncated; the configured `appendFile` value is restored afterwards. Only when the path is genuinely free does the normal (possibly truncating) open apply.

This is `virtual` so subclasses (e.g. `DailyRollingFileAppender`) can extend rollover behaviour.

## 11. Ownership and Lifecycle

The triggering policy and rollover strategy are held by value as shared pointers (`TriggeringPolicySharedPtr` / `RolloverStrategySharedPtr`, which are `Log4QtSharedPtr` aliases). The appender therefore *shares* ownership of these objects; their lifetime is managed by reference counting rather than Qt parent ownership. When `addTriggeringPolicy()` wraps existing policies in a `CompositeTriggeringPolicy`, the composite takes shared ownership of the contained policies.

The appender itself follows the library's managed-ownership convention (see the `Ownership` documentation): it may be parented to a `QObject` or held via the appender shared-pointer machinery.

## 12. Thread Safety

All public functions are thread-safe. State is guarded by the inherited recursive object mutex `mObjectGuard` (a `QMutex`). `setTriggeringPolicy()`, `addTriggeringPolicy()`, `triggeringPolicy()`, `setRolloverStrategy()`, `rolloverStrategy()`, and `activateOptions()` all lock this mutex. `append()` runs inside the locked `doAppend()` path of the appender skeleton, and `rollOver()` is invoked from within that locked context, so the recursive mutex permits the nested file open/close operations.

## 13. QML Exposure

Not registered for QML. No `QML_ELEMENT`/`qmlRegisterType` exists for this class.

## 14. Inter-Class Interactions

- **TriggeringPolicy (`spi/`)**: pluggable *when* component. Concrete policies include `SizeBasedTriggeringPolicy` (rolls when the file exceeds `maximumFileSize` / `maxFileSize`, default 10 MB), `TimeBasedTriggeringPolicy`, `CronTriggeringPolicy`, `OnStartupTriggeringPolicy` (rolls once at startup if the file exists and is non-empty), and `CompositeTriggeringPolicy` (OR-combines several policies). The appender calls `isTriggeringEvent()` per append and `isStartupTrigger()` once during activation.
- **RolloverStrategy (`spi/`)**: pluggable *how* component. `DefaultRolloverStrategy` shifts numbered backups (`.N` → `.N+1`) within a fixed window (`minIndex` default 1, `maxIndex` default 7) and renames the base file to `.minIndex`. `DateRolloverStrategy` renames/embeds dates instead. The appender calls `initialFileName()` during activation and `rollover()` on each roll.
- Together these reproduce the classic log4j size/backup-index model: pair a `SizeBasedTriggeringPolicy` (the "maxFileSize") with a `DefaultRolloverStrategy` (the "maxBackupIndex" via `maxIndex`).

## 15. External Communication

File I/O is delegated entirely to the `FileAppender` base (a buffered or unbuffered `QFile` exposed through a `QTextStream`). `rollOver()` closes and reopens that file around the strategy's rename/delete operations on the backup files. No network or IPC is involved.

## 16. Usage Example

```cpp
#include "log4qt/rollingfileappender.h"
#include "log4qt/spi/sizebasedtriggeringpolicy.h"
#include "log4qt/spi/onstartuptriggeringpolicy.h"
#include "log4qt/spi/defaultrolloverstrategy.h"
#include "log4qt/patternlayout.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(u"%d{ISO8601} [%t] %-5p %c - %m%n"_s));
layout->activateOptions();

auto *appender = new RollingFileAppender(layout, u"app.log"_s, /*append*/ true);

// WHEN to roll: size limit (10 MB) OR once at startup.
auto sizePolicy = TriggeringPolicySharedPtr(new SizeBasedTriggeringPolicy);
qobject_cast<SizeBasedTriggeringPolicy *>(sizePolicy.data())->setMaxFileSize(u"10MB"_s);
appender->addTriggeringPolicy(sizePolicy);
appender->addTriggeringPolicy(TriggeringPolicySharedPtr(new OnStartupTriggeringPolicy));

// HOW to roll: keep app.log.1 .. app.log.5
auto strategy = new DefaultRolloverStrategy;
strategy->setMaxIndex(5);
appender->setRolloverStrategy(RolloverStrategySharedPtr(strategy));

appender->activateOptions();
// ... attach to a Logger and log; rollovers happen automatically.
```
