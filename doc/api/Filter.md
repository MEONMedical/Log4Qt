# Filter

## 1. Class Overview

`Filter` is the abstract base class of the appender filter chain. Each appender may hold a singly-linked chain of filters that are consulted, in order, before a logging event is appended. Every filter returns one of three decisions for a given event — **Accept**, **Deny**, or **Neutral** — which controls whether the event is logged immediately, dropped immediately, or passed on to the next filter in the chain.

Concrete filters subclass `Filter` and implement `decide()`. Examples shipped with Log4Qt include `DenyAllFilter`, `LevelMatchFilter`, `LevelRangeFilter`, and `StringMatchFilter` (in `src/log4qt/varia/`).

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/filter.h`
- Source: `src/log4qt/spi/filter.cpp`

Direct dependencies:

- `QObject` — base class.
- `QMutex` / `QMutexLocker` — guard the `next` link for thread-safe access.
- `LoggingEvent` (forward-declared) — the event passed to `decide()`.
- `Log4QtSharedPtr<Filter>` (`log4qtsharedptr.h`) — managed shared pointer for chain links; aliased as `FilterSharedPtr`.

## 3. Class Hierarchy and Role

`QObject` → **`Filter`** (abstract)

`Filter` is an abstract `QObject` base. Its role is twofold: it defines the filter decision contract (`decide()`), and it maintains the forward link (`next`) that forms the filter chain. `AppenderSkeleton` walks this chain during `doAppend()`.

## 4. Q_PROPERTY

| Property | Type | READ | WRITE | NOTIFY | Default | Description |
|----------|------|------|-------|--------|---------|-------------|
| `next` | `FilterSharedPtr` | `next()` | `setNext()` | — | null (no next filter) | The next filter in the chain. A null pointer terminates the chain. Both accessors are mutex-guarded. |

## 5. Enumerations

`enum Decision : int` (`Q_ENUM`)

| Value | Integer | Meaning |
|-------|---------|---------|
| `Accept` | `0` | The event must be logged immediately, without consulting any remaining filters in the chain. |
| `Deny` | `1` | The event must be dropped immediately, without consulting any remaining filters in the chain. |
| `Neutral` | `2` | This filter is neutral about the event; the remaining filters (if any) should be consulted for the final decision. |

## 6. Public Member Variables

None exposed. The chain link (`mNext`) and its guard (`mNextGuard`) are private; access is via the methods below.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

#### Filter(QObject *parent = nullptr)
Constructs a filter with an optional QObject parent and a null `next` link.

#### virtual ~Filter()
Destroys the filter. Defaulted in the source; declared virtual for safe polymorphic deletion.

#### FilterSharedPtr next() const
Returns the next filter in the chain (or null). Thread-safe — acquires the internal mutex. `[[nodiscard]]`.

#### void setNext(const FilterSharedPtr &filter)
Sets the next filter in the chain. Thread-safe — acquires the internal mutex.

#### virtual void activateOptions()
Applies configuration after properties are set. The default implementation is a no-op. Subclasses override it to validate/precompute configured options.

#### virtual Decision decide(const LoggingEvent &event) const = 0
Pure virtual. Returns this filter's decision for `event`: `Accept`, `Deny`, or `Neutral`. Declared `const` and `[[nodiscard]]`, so an appender can call it concurrently while holding no lock. See section 10 for how the chain interprets the result.

## 10. Protected Virtual Methods

There are no `protected` virtuals; the overridable contract is the public pure-virtual `decide()` (documented above) and the virtual `activateOptions()`.

The `decide()` override defines a single filter's verdict. The chain semantics — implemented by the caller (`AppenderSkeleton`) — are: walk filters in order, stop and append on the first `Accept`, stop and drop on the first `Deny`, and continue to the next filter on `Neutral`. If the chain is exhausted with only `Neutral` results, the event is appended by default.

## 11. Ownership and Lifecycle

Filters are held through `FilterSharedPtr` (reference-counted `Log4QtSharedPtr`). A filter owns its `next` filter in the reference-counting sense: holding a `FilterSharedPtr` to the head keeps the whole chain alive, since each filter retains a shared pointer to the next. There is no QObject parent-tree ownership of the chain link. Care should be taken not to create a cycle in the `next` links, which would leak the chain.

## 12. Thread Safety

The `next` accessors (`next()` / `setNext()`) are individually thread-safe via an internal `QMutex`. `decide()` is `const` and is invoked by `AppenderSkeleton` outside the appender's object lock, so concrete implementations must be safe to call concurrently and should treat their state as read-only during a decision. The base class makes no broader atomicity guarantee about reconfiguring the chain while it is being walked.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- `AppenderSkeleton::doAppend()` snapshots the head filter under its object lock, then walks the chain *outside* the lock (because `decide()` is `const`): on `Accept` it breaks and proceeds to formatting/I-O, on `Deny` it returns without logging, and on `Neutral` it advances via `next().data()`.
- Concrete filters in `src/log4qt/varia/` (`DenyAllFilter`, `LevelMatchFilter`, `LevelRangeFilter`, `StringMatchFilter`) implement the actual decision logic.

## 15. External Communication

None. Filtering is a pure in-process decision; the base class performs no I/O.
