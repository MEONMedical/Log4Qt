# DenyAllFilter

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. A *filter* inspects each `LoggingEvent` as it passes through an appender's filter chain and returns a `Decision` of `Accept`, `Deny`, or `Neutral`. `DenyAllFilter` unconditionally returns `Deny` for every event.

It is the canonical "stop here" terminator placed at the **end** of a filter chain: preceding filters can `Accept` the events they want, and any event that reaches `DenyAllFilter` (i.e. no earlier filter accepted it) is dropped. This implements an allow-list policy — accept the explicitly wanted events, deny everything else.

## 2. Project Structure and Dependencies

- **Header includes:** `spi/filter.h` (base class).
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `Filter` — the base class providing the chain and the `Decision` enum.
  - `LoggingEvent` — the event evaluated (ignored here).

## 3. Class Hierarchy and Role

`DenyAllFilter` inherits **`Filter`** (→ `QObject`), gaining the meta-object system, parent-based ownership, the `next` chain property, and the `Decision` enum. It overrides only `decide()`. Its role is a chain terminator that rejects everything.

## 4. Q_PROPERTY Declarations

None beyond the `next` property inherited from `Filter`.

## 5. Enumerations

`DenyAllFilter` returns the inherited **`Filter::Decision`** enum. Its `decide()` always returns `Filter::Deny`; it never returns `Accept` or `Neutral`. The enum values are:

| Value | Meaning |
|-------|---------|
| `Filter::Accept` | Log the event immediately, without consulting the remaining filters. |
| `Filter::Deny` | Drop the event immediately, without consulting the remaining filters. |
| `Filter::Neutral` | This filter has no opinion; defer to the remaining filters in the chain. |

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### DenyAllFilter(QObject *parent = nullptr)

Constructor. Chains to `Filter(parent)`. The filter is stateless and needs no configuration.

#### Decision decide(const LoggingEvent &event) const override

Always returns `Filter::Deny`. The `event` parameter is marked `[[maybe_unused]]`; the decision does not depend on it. Defined inline in the header.

## 10. Protected Virtual Methods

None of its own. `decide(const LoggingEvent&)` is the pure-virtual hook declared by the `Filter` base class; `DenyAllFilter` provides the override described above (it is `public`, not protected).

## 11. Ownership and Lifecycle

- The filter is a `QObject`; a `parent` deletes it. In normal use it is held via `FilterSharedPtr` and attached to an appender with `Appender::addFilter()`.
- Filters are *chained*: each filter holds a `next` pointer, and the appender walks the chain in order. `DenyAllFilter` is normally the final link.
- It holds no external resources and no mutable state.

## 12. Thread Safety

The filter is stateless and its `decide()` is a pure function returning a constant, so it is inherently safe to evaluate from multiple threads concurrently. The `Filter` base serialises mutation of the `next` pointer with its own mutex.

## 14. Inter-Class Interactions

- Plugs into `AppenderSkeleton`'s filter chain (added via `addFilter()`); `doAppend()` evaluates the chain before calling `append()`.
- Typically paired with one or more accepting filters (e.g. `LevelMatchFilter`, `LevelRangeFilter`, `StringMatchFilter`) placed ahead of it to form an allow-list.

## 16. Usage Example

```cpp
#include "log4qt/varia/levelmatchfilter.h"
#include "log4qt/varia/denyallfilter.h"
#include "log4qt/consoleappender.h"

using namespace Log4Qt;

auto *appender = new ConsoleAppender;

// Allow-list: accept ERROR, deny everything else.
auto matchError = FilterSharedPtr(new LevelMatchFilter);
static_cast<LevelMatchFilter *>(matchError.data())->setLevelToMatch(Level::ERROR_INT);
static_cast<LevelMatchFilter *>(matchError.data())->setAcceptOnMatch(true);

appender->addFilter(matchError);
appender->addFilter(FilterSharedPtr(new DenyAllFilter)); // terminator
```
