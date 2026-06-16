# LevelRangeFilter

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. A *filter* inspects each `LoggingEvent` as it passes through an appender's filter chain and returns a `Decision` of `Accept`, `Deny`, or `Neutral`. `LevelRangeFilter` checks whether the event's level falls within an inclusive `[levelMin, levelMax]` band.

Events **outside** the band are always `Deny`-ed. Events **inside** the band are either `Accept`-ed (when `acceptOnMatch` is `true`) or passed through as `Neutral` (when `acceptOnMatch` is `false`), letting the rest of the chain decide. This implements "log only events between WARN and ERROR", "drop everything below INFO", and similar level-window policies.

## 2. Project Structure and Dependencies

- **Header includes:** `spi/filter.h` (base class), `level.h`.
- **Implementation includes:** `loggingevent.h`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `Filter` — the base class providing the chain and the `Decision` enum.
  - `Level` — the range bounds and the compared `event.level()`.
  - `LoggingEvent` — the event evaluated.

## 3. Class Hierarchy and Role

`LevelRangeFilter` inherits **`Filter`** (→ `QObject`), gaining the meta-object system, parent-based ownership, the `next` chain property, and the `Decision` enum. It overrides `decide()` and adds three configurable properties. Its role is a level-range (band) filter.

## 4. Q_PROPERTY Declarations

| Property | Type | Read | Write | Default | Description |
|----------|------|------|-------|---------|-------------|
| `acceptOnMatch` | `bool` | `acceptOnMatch()` | `setAcceptOnMatch()` | `true` | What an in-range event yields: `true` returns `Accept`, `false` returns `Neutral`. |
| `levelMin` | `Log4Qt::Level` | `levelMin()` | `setLevelMin()` | `Level::NULL_INT` | Lower (inclusive) bound. An event whose level is **below** this is denied. The `NULL_INT` default is the lowest possible level, so by default nothing is excluded from below. |
| `levelMax` | `Log4Qt::Level` | `levelMax()` | `setLevelMax()` | `Level::OFF_INT` | Upper (inclusive) bound. An event whose level is **above** this is denied. The `OFF_INT` default is the highest possible level, so by default nothing is excluded from above. |

> With the defaults (`levelMin = NULL_INT`, `levelMax = OFF_INT`) the band spans every level, so every event is in range.

## 5. Enumerations

`LevelRangeFilter::decide()` returns the inherited **`Filter::Decision`** enum:

| Value | Meaning |
|-------|---------|
| `Filter::Accept` | Log the event immediately, without consulting the remaining filters. |
| `Filter::Deny` | Drop the event immediately, without consulting the remaining filters. |
| `Filter::Neutral` | No opinion; defer to the remaining filters in the chain. |

## 6. Public Member Variables

None public. (`acceptOnMatch`, `levelMin`, and `levelMax` are private, exposed through the accessors above.)

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### LevelRangeFilter(QObject *parent = nullptr)

Constructor. Chains to `Filter(parent)` and initialises `acceptOnMatch` to `true`, `levelMin` to `Level::NULL_INT`, and `levelMax` to `Level::OFF_INT`.

#### bool acceptOnMatch() const

Returns whether an in-range event yields `Accept` (`true`) or `Neutral` (`false`). Marked `[[nodiscard]]`.

#### Level levelMax() const

Returns the upper (inclusive) bound. Marked `[[nodiscard]]`.

#### Level levelMin() const

Returns the lower (inclusive) bound. Marked `[[nodiscard]]`.

#### void setAcceptOnMatch(bool accept)

Sets the in-range decision direction.

#### void setLevelMax(Level level)

Sets the upper bound.

#### void setLevelMin(Level level)

Sets the lower bound.

#### Decision decide(const LoggingEvent &event) const override

Returns the chain decision for `event` — see Protected Virtual Methods for the exact logic.

## 10. Protected Virtual Methods

#### Decision decide(const LoggingEvent &event) const override

`decide(const LoggingEvent&)` is the pure-virtual hook declared by `Filter`; this override (declared `public`) implements:

- If `event.level() < levelMin` → return `Filter::Deny` (below the band).
- If `event.level() > levelMax` → return `Filter::Deny` (above the band).
- Otherwise the event is in range:
  - if `acceptOnMatch` is `true` → return `Filter::Accept`;
  - else → return `Filter::Neutral`.

Note the asymmetry with `LevelMatchFilter`: an in-range event with `acceptOnMatch == false` yields `Neutral` (not `Deny`), so the remaining filters still get a say.

## 11. Ownership and Lifecycle

- The filter is a `QObject`; a `parent` deletes it. In normal use it is held via `FilterSharedPtr` and attached to an appender with `Appender::addFilter()`.
- Filters are *chained* via the inherited `next` property; the appender walks the chain in order until a filter returns `Accept` or `Deny`.
- It holds no external resources.

## 12. Thread Safety

`decide()` reads three plain members and is `const`; it performs no locking. The bounds are intended to be configured before the filter is put into service, after which evaluation is read-only. Concurrent reconfiguration while logging is not synchronised by this class.

## 14. Inter-Class Interactions

- Plugs into `AppenderSkeleton`'s filter chain (`addFilter()`); `doAppend()` consults the chain before `append()`.
- Frequently used standalone (deny out-of-band events, accept in-band) or combined with other filters in sequence.

## 16. Usage Example

```cpp
#include "log4qt/varia/levelrangefilter.h"
#include "log4qt/consoleappender.h"

using namespace Log4Qt;

auto *appender = new ConsoleAppender;

// Accept events from WARN up to ERROR (inclusive); deny anything outside.
auto *filter = new LevelRangeFilter;
filter->setLevelMin(Level::WARN_INT);
filter->setLevelMax(Level::ERROR_INT);
filter->setAcceptOnMatch(true);

appender->addFilter(FilterSharedPtr(filter));
```
