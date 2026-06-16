# OnStartupTriggeringPolicy

## 1. Class Overview

`OnStartupTriggeringPolicy` triggers a rollover exactly once, at application startup, if the active log file already exists and is non-empty. It never triggers on subsequent logged events. This is useful for keeping each run's log in its own rolled file.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/onstartuptriggeringpolicy.h`
- Source: `src/log4qt/spi/onstartuptriggeringpolicy.cpp`
- Base class: `TriggeringPolicy`
- Implementation dependencies: `QFileInfo`
- Exported via the `LOG4QT_EXPORT` macro.

## 3. Class Hierarchy and Role

Derives from `TriggeringPolicy` (which derives from `QObject`). It is the only built-in policy that derives its behaviour from the startup hook: it overrides `isStartupTrigger()` (the base returns `false`) and makes `isTriggeringEvent()` always return `false`.

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

#### explicit OnStartupTriggeringPolicy(QObject *parent = nullptr)

Constructs the policy. No configuration is required.

## 10. Protected Virtual Methods

This class overrides two virtuals inherited from `TriggeringPolicy` (declared public there). It does not override `activateOptions()` (base no-op).

#### bool isTriggeringEvent(QIODevice *activeDevice, const LoggingEvent &event) override

Overrides `TriggeringPolicy::isTriggeringEvent()`. Both arguments are unused. Always returns `false` — this policy never triggers on a logged event; its only trigger path is at startup.

#### bool isStartupTrigger(const QString &fileName, qint64 fileSize) override

Overrides `TriggeringPolicy::isStartupTrigger()` (which returns `false` by default). The `fileSize` argument is unused; instead the method constructs a `QFileInfo` for `fileName` and returns `true` when the file exists and its size is greater than zero. Called once by `RollingFileAppender::activateOptions()` to decide whether to roll the pre-existing log on startup.

## 11. Ownership and Lifecycle

Held by `RollingFileAppender` through a `TriggeringPolicySharedPtr` (`Log4QtSharedPtr<TriggeringPolicy>`); reference-counted ownership keeps it alive while referenced. Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 12. Thread Safety

This class declares no internal mutable state, so its operations are effectively reentrant. The startup check is invoked once during the appender's own (mutex-guarded) `activateOptions()`.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

- `RollingFileAppender::activateOptions()` calls `isStartupTrigger()` *before* opening the file (because opening may truncate it). When `true`, the appender optionally suppresses the next footer (`skipFooterOnStartup`) and performs the rollover.
- `RollingFileAppender::append()` calls `isTriggeringEvent()` per event, which always returns `false` here.
- `Factory` registers this class under `"Log4Qt::OnStartupTriggeringPolicy"` and `"OnStartupTriggeringPolicy"`.

## 15. External Communication

None.

## 16. Usage Example

```cpp
using namespace Log4Qt;

auto appender = RollingFileAppenderSharedPtr(
    new RollingFileAppender(layout, u"app.log"_s));

appender->setTriggeringPolicy(
    TriggeringPolicySharedPtr(new OnStartupTriggeringPolicy));
// Optional: keep the previous file's footer intact on a startup roll.
appender->setSkipFooterOnStartup(true);

appender->activateOptions(); // rolls now if app.log already exists & is non-empty
```
