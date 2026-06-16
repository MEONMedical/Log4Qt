# LevelMatchFilter

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. A *filter* inspects each `LoggingEvent` as it passes through an appender's filter chain and returns a `Decision` of `Accept`, `Deny`, or `Neutral`. `LevelMatchFilter` compares the event's level against a single configured level (`levelToMatch`) and acts only on an exact match.

On a match it returns `Accept` or `Deny` depending on `acceptOnMatch`; on a non-match (or when no level is configured) it returns `Neutral`, deferring to the rest of the chain. This makes it the building block for "accept only DEBUG", "drop only WARN", and similar single-level policies.

## 2. Project Structure and Dependencies

- **Header includes:** `spi/filter.h` (base class), `level.h`.
- **Implementation includes:** `loggingevent.h`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `Filter` — the base class providing the chain and the `Decision` enum.
  - `Level` — the level value matched against; default `Level::NULL_INT`.
  - `LoggingEvent` — the event evaluated (`event.level()` is compared).

## 3. Class Hierarchy and Role

`LevelMatchFilter` inherits **`Filter`** (→ `QObject`), gaining the meta-object system, parent-based ownership, the `next` chain property, and the `Decision` enum. It overrides `decide()` and adds two configurable properties. Its role is a single-level match filter.

## 4. Q_PROPERTY Declarations

| Property | Type | Read | Write | Default | Description |
|----------|------|------|-------|---------|-------------|
| `acceptOnMatch` | `bool` | `acceptOnMatch()` | `setAcceptOnMatch()` | `true` | Direction of the decision on a match: `true` returns `Accept`, `false` returns `Deny`. |
| `levelToMatch` | `Log4Qt::Level` | `levelToMatch()` | `setLevelToMatch()` | `Level::NULL_INT` | The exact level an event must have to match. While left at `NULL_INT`, the filter matches nothing and always returns `Neutral`. |

## 5. Enumerations

`LevelMatchFilter::decide()` returns the inherited **`Filter::Decision`** enum:

| Value | Meaning |
|-------|---------|
| `Filter::Accept` | Log the event immediately, without consulting the remaining filters. |
| `Filter::Deny` | Drop the event immediately, without consulting the remaining filters. |
| `Filter::Neutral` | No opinion; defer to the remaining filters in the chain. |

## 6. Public Member Variables

None public. (`acceptOnMatch` and `levelToMatch` are private, exposed through the accessors above.)

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### LevelMatchFilter(QObject *parent = nullptr)

Constructor. Chains to `Filter(parent)` and initialises `acceptOnMatch` to `true` and `levelToMatch` to `Level::NULL_INT`.

#### bool acceptOnMatch() const

Returns whether a match yields `Accept` (`true`) or `Deny` (`false`). Marked `[[nodiscard]]`.

#### Level levelToMatch() const

Returns the level the filter matches against. Marked `[[nodiscard]]`.

#### void setAcceptOnMatch(bool accept)

Sets the on-match direction.

#### void setLevelToMatch(Level level)

Sets the level to match. Setting it to a real level (other than `NULL_INT`) activates the filter.

#### Decision decide(const LoggingEvent &event) const override

Returns the chain decision for `event` — see Protected Virtual Methods for the exact logic.

## 10. Protected Virtual Methods

#### Decision decide(const LoggingEvent &event) const override

`decide(const LoggingEvent&)` is the pure-virtual hook declared by `Filter`; this override (declared `public`) implements:

- If `levelToMatch == Level::NULL_INT` **or** `event.level() != levelToMatch` → return `Filter::Neutral` (no match; defer).
- Otherwise (exact level match):
  - if `acceptOnMatch` is `true` → return `Filter::Accept`;
  - else → return `Filter::Deny`.

## 11. Ownership and Lifecycle

- The filter is a `QObject`; a `parent` deletes it. In normal use it is held via `FilterSharedPtr` and attached to an appender with `Appender::addFilter()`.
- Filters are *chained* via the inherited `next` property; the appender walks the chain in order until a filter returns `Accept` or `Deny`.
- It holds no external resources.

## 12. Thread Safety

`decide()` reads the two plain members and is `const`; it performs no locking. The properties are intended to be configured before the filter is put into service (during configuration / `activateOptions()`), after which evaluation is read-only. Concurrent reconfiguration while logging is not synchronised by this class.

## 14. Inter-Class Interactions

- Plugs into `AppenderSkeleton`'s filter chain (`addFilter()`); `doAppend()` consults the chain before `append()`.
- Commonly combined with `DenyAllFilter` as a terminator to form an allow-list, or with other level/string filters in sequence.

## 16. Usage Example

```cpp
#include "log4qt/varia/levelmatchfilter.h"
#include "log4qt/consoleappender.h"

using namespace Log4Qt;

auto *appender = new ConsoleAppender;

// Accept only events whose level is exactly INFO; stay Neutral otherwise.
auto *filter = new LevelMatchFilter;
filter->setLevelToMatch(Level::INFO_INT);
filter->setAcceptOnMatch(true);

appender->addFilter(FilterSharedPtr(filter));
```
