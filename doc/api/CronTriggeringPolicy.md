# CronTriggeringPolicy

## 1. Class Overview

`CronTriggeringPolicy` triggers a rollover according to a cron schedule, similar to log4j2's `CronTriggeringPolicy`. It delegates schedule parsing and next-fire-time computation to the `CronExpression` helper, then fires whenever the current time reaches the computed next fire time.

The cron expression uses the Quartz format with six fields: `seconds minutes hours day-of-month month day-of-week`. The default schedule is `"0 0 0 * * ?"` (midnight every day).

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/crontriggeringpolicy.h`
- Source: `src/log4qt/spi/crontriggeringpolicy.cpp`
- Base class: `TriggeringPolicy`
- Public dependencies: `log4qt/helpers/cronexpression.h` (`CronExpression`), `QDateTime`
- Implementation dependencies: `helpers/datetime.h` (`DateTime::currentDateTime`), `helpers/logerror.h`, `logger.h`, plus a `Qt::StringLiterals` using-directive
- Exported via the `LOG4QT_EXPORT` macro.

## 3. Class Hierarchy and Role

Derives from `TriggeringPolicy` (which derives from `QObject`). It implements `isTriggeringEvent()` by comparing the current time against a precomputed next fire time, and overrides `activateOptions()` to parse the cron expression and compute that fire time.

## 4. Q_PROPERTY

| Property | Type | Access | Description |
| --- | --- | --- | --- |
| `schedule` | `QString` | READ `schedule` / WRITE `setSchedule` | The Quartz-style cron expression that determines the rollover schedule. Default `"0 0 0 * * ?"` (midnight every day). |

Example schedules:

| Expression | Meaning |
| --- | --- |
| `"0 0 0 * * ?"` | Every day at midnight |
| `"0 0 0/4 * * ?"` | Every 4 hours starting at midnight |
| `"0 0 0 1 * ?"` | First day of every month at midnight |
| `"0 0 8 ? * MON-FRI"` | Weekdays at 08:00 |

## 5. Enumerations

This class declares no enumerations.

## 6. Public Member Variables

This class declares no public member variables.

## 7. Signals

This class declares no signals.

## 8. Public Slots & Q_INVOKABLE

This class declares no public slots or Q_INVOKABLE methods.

## 9. Public Methods

#### explicit CronTriggeringPolicy(QObject *parent = nullptr)

Constructs the policy with `schedule` set to the default `"0 0 0 * * ?"`.

#### QString schedule() const

Returns the configured cron expression string.

#### void setSchedule(const QString &schedule)

Sets the cron expression string. The expression is not parsed until `activateOptions()` runs.

## 10. Protected Virtual Methods

This class overrides two virtuals inherited from `TriggeringPolicy` (declared public there). It does not override `isStartupTrigger()` (base returns `false`).

#### void activateOptions() override

Overrides `TriggeringPolicy::activateOptions()`. Constructs a `CronExpression` from `schedule`. If the expression is invalid, it logs a `ConfiguratorInvalidOptionError` (with the parser's error string attached as a causing error), leaves the next fire time invalid, and returns — so the policy never triggers. If the expression is valid, it computes the next fire time from the current time.

#### bool isTriggeringEvent(QIODevice *activeDevice, const LoggingEvent &event) override

Overrides `TriggeringPolicy::isTriggeringEvent()`. Both arguments are unused. Returns `false` if the next fire time is invalid (no valid schedule). Otherwise, if `DateTime::currentDateTime()` is at or after the next fire time, it advances the next fire time via `CronExpression::nextFireTime()` and returns `true`; otherwise returns `false`.

## 11. Ownership and Lifecycle

Held by `RollingFileAppender` through a `TriggeringPolicySharedPtr` (`Log4QtSharedPtr<TriggeringPolicy>`); reference-counted ownership keeps it alive while referenced. The `CronExpression` it uses is an owned value member (not a `QObject`), constructed during `activateOptions()`. Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 12. Thread Safety

The class documents all its functions as thread-safe.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

- `RollingFileAppender::activateOptions()` calls this policy's `activateOptions()`; `RollingFileAppender::append()` calls `isTriggeringEvent()` after each event and drives `rollOver()` on a `true` result.
- `CronExpression` parses the schedule (`isValid()`, `errorString()`) and computes successive fire times (`nextFireTime()`). It supports six Quartz fields and the specifiers `*`, `,`, `-`, `/`, and `?`, with month names (`JAN`-`DEC`) and day-of-week names (`SUN`-`SAT`). `nextFireTime()` searches up to a 4-year window and returns an invalid `QDateTime` if no match is found.
- `DateTime::currentDateTime()` supplies the current time.
- `Factory` registers this class under `"Log4Qt::CronTriggeringPolicy"` and `"CronTriggeringPolicy"`.

## 15. External Communication

None.

## 16. Usage Example

```cpp
using namespace Log4Qt;

auto appender = RollingFileAppenderSharedPtr(
    new RollingFileAppender(layout, u"app.log"_s));

auto cronPolicy = TriggeringPolicySharedPtr(new CronTriggeringPolicy);
qobject_cast<CronTriggeringPolicy *>(cronPolicy.data())
    ->setSchedule(u"0 0 8 ? * MON-FRI"_s); // weekdays at 08:00

appender->setTriggeringPolicy(cronPolicy);
appender->activateOptions();
```
