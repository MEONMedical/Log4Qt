# StringMatchFilter

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. A *filter* inspects each `LoggingEvent` as it passes through an appender's filter chain and returns a `Decision` of `Accept`, `Deny`, or `Neutral`. `StringMatchFilter` tests whether the event's rendered message **contains** a configured substring (`stringToMatch`), with configurable case sensitivity.

On a match it returns `Accept` or `Deny` depending on `acceptOnMatch`; otherwise (empty message, empty pattern, or no substring hit) it returns `Neutral`, deferring to the rest of the chain. This implements policies such as "accept only messages mentioning 'timeout'" or "drop everything containing 'heartbeat'".

## 2. Project Structure and Dependencies

- **Header includes:** `spi/filter.h` (base class).
- **Implementation includes:** `loggingevent.h`.
- **Qt module:** Qt Core only.
- **Project-internal types:**
  - `Filter` — the base class providing the chain and the `Decision` enum.
  - `LoggingEvent` — the event evaluated (`event.message()` is searched).
- **Qt types:** `QString` (the pattern and message), `Qt::CaseSensitivity` (match mode).

## 3. Class Hierarchy and Role

`StringMatchFilter` inherits **`Filter`** (→ `QObject`), gaining the meta-object system, parent-based ownership, the `next` chain property, and the `Decision` enum. It overrides `decide()` and adds three configurable properties. Its role is a message-substring filter.

## 4. Q_PROPERTY Declarations

| Property | Type | Read | Write | Default | Description |
|----------|------|------|-------|---------|-------------|
| `acceptOnMatch` | `bool` | `acceptOnMatch()` | `setAcceptOnMatch()` | `true` | Direction of the decision on a match: `true` returns `Accept`, `false` returns `Deny`. |
| `stringToMatch` | `QString` | `stringToMatch()` | `setStringToMatch()` | empty | The substring searched for in `event.message()`. While empty, the filter matches nothing and always returns `Neutral`. |
| `caseSensitivity` | `Qt::CaseSensitivity` | `caseSensitivity()` | `setCaseSensitivity()` | `Qt::CaseSensitive` | Whether the substring search is case-sensitive. |

## 5. Enumerations

`StringMatchFilter::decide()` returns the inherited **`Filter::Decision`** enum:

| Value | Meaning |
|-------|---------|
| `Filter::Accept` | Log the event immediately, without consulting the remaining filters. |
| `Filter::Deny` | Drop the event immediately, without consulting the remaining filters. |
| `Filter::Neutral` | No opinion; defer to the remaining filters in the chain. |

The match mode reuses the standard Qt `Qt::CaseSensitivity` enum (`Qt::CaseSensitive`, `Qt::CaseInsensitive`).

## 6. Public Member Variables

None public. (`acceptOnMatch`, `stringToMatch`, and `caseSensitivity` are private, exposed through the accessors above.)

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### StringMatchFilter(QObject *parent = nullptr)

Constructor. Chains to `Filter(parent)`. Members default via in-class initialisers: `acceptOnMatch = true`, `stringToMatch` empty, `caseSensitivity = Qt::CaseSensitive`.

#### bool acceptOnMatch() const

Returns whether a match yields `Accept` (`true`) or `Deny` (`false`). Marked `[[nodiscard]]`.

#### QString stringToMatch() const

Returns the substring searched for. Marked `[[nodiscard]]`.

#### Qt::CaseSensitivity caseSensitivity() const

Returns the configured case-sensitivity mode. Marked `[[nodiscard]]`.

#### void setAcceptOnMatch(bool accept)

Sets the on-match direction.

#### void setStringToMatch(const QString &string)

Sets the pattern, leaving the case sensitivity unchanged. Per the in-source note, this single-argument form deliberately preserves case-sensitive matching for backward compatibility — opt into case-insensitive matching via the two-argument overload or the `caseSensitivity` property.

#### void setStringToMatch(const QString &string, Qt::CaseSensitivity cs)

Sets the pattern **and** the case-sensitivity mode together.

#### void setCaseSensitivity(Qt::CaseSensitivity cs)

Sets the case-sensitivity mode independently.

#### Decision decide(const LoggingEvent &event) const override

Returns the chain decision for `event` — see Protected Virtual Methods for the exact logic.

## 10. Protected Virtual Methods

#### Decision decide(const LoggingEvent &event) const override

`decide(const LoggingEvent&)` is the pure-virtual hook declared by `Filter`; this override (declared `public`) implements:

- If `event.message()` is empty, **or** `stringToMatch` is empty, **or** `event.message()` does not contain `stringToMatch` (using the configured `caseSensitivity`) → return `Filter::Neutral`.
- Otherwise (the message contains the substring):
  - if `acceptOnMatch` is `true` → return `Filter::Accept`;
  - else → return `Filter::Deny`.

## 11. Ownership and Lifecycle

- The filter is a `QObject`; a `parent` deletes it. In normal use it is held via `FilterSharedPtr` and attached to an appender with `Appender::addFilter()`.
- Filters are *chained* via the inherited `next` property; the appender walks the chain in order until a filter returns `Accept` or `Deny`.
- It holds no external resources.

## 12. Thread Safety

`decide()` reads the plain members and is `const`; it performs no locking. The pattern and case-sensitivity are intended to be configured before the filter is put into service, after which evaluation is read-only. Concurrent reconfiguration while logging is not synchronised by this class.

## 14. Inter-Class Interactions

- Plugs into `AppenderSkeleton`'s filter chain (`addFilter()`); `doAppend()` consults the chain before `append()`.
- Often paired with `DenyAllFilter` as a terminator (accept matching messages, deny the rest), or combined with level filters in sequence.

## 16. Usage Example

```cpp
#include "log4qt/varia/stringmatchfilter.h"
#include "log4qt/varia/denyallfilter.h"
#include "log4qt/consoleappender.h"

using namespace Log4Qt;

auto *appender = new ConsoleAppender;

// Accept only messages that mention "timeout" (case-insensitive); deny the rest.
auto *filter = new StringMatchFilter;
filter->setStringToMatch(QStringLiteral("timeout"), Qt::CaseInsensitive);
filter->setAcceptOnMatch(true);

appender->addFilter(FilterSharedPtr(filter));
appender->addFilter(FilterSharedPtr(new DenyAllFilter)); // drop non-matching events
```
