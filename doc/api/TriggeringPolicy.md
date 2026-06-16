# TriggeringPolicy

## 1. Class Overview

`TriggeringPolicy` is the abstract base class for all triggering policies in Log4Qt. A *triggering policy* decides **when** a `RollingFileAppender` should roll the active log file over. The base class defines the contract — chiefly the pure virtual `isTriggeringEvent()` — that every concrete policy implements with its own decision logic (size threshold, time/date boundary, cron schedule, application startup, or an OR-combination of several).

It is inspired by log4j2's `TriggeringPolicy` interface.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/triggeringpolicy.h`
- Source: `src/log4qt/spi/triggeringpolicy.cpp`
- Base class: `QObject`
- Public dependencies: `log4qt/log4qt.h`, `log4qt/log4qtsharedptr.h`, `QObject`
- Forward declarations: `QIODevice`, `Log4Qt::LoggingEvent`
- Exported via the `LOG4QT_EXPORT` macro.

The header defines the type alias `TriggeringPolicySharedPtr` used throughout the library:

| Alias | Definition |
| --- | --- |
| `TriggeringPolicySharedPtr` | `Log4QtSharedPtr<TriggeringPolicy>` |

## 3. Class Hierarchy and Role

`TriggeringPolicy` derives from `QObject` and is the root of the policy hierarchy:

- `TriggeringPolicy` (abstract base)
  - `SizeBasedTriggeringPolicy` — rolls at a byte threshold
  - `TimeBasedTriggeringPolicy` — rolls on a date/time pattern boundary
  - `CronTriggeringPolicy` — rolls on a cron schedule
  - `OnStartupTriggeringPolicy` — rolls once when the appender activates
  - `CompositeTriggeringPolicy` — OR-combines several child policies

The base declares the single mandatory decision point, `bool isTriggeringEvent(QIODevice *, const LoggingEvent &)`, as pure virtual; subclasses must override it. The base also provides two non-pure virtuals with default behaviour: `activateOptions()` (no-op) and `isStartupTrigger()` (returns `false`).

## 4. Q_PROPERTY

This class declares no Q_PROPERTY members.

## 5. Enumerations

This class declares no enumerations.

## 6. Public Member Variables

This class declares no public member variables.

## 7. Signals

This class declares no signals.

## 8. Public Slots & Q_INVOKABLE

This class declares no public slots or Q_INVOKABLE methods.

## 9. Public Methods

#### explicit TriggeringPolicy(QObject *parent = nullptr)

Constructs a triggering policy with an optional parent `QObject`.

#### ~TriggeringPolicy() override

Destroys the policy. Defaulted.

## 10. Protected Virtual Methods

`TriggeringPolicy` declares no protected members; its virtuals are public. They are documented here as the overridable contract shared by all subclasses.

#### virtual void activateOptions()

Applies and validates configuration after all properties have been set. The base implementation is a no-op. `RollingFileAppender::activateOptions()` invokes this on its configured policy. Subclasses override it to precompute internal schedule state (for example `TimeBasedTriggeringPolicy` computes its frequency and first rollover time; `CronTriggeringPolicy` parses its expression and computes the next fire time).

#### virtual bool isTriggeringEvent(QIODevice *activeDevice, const LoggingEvent &event) = 0

Pure virtual. Returns `true` if a rollover should be triggered after `event` was written. Called by `RollingFileAppender::append()` on every logged event.

- `activeDevice` — the underlying `QIODevice` of the active log file. Policies that need the file size (e.g. `SizeBasedTriggeringPolicy`) call `activeDevice->pos()`, which reads a cached value without a syscall. All other policies ignore this parameter.
- `event` — the logging event that was just written.

Each subclass defines its own decision: size threshold exceeded, computed rollover time passed, cron fire time reached, etc.

#### virtual bool isStartupTrigger(const QString &fileName, qint64 fileSize)

Returns `true` if a rollover should be triggered at startup (when the appender activates). The base implementation returns `false`, so by default policies do not roll on startup. `OnStartupTriggeringPolicy` overrides this to return `true` when the file already exists and is non-empty.

- `fileName` — the current active log file path.
- `fileSize` — the current file size in bytes.

## 11. Ownership and Lifecycle

Policies are reference-counted via `Log4QtSharedPtr<TriggeringPolicy>` (`TriggeringPolicySharedPtr`). A `RollingFileAppender` holds its policy through this shared pointer (`setTriggeringPolicy()` / `triggeringPolicy()`), so the policy lives as long as the appender (or any other holder) references it. Although `TriggeringPolicy` is a `QObject` accepting a `parent`, the intended lifetime model is the shared-pointer / ref-counted ownership documented for the library.

Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 12. Thread Safety

The base class itself is stateless beyond `QObject`. Thread-safety guarantees are made per subclass: `SizeBasedTriggeringPolicy`, `TimeBasedTriggeringPolicy` and `CronTriggeringPolicy` document all their functions as thread-safe. The appender that drives policies (`RollingFileAppender`) serializes access to its policy under its own mutex.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

- `RollingFileAppender` owns one `TriggeringPolicy` and calls `isTriggeringEvent()` on each `append()`, calls `isStartupTrigger()` once during `activateOptions()`, and calls the policy's `activateOptions()`.
- `Factory` registers concrete leaf policies (`SizeBasedTriggeringPolicy`, `TimeBasedTriggeringPolicy`, `CronTriggeringPolicy`, `OnStartupTriggeringPolicy`) under both qualified and unqualified class names for configuration-driven creation.
- `CompositeTriggeringPolicy` aggregates other `TriggeringPolicy` instances and forwards the three virtuals to each child.

## 15. External Communication

None.
