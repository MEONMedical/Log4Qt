# CronExpression

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging library. Several of its rolling-file features need to roll over (start a new log file) on a schedule. `CronExpression` is the helper that parses a Quartz-style cron string and answers the question "when does this schedule next fire?".

A developer reaches for `CronExpression` when a time-based schedule must be expressed declaratively in configuration. The class parses the expression once, validates it, and then repeatedly computes the next matching `QDateTime` at or after a given instant. In Log4Qt it is the engine behind `CronTriggeringPolicy`, which uses it to decide when a `RollingFileAppender` should roll over.

## 2. Project Structure and Dependencies

- **Used by:** `CronTriggeringPolicy` (`spi/crontriggeringpolicy.h` / `.cpp`). The policy stores a `CronExpression` built from its configured schedule string, checks `isValid()` (logging `errorString()` on failure), and calls `nextFireTime()` against `DateTime::currentDateTime()` to compute the next rollover instant.
- **Qt module dependency:** Qt Core — uses `QDateTime`, `QDate`, `QTime`, `QString`, and `QStringList`.
- **Standard library:** `<bitset>` for compact per-field match sets.
- **Project headers:** `log4qt/log4qt.h`, which pulls in `log4qtshared.h` for the `LOG4QT_EXPORT` macro. The `.cpp` imports `Qt::StringLiterals` for the `u"..."_s` literal helper.
- **Build requirement:** part of the `log4qt` target; sources `helpers/cronexpression.cpp` and `helpers/cronexpression.h` are listed in `src/log4qt/CMakeLists.txt`. The class is exported from the shared library via `LOG4QT_EXPORT`.

## 3. Class Hierarchy and Role

`CronExpression` is a standalone value class with no base class. It is not a `QObject`; it has no meta-object, signals, or slots. It is copyable and assignable using the compiler-generated members (it stores only `std::bitset` fields, two booleans/strings, and no resources), which is why `CronTriggeringPolicy` can assign a freshly parsed expression into a member with `mCronExpression = CronExpression(mSchedule)`.

### Supported cron syntax

The expression has **6 space-separated fields**, in order:

| Position | Field | Range | Names accepted |
|----------|-------|-------|----------------|
| 1 | seconds | 0–59 | — |
| 2 | minutes | 0–59 | — |
| 3 | hours | 0–23 | — |
| 4 | day-of-month | 1–31 | — |
| 5 | month | 1–12 | `JAN`–`DEC` (case-insensitive) |
| 6 | day-of-week | 1–7 (1 = Sunday) | `SUN`–`SAT` (case-insensitive) |

Each field supports these specifiers:

- `*` — every value in the field's range.
- `?` — treated the same as `*` (the Quartz "no specific value" placeholder).
- `,` — a list, e.g. `1,5,10`.
- `-` — an inclusive range, e.g. `1-5`.
- `/` — a step, applied to `*`, to a range (`1-30/5`), or to a single start value (`5/10` meaning "from 5 to the field maximum, every 10").

Example: `0 0 0 * * ?` fires at midnight every day.

## 4. Q_PROPERTY Declarations

None.

## 5. Enumerations

None.

## 6. Public Member Variables

None. All state (the six `std::bitset` match sets, `mValid`, `mErrorString`) is private and skipped.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### CronExpression() = default

Constructs an empty, **invalid** expression. `isValid()` returns `false` and `nextFireTime()` returns an invalid `QDateTime`. Used as the default member state before a real expression is assigned.

#### explicit CronExpression(const QString &expression)

Parses `expression` immediately and records the result. The expression is split on whitespace (after `simplified()`) and must yield exactly 6 fields. Month and day-of-week names are substituted for their numbers before parsing. If parsing succeeds, `isValid()` returns `true`; otherwise `isValid()` is `false` and `errorString()` describes the failure (wrong field count, invalid step, invalid range, invalid value, or out-of-range value).

#### bool isValid() const

Returns `true` if the expression parsed successfully and can be evaluated. Always check this before relying on `nextFireTime()`.

#### QString errorString() const

Returns a human-readable description of why parsing failed, or an empty string if the expression is valid. `CronTriggeringPolicy` forwards this into its error log when configuration is rejected.

#### QDateTime nextFireTime(const QDateTime &from) const

Returns the next instant **strictly after** `from` (search begins at `from` + 1 second, with the sub-second part cleared) at which all six fields of the expression match. The search advances field by field — skipping whole months, days, hours, or minutes that cannot match — and is bounded to a 4-year window from `from` to guarantee termination. Returns an **invalid** `QDateTime` if the expression is invalid or if no match is found within that window.

Day-of-week matching converts Qt's `dayOfWeek()` (1 = Monday … 7 = Sunday) to the Quartz convention (1 = Sunday … 7 = Saturday) before testing. Note that day-of-month and day-of-week are treated as an **AND** condition: both must match on the candidate date.

## 10. Protected Virtual Methods / Event Handlers

None.

## 11. Ownership and Lifecycle

`CronExpression` is a lightweight value type with **value semantics**: no heap ownership, no parent, no manual cleanup. It is copied and assigned freely (e.g. constructed from a string and assigned into a `CronTriggeringPolicy` member). All evaluation state is computed once during construction; `nextFireTime()` is a `const` query that does not mutate the object.

## 12. Thread Safety

After construction the object is **immutable** — all match sets and the validity flag are fixed by the constructor. `isValid()`, `errorString()`, and `nextFireTime()` are all `const` and perform no shared mutation, so a single `CronExpression` instance can be queried concurrently from multiple threads without external synchronisation. Concurrent *construction or assignment* of the same instance is not protected; callers that re-parse must serialise that themselves (as `CronTriggeringPolicy` does within `activateOptions()`).

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- **`CronTriggeringPolicy`** is the primary client. In `activateOptions()` it constructs a `CronExpression` from its `schedule` string, logs a configuration error via `errorString()` when invalid, and in `computeNextFireTime()` calls `nextFireTime(DateTime::currentDateTime())` to obtain the next rollover time. When the expression is invalid the policy leaves its next-fire time unset so it never triggers.
- Indirectly cooperates with `DateTime` (the `from` argument typically originates from `DateTime::currentDateTime()`).

## 15. External Communication

None. It performs pure in-process date arithmetic.

## 16. Usage Example

```cpp
#include "helpers/cronexpression.h"
#include <QDateTime>

using namespace Log4Qt;

// Fire at midnight every day.
CronExpression cron(u"0 0 0 * * ?"_s);
if (!cron.isValid())
{
    qWarning() << "Bad cron:" << cron.errorString();
    return;
}

QDateTime now  = QDateTime::currentDateTime();
QDateTime next = cron.nextFireTime(now);   // next midnight strictly after now
if (next.isValid())
    scheduleRolloverAt(next);

// Every 15 minutes during business hours, Mon-Fri:
CronExpression office(u"0 0/15 9-17 ? * MON-FRI"_s);
```
