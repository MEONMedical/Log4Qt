# CompositeTriggeringPolicy

## 1. Class Overview

`CompositeTriggeringPolicy` combines multiple triggering policies using **OR** logic: a rollover is triggered if *any* contained child policy returns `true`. It lets a single `RollingFileAppender` roll on more than one condition — for example, roll when the file exceeds a size **or** when a daily boundary is reached.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/compositetriggeringpolicy.h`
- Source: `src/log4qt/spi/compositetriggeringpolicy.cpp`
- Base class: `TriggeringPolicy`
- Public dependencies: `QList`
- Exported via the `LOG4QT_EXPORT` macro.

## 3. Class Hierarchy and Role

Derives from `TriggeringPolicy` (which derives from `QObject`) and is declared **`final`** — it is not an extension point. It implements all three policy virtuals — `activateOptions()`, `isTriggeringEvent()`, and `isStartupTrigger()` — by forwarding to each child policy in turn and OR-combining their results. `final` also protects the unsynchronised child-list contract (see Thread Safety) from being subverted by a subclass.

`RollingFileAppender` constructs a `CompositeTriggeringPolicy` automatically when `addTriggeringPolicy()` is called more than once: the first added policy is stored directly; a second add wraps the existing policy and the new one in a composite; further adds append to the existing composite.

## 4. Q_PROPERTY

This class declares no Q_PROPERTY members.

## 5. Enumerations

This class declares no enumerations.

## 6. Public Member Variables

This class declares no public member variables. The contained children are held in a private `QList<TriggeringPolicySharedPtr>` and exposed via `policies()`.

## 7. Signals

This class declares no signals.

## 8. Public Slots & Q_INVOKABLE

This class declares no public slots or Q_INVOKABLE methods.

## 9. Public Methods

#### explicit CompositeTriggeringPolicy(QObject *parent = nullptr)

Constructs an empty composite policy (no children).

#### void addPolicy(const TriggeringPolicySharedPtr &policy)

Appends a child policy to the composite. The child is retained via its shared pointer.

#### QList<TriggeringPolicySharedPtr> policies() const

Returns a copy of the list of child policies (in insertion order).

## 10. Protected Virtual Methods

This class overrides all three virtuals inherited from `TriggeringPolicy` (declared public there).

#### void activateOptions() override

Overrides `TriggeringPolicy::activateOptions()`. Calls `activateOptions()` on every child policy, allowing each to precompute its own schedule state.

#### bool isTriggeringEvent(QIODevice *activeDevice, const LoggingEvent &event) override

Overrides `TriggeringPolicy::isTriggeringEvent()`. Calls `isTriggeringEvent(activeDevice, event)` on each child in order and returns `true` as soon as one returns `true` (short-circuit OR); returns `false` if no child triggers.

#### bool isStartupTrigger(const QString &fileName, qint64 fileSize) override

Overrides `TriggeringPolicy::isStartupTrigger()`. Calls `isStartupTrigger(fileName, fileSize)` on each child in order and returns `true` as soon as one returns `true`; returns `false` if no child triggers on startup.

## 11. Ownership and Lifecycle

The composite is itself held by `RollingFileAppender` through a `TriggeringPolicySharedPtr`. It owns its children by holding each as a `TriggeringPolicySharedPtr` in its internal list — the children are reference-counted, so they live at least as long as the composite references them (and may be shared with other holders). Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

Unlike the four concrete leaf policies, `CompositeTriggeringPolicy` is **not** registered with `Factory`; it is created programmatically (directly, or implicitly by `RollingFileAppender::addTriggeringPolicy()`).

## 12. Thread Safety

The class performs no internal locking; the child list is not independently synchronised. The contract is that the owning `RollingFileAppender` serialises all access under its own mutex: `addPolicy()` runs under that lock during configuration, and `activateOptions()` / `isTriggeringEvent()` / `isStartupTrigger()` run under the same lock during logging. Policies must therefore be added before or at activation, never concurrently with rollover evaluation. The class is `final` so this contract cannot be broken by a subclass adding unsynchronised mutation. Per-call thread-safety otherwise depends on the children (the built-in size, time, and cron policies are documented thread-safe).

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

- `RollingFileAppender::addTriggeringPolicy()` creates and populates a composite when more than one policy is added, and `qobject_cast`s the held policy to `CompositeTriggeringPolicy *` to append further children.
- During evaluation, the appender calls the composite's `isTriggeringEvent()` / `isStartupTrigger()` / `activateOptions()`, which fan out to all children.
- Children may be any `TriggeringPolicy` subclass: `SizeBasedTriggeringPolicy`, `TimeBasedTriggeringPolicy`, `CronTriggeringPolicy`, `OnStartupTriggeringPolicy`, or even nested composites.

## 15. External Communication

None.

## 16. Usage Example

```cpp
using namespace Log4Qt;

auto appender = RollingFileAppenderSharedPtr(
    new RollingFileAppender(layout, u"app.log"_s));

// Roll when EITHER condition fires (size OR daily boundary).
auto sizePolicy = TriggeringPolicySharedPtr(new SizeBasedTriggeringPolicy);
qobject_cast<SizeBasedTriggeringPolicy *>(sizePolicy.data())
    ->setMaxFileSize(u"10MB"_s);

auto timePolicy = TriggeringPolicySharedPtr(new TimeBasedTriggeringPolicy);
qobject_cast<TimeBasedTriggeringPolicy *>(timePolicy.data())
    ->setDatePattern(u"'.'yyyy-MM-dd"_s);

// Adding two policies makes the appender build a CompositeTriggeringPolicy.
appender->addTriggeringPolicy(sizePolicy);
appender->addTriggeringPolicy(timePolicy);
appender->activateOptions();

// Equivalent explicit composite:
// auto composite = new CompositeTriggeringPolicy;
// composite->addPolicy(sizePolicy);
// composite->addPolicy(timePolicy);
// appender->setTriggeringPolicy(TriggeringPolicySharedPtr(composite));
```
