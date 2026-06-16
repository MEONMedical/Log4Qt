# NullAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `NullAppender` is the empty sink: it accepts every event handed to it and **discards it**, producing no output at all.

A developer uses it as a placeholder to disable an appender slot without removing it from the configuration, or as a baseline for benchmarking the logging pipeline (it isolates the cost of event creation, threshold checks, and filtering from the cost of real I/O). It is the Log4Qt equivalent of routing logging to `/dev/null`.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class).
- **Implementation includes:** `abstractlayout.h`, `loggingevent.h`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `LoggingEvent` — the event passed to `append()` (and immediately ignored).

## 3. Class Hierarchy and Role

`NullAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It overrides `requiresLayout()` and `append()`. Its role is a no-op sink.

## 4. Q_PROPERTY Declarations

None beyond those inherited from `AppenderSkeleton` (`isActive`, `isClosed`, `threshold`).

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### NullAppender(QObject *parent = nullptr)

Constructor. Chains to `AppenderSkeleton(false, parent)` — note the `false`: the appender is created **already active** through that protected base constructor's first argument (the `isActive` flag), so it requires no `activateOptions()` call to start accepting events.

#### ~NullAppender() override

Destructor. Calls `close()` to mark the appender closed, mirroring the standard appender teardown contract even though there is nothing to flush.

#### bool requiresLayout() const override

Returns `false` — no layout is needed because nothing is ever formatted or written.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Defined as a pure virtual in `AppenderSkeleton`; this override is intentionally empty. The `event` parameter is marked `[[maybe_unused]]`. Invoked from `doAppend()` under `mObjectGuard` after entry conditions, threshold, and the filter chain have passed — and then does nothing, dropping the event.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr` and managed by the logger repository.
- It holds no external resources. The destructor calls `close()` purely to satisfy the appender lifecycle contract.

## 12. Thread Safety

All public functions are thread-safe (as documented on the class). Because `append()` does no work and touches no shared state, concurrent log calls are trivially safe; they still pass through the `doAppend()` pipeline (recursion guard, atomic active/closed checks, threshold, filter chain) before reaching the empty `append()`.

## 15. External Communication

None. `NullAppender` performs no I/O of any kind.

## 16. Usage Example

```cpp
#include "log4qt/varia/nullappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

// A sink that discards everything — useful to benchmark the pipeline
// or to temporarily silence a logger without altering the rest of the config.
auto *nullAppender = new NullAppender;
nullAppender->setName(QStringLiteral("null"));

Logger::rootLogger()->addAppender(AppenderSharedPtr(nullAppender));
Logger::rootLogger()->info(QStringLiteral("this message goes nowhere"));
```
