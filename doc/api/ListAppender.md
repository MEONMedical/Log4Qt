# ListAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `ListAppender` keeps events **in memory**: every accepted `LoggingEvent` is appended to an internal `QList<LoggingEvent>` for later retrieval and inspection rather than being written to a device.

It serves two distinct purposes:

- **Testing / inspection.** Tests attach a `ListAppender`, exercise some logging, then call `list()` or `clearList()` to assert on the captured events. Because it stores the raw `LoggingEvent` objects (not formatted strings), callers can examine level, message, logger name, timestamp, etc.
- **Configuration error capture.** When `configuratorList` is set, the appender survives `Logger::removeAllAppenders()` so it can collect events raised during the configuration process (used by the configurators and `ConfiguratorHelper`).

An optional `maxCount` bounds memory growth.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class), `loggingevent.h`, `<QList>`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `LoggingEvent` — the value type stored in the list.

## 3. Class Hierarchy and Role

`ListAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It overrides `requiresLayout()` and `append()`, and adds `ensureMaxCount()`. Its role is an in-memory accumulating sink.

## 4. Q_PROPERTY Declarations

| Property | Type | Read | Write | Default | Description |
|----------|------|------|-------|---------|-------------|
| `configuratorList` | `bool` | `configuratorList()` | `setConfiguratorList()` | `false` | Marks the appender as belonging to a configurator. When `true`, the appender is **not** removed by `Logger::removeAllAppenders()`, allowing it to collect events raised during configuration. |
| `maxCount` | `int` | `maxCount()` | `setMaxCount()` | `0` (treated as unlimited) | Maximum number of events retained. Values `<= 0` mean unlimited. A negative value passed to `setMaxCount()` is rejected with a warning and replaced by `0`. |

> Note: the header docstring describes the historical default as `-1` for unlimited, but the constructor initialises `maxCount` to `0`, and the runtime treats any value `<= 0` as unlimited.

## 5. Enumerations

None.

## 6. Public Member Variables

None public. (The list, the count, and the configurator flag are private members.)

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### ListAppender(QObject *parent = nullptr)

Constructor. Chains to `AppenderSkeleton(parent)` and initialises `configuratorList` to `false` and `maxCount` to `0` (unlimited).

#### bool configuratorList() const

Returns `true` if the appender is flagged as a configurator list (see the property table), `false` otherwise.

#### QList&lt;LoggingEvent&gt; list() const

Returns a **copy** of the currently retained events. Taken under `mObjectGuard`, so it is safe to call while other threads are logging.

#### int maxCount() const

Returns the configured maximum retained-event count (`<= 0` meaning unlimited).

#### void setConfiguratorList(bool isConfiguratorList)

Sets the configurator-list flag. When set to `true`, the appender is preserved across `Logger::removeAllAppenders()` so it can keep collecting events during configuration.

#### void setMaxCount(int n)

Sets the maximum retained-event count under `mObjectGuard`. A negative `n` is logged as a warning and clamped to `0`. After updating, `ensureMaxCount()` trims any excess events already in the list.

#### QList&lt;LoggingEvent&gt; clearList()

Atomically (under `mObjectGuard`) returns a copy of the retained events **and** empties the internal list, leaving the appender ready to collect a fresh batch.

#### bool requiresLayout() const override

Returns `false` — events are stored as objects, never formatted, so no layout is required.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Defined as a pure virtual in `AppenderSkeleton`; this override stores the event. Invoked from `doAppend()` under `mObjectGuard` after entry conditions, threshold, and the filter chain have passed. It appends `event` to the internal list **only if** the list is unbounded (`maxCount <= 0`) or has not yet reached `maxCount`; once the bound is hit, further events are simply dropped.

#### void ensureMaxCount()

Helper invoked from `setMaxCount()` (while holding `mObjectGuard`). If `maxCount > 0`, it removes events from the **front** of the list until the size is within the new limit; for an unlimited count it returns immediately.

> Note the asymmetry between the two paths: `append()` drops *new* events once full, whereas `ensureMaxCount()` (triggered by a `setMaxCount()` change) drops the *oldest* events to fit the new bound.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr` and managed by the logger repository.
- **Memory growth:** with the default `maxCount` of `0` (unlimited) the internal list grows without bound for every accepted event, and each `LoggingEvent` is retained by value. For long-running collection, set a sensible `maxCount`, or periodically drain with `clearList()`, to cap memory use.
- `LoggingEvent` objects are stored and returned by value, so callers own their own copies; clearing the appender does not invalidate previously returned lists.

## 12. Thread Safety

All public functions are thread-safe (as documented on the class). `list()`, `clearList()`, and `setMaxCount()` each take `mObjectGuard`; `append()` runs under the same guard within Phase 5 of `doAppend()`. The retained-count fields are also held as atomics, but mutation of the list itself is serialised by the mutex.

## 14. Inter-Class Interactions

- Used pervasively by the test suite and inspection code: attach, log, then assert on `list()`.
- The `configuratorList` flag ties into `BasicConfigurator`, `PropertyConfigurator`, and `ConfiguratorHelper::configureError()` — such an appender persists across `Logger::removeAllAppenders()` to capture configuration-time events.

## 16. Usage Example

```cpp
#include "log4qt/varia/listappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto *capture = new ListAppender;
capture->setName(QStringLiteral("capture"));
capture->setMaxCount(100);          // cap memory; oldest events trimmed on shrink
capture->activateOptions();

Logger::logger(QStringLiteral("Test"))->addAppender(AppenderSharedPtr(capture));
Logger::logger(QStringLiteral("Test"))->warn(QStringLiteral("something happened"));

// Inspect what was logged (e.g. in a unit test).
const QList<LoggingEvent> events = capture->list();
Q_ASSERT(events.size() == 1);
Q_ASSERT(events.first().level() == Level::WARN_INT);

// Drain and reset for the next scenario.
const QList<LoggingEvent> drained = capture->clearList();
```
