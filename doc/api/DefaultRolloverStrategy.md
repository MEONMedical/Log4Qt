# DefaultRolloverStrategy

## 1. Class Overview

`DefaultRolloverStrategy` performs a fixed-window, numbered rotation of backup files. It is the rollover strategy `RollingFileAppender` installs automatically when no strategy is configured. When a rollover occurs it:

1. Deletes the oldest backup file (the one at `maxIndex`).
2. Shifts numbered backups up one slot: `.N` becomes `.N+1`.
3. Renames the active base file to `.minIndex`.

For a base file `app.log` with the defaults (`minIndex = 1`, `maxIndex = 7`), this produces backups `app.log.1` (newest) through `app.log.7` (oldest), discarding anything older than `.7`.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/defaultrolloverstrategy.h`
- Source: `src/log4qt/spi/defaultrolloverstrategy.cpp`

Direct dependencies:

- `RolloverStrategy` (`spi/rolloverstrategy.h`) — abstract base; provides the protected `removeFile()` / `renameFile()` helpers used here.
- `QFile` — used to test for the existence of each backup slot before renaming.

## 3. Class Hierarchy and Role

`QObject` → `RolloverStrategy` (abstract) → **`DefaultRolloverStrategy`**

A concrete `RolloverStrategy` implementing numbered-backup rotation. Non-copyable and non-movable (`Q_DISABLE_COPY_MOVE`). It overrides only `rollover()`; it uses the base-class no-op `activateOptions()` and the base-class identity `initialFileName()`.

## 4. Q_PROPERTY

| Property | Type | READ | WRITE | NOTIFY | Default | Description |
|----------|------|------|-------|--------|---------|-------------|
| `minIndex` | `int` | `minIndex()` | `setMinIndex()` | — | `1` | Lowest backup index. The active file is renamed to `base.minIndex` on rollover; this becomes the newest backup. |
| `maxIndex` | `int` | `maxIndex()` | `setMaxIndex()` | — | `7` | Highest backup index. The backup at this index is deleted on each rollover, bounding the number of retained backups. |

## 5. Enumerations

None.

## 6. Public Member Variables

None exposed. State (`mMinIndex`, `mMaxIndex`) is private and accessed through the property getters/setters below.

Public compile-time constants:

| Constant | Type | Value | Meaning |
|----------|------|-------|---------|
| `defaultMinIndex` | `static constexpr int` | `1` | Default value for `minIndex`. |
| `defaultMaxIndex` | `static constexpr int` | `7` | Default value for `maxIndex`. |

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

#### explicit DefaultRolloverStrategy(QObject *parent = nullptr)
Constructs the strategy with `minIndex = defaultMinIndex` (1) and `maxIndex = defaultMaxIndex` (7).

#### int minIndex() const
Returns the configured minimum backup index. `[[nodiscard]]`.

#### void setMinIndex(int minIndex)
Sets the minimum backup index.

#### int maxIndex() const
Returns the configured maximum backup index. `[[nodiscard]]`.

#### void setMaxIndex(int maxIndex)
Sets the maximum backup index.

#### QString rollover(const QString &fileName) override
Performs the numbered rotation described in section 1 and returns `fileName` unchanged (the appender reopens the same base name). Each rename and delete is funnelled through the inherited `RolloverStrategy::removeFile()` / `renameFile()` helpers, which log on failure. Intermediate slots are renamed only if the source file exists; the base file is renamed only if it exists (so a missing file on first startup is tolerated).

## 10. Protected Virtual Methods

No additional protected virtuals. `rollover()` is the override of the base-class pure virtual (documented above). The inherited static helpers `removeFile()` and `renameFile()` are used internally.

## 11. Ownership and Lifecycle

Held by `RollingFileAppender` through a `RolloverStrategySharedPtr` (reference-counted `Log4QtSharedPtr`). Created automatically by the appender as the fallback strategy when none is configured. Non-copyable, non-movable.

## 12. Thread Safety

The header documents all functions of this class as thread-safe. The strategy holds only two scalar index values and does not mutate them during `rollover()`; the rollover itself is serialized by the owning `RollingFileAppender`.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Installed by `RollingFileAppender` as the default strategy and invoked from `RollingFileAppender::rollOver()` after the active file is closed.
- Relies on the protected file helpers inherited from `RolloverStrategy` for all disk operations and their error reporting.

## 15. External Communication

During a rollover it deletes the oldest backup and renames a chain of backup files plus the active base file on the local filesystem.

## 16. Usage Example

```cpp
#include "log4qt/rollingfileappender.h"
#include "log4qt/spi/defaultrolloverstrategy.h"
#include "log4qt/spi/sizebasedtriggeringpolicy.h"

using namespace Log4Qt;

auto *appender = new RollingFileAppender;
appender->setFile(u"app.log"_s);

// Keep backups app.log.1 .. app.log.5; delete anything older.
auto strategy = RolloverStrategySharedPtr(new DefaultRolloverStrategy);
static_cast<DefaultRolloverStrategy *>(strategy.data())->setMinIndex(1);
static_cast<DefaultRolloverStrategy *>(strategy.data())->setMaxIndex(5);
appender->setRolloverStrategy(strategy);

// Roll when the file exceeds 10 MiB.
auto policy = TriggeringPolicySharedPtr(new SizeBasedTriggeringPolicy);
static_cast<SizeBasedTriggeringPolicy *>(policy.data())->setMaxFileSize(10 * 1024 * 1024);
appender->setTriggeringPolicy(policy);

appender->activateOptions();
```
