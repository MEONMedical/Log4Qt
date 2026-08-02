# TimeBasedTriggeringPolicy

## 1. Class Overview

`TimeBasedTriggeringPolicy` triggers a rollover based on a time/date pattern. The date pattern determines the rollover **frequency** (minutely, hourly, half-daily, daily, weekly, or monthly); the policy fires when the current time passes a computed rollover time. An `interval` multiplier, calendar `modulate` alignment, and a `maxRandomDelay` jitter further shape exactly when the rollover occurs.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/timebasedtriggeringpolicy.h`
- Source: `src/log4qt/spi/timebasedtriggeringpolicy.cpp`
- Base class: `TriggeringPolicy`
- Public dependencies: `QDateTime`
- Implementation dependencies: `helpers/datetime.h` (`DateTime::currentDateTime`), `QRandomGenerator`, plus a `Qt::StringLiterals` using-directive
- Exported via the `LOG4QT_EXPORT` macro.

## 3. Class Hierarchy and Role

Derives from `TriggeringPolicy` (which derives from `QObject`). It implements `isTriggeringEvent()` by comparing the current time against an internally computed rollover time, and overrides `activateOptions()` to derive the frequency from the date pattern and compute the first rollover time.

## 4. Q_PROPERTY

| Property | Type | Access | Description |
| --- | --- | --- | --- |
| `datePattern` | `QString` | READ `datePattern` / WRITE `setDatePattern` | The date pattern that determines the rollover frequency. Default is `"'.'yyyy-MM-dd"` (daily). The frequency is derived automatically from which time fields appear in the pattern (see Frequency derivation below). |
| `interval` | `int` | READ `interval` / WRITE `setInterval` | Multiplies the base frequency unit. For example, an hourly pattern with `interval = 4` rolls every 4 hours. Default `1`. `setInterval()` clamps any value below `1` to `1` immediately, so the divisor used in rollover-time computation can never be zero or negative regardless of call order. |
| `modulate` | `bool` | READ `modulate` / WRITE `setModulate` | When `true`, rollover times are aligned to calendar boundaries derived from a fixed epoch. For example, `interval = 4` hourly aligns rollovers to 00:00, 04:00, 08:00, 12:00, 16:00, 20:00. Default `false`. |
| `maxRandomDelay` | `int` | READ `maxRandomDelay` / WRITE `setMaxRandomDelay` | Adds a uniformly random delay of `[0, maxRandomDelay]` seconds to each computed rollover time, to avoid a thundering-herd rollover across multiple processes. Default `0` (no delay). `setMaxRandomDelay()` clamps negative values to `0` immediately. |

## 5. Enumerations

| Enum | Underlying type | Values | Description |
| --- | --- | --- | --- |
| `Frequency` | `int` (`Q_ENUM`) | `Minutely = 0`, `Hourly`, `HalfDaily`, `Daily`, `Weekly`, `Monthly` | The rollover frequency derived from the date pattern. The base unit is then scaled by `interval`. |

**Frequency derivation:** `activateOptions()` tokenizes `datePattern` (honoring single-quote literal escaping and greedy runs of identical characters) and picks the *finest* time field present, in priority order: `m` -> Minutely, `h`/`H` -> Hourly, `a`/`A` -> HalfDaily, `d` -> Daily, `w` -> Weekly, `M` -> Monthly. If no recognized field is found, the active pattern is left empty and the policy never triggers.

## 6. Public Member Variables

This class declares no public member variables.

## 7. Signals

This class declares no signals.

## 8. Public Slots & Q_INVOKABLE

This class declares no public slots or Q_INVOKABLE methods.

## 9. Public Methods

#### explicit TimeBasedTriggeringPolicy(QObject *parent = nullptr)

Constructs the policy with `datePattern` set to `"'.'yyyy-MM-dd"` and `frequency` initialized to `Frequency::Daily`.

#### QString datePattern() const

Returns the configured date pattern. Marked `[[nodiscard]]`.

#### void setDatePattern(const QString &datePattern)

Sets the date pattern. The derived frequency is not recomputed until `activateOptions()` runs.

#### int interval() const

Returns the interval multiplier. Marked `[[nodiscard]]`.

#### void setInterval(int interval)

Sets the interval multiplier. Values below `1` are clamped to `1` immediately by the setter, guaranteeing the rollover-time divisor stays positive at all times.

#### bool modulate() const

Returns whether calendar-boundary alignment is enabled. Marked `[[nodiscard]]`.

#### void setModulate(bool modulate)

Enables or disables calendar-boundary alignment.

#### int maxRandomDelay() const

Returns the maximum random delay in seconds. Marked `[[nodiscard]]`.

#### void setMaxRandomDelay(int maxRandomDelay)

Sets the maximum random delay in seconds. Negative values are clamped to `0` immediately by the setter.

#### Frequency frequency() const

Returns the frequency derived from the date pattern. Meaningful only after `activateOptions()` has run. Marked `[[nodiscard]]`.

## 10. Protected Virtual Methods

This class overrides two virtuals inherited from `TriggeringPolicy` (declared public there). It does not override `isStartupTrigger()` (base returns `false`).

#### void activateOptions() override

Overrides `TriggeringPolicy::activateOptions()`. Clamps `interval` to `>= 1` and `maxRandomDelay` to `>= 0`, derives the `Frequency` from `datePattern`, and — if a recognized pattern was found — computes the first rollover time. If no recognized time field is present, the active pattern stays empty and the policy will never trigger.

#### bool isTriggeringEvent(QIODevice *activeDevice, const LoggingEvent &event) override

Overrides `TriggeringPolicy::isTriggeringEvent()`. Both arguments are unused. Returns `false` immediately if no active date pattern was derived. Otherwise, if `DateTime::currentDateTime()` is later than the stored rollover time, it advances the rollover time to the next boundary and returns `true`; otherwise returns `false`.

The next rollover time is computed from the current time according to `frequency`, `interval`, and `modulate`. With `modulate = false` the boundary is `interval` units after the start of the current period; with `modulate = true` the boundary is aligned to `interval`-sized buckets measured from a fixed epoch (year 2000 for daily/weekly/monthly; midnight of the current day/hour for sub-day frequencies). A random delay in `[0, maxRandomDelay]` seconds is then added when `maxRandomDelay > 0`.

## 11. Ownership and Lifecycle

Held by `RollingFileAppender` through a `TriggeringPolicySharedPtr` (`Log4QtSharedPtr<TriggeringPolicy>`); reference-counted ownership keeps it alive while referenced. Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 12. Thread Safety

The class documents all its functions as thread-safe.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

- `RollingFileAppender::activateOptions()` calls this policy's `activateOptions()`; `RollingFileAppender::append()` calls `isTriggeringEvent()` after each event and drives `rollOver()` on a `true` result.
- `DateTime::currentDateTime()` supplies the current time used for both the trigger comparison and rollover-time computation.
- `QRandomGenerator::global()` supplies the optional random delay.
- `Factory` registers this class under `"Log4Qt::TimeBasedTriggeringPolicy"` and `"TimeBasedTriggeringPolicy"`.

## 15. External Communication

None.

## 16. Usage Example

```cpp
using namespace Log4Qt;

auto appender = RollingFileAppenderSharedPtr(
    new RollingFileAppender(layout, u"app.log"_s));

auto timePolicy = TriggeringPolicySharedPtr(new TimeBasedTriggeringPolicy);
auto *tp = qobject_cast<TimeBasedTriggeringPolicy *>(timePolicy.data());
tp->setDatePattern(u"'.'yyyy-MM-dd-HH"_s); // hourly
tp->setInterval(4);                        // every 4 hours
tp->setModulate(true);                     // align to 00,04,08,...

appender->setTriggeringPolicy(timePolicy);
appender->activateOptions();
```
