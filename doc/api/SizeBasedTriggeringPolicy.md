# SizeBasedTriggeringPolicy

## 1. Class Overview

`SizeBasedTriggeringPolicy` triggers a rollover when the active log file exceeds a configured maximum size in bytes. It is the simplest and most common policy: roll the file when it grows past a byte threshold.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/sizebasedtriggeringpolicy.h`
- Source: `src/log4qt/spi/sizebasedtriggeringpolicy.cpp`
- Base class: `TriggeringPolicy`
- Implementation dependencies: `helpers/optionconverter.h` (`OptionConverter::toFileSize`), `logger.h`, `QIODevice`
- Exported via the `LOG4QT_EXPORT` macro.

## 3. Class Hierarchy and Role

Derives from `TriggeringPolicy` (which derives from `QObject`). It implements the abstract `isTriggeringEvent()` decision by comparing the current write position of the active device against the configured maximum file size.

## 4. Q_PROPERTY

| Property | Type | Access | Description |
| --- | --- | --- | --- |
| `maximumFileSize` | `qint64` | READ `maximumFileSize` / WRITE `setMaximumFileSize` | The maximum file size in bytes. Default is `10 MB` (`10 * 1024 * 1024`). Values `<= 0` are rejected. |
| `maxFileSize` | `QString` | READ `maxFileSize` / WRITE `setMaxFileSize` | Sets/reads the maximum file size as a string with unit suffixes (e.g. `"10MB"`, `"500KB"`). The string form is parsed via `OptionConverter::toFileSize`. The reader returns the byte count as a plain number string. |

Both properties manipulate the same underlying byte value; `maxFileSize` is a string-convenience facade over `maximumFileSize`.

## 5. Enumerations

This class declares no enumerations.

## 6. Public Member Variables

This class declares no public member variables. It exposes a public static constant:

| Member | Type | Value | Description |
| --- | --- | --- | --- |
| `defaultMaximumFileSize` | `static constexpr qint64` | `10 * 1024 * 1024` | The default maximum file size (10 MB) used when none is configured. |

## 7. Signals

This class declares no signals.

## 8. Public Slots & Q_INVOKABLE

This class declares no public slots or Q_INVOKABLE methods.

## 9. Public Methods

#### explicit SizeBasedTriggeringPolicy(QObject *parent = nullptr)

Constructs the policy with `maximumFileSize` initialized to `defaultMaximumFileSize` (10 MB).

#### qint64 maximumFileSize() const

Returns the current maximum file size in bytes. Marked `[[nodiscard]]`.

#### void setMaximumFileSize(qint64 maximumFileSize)

Sets the maximum file size in bytes. If `maximumFileSize <= 0`, the call logs a warning and retains the current value (the invalid value is ignored).

#### QString maxFileSize() const

Returns the current maximum file size as a plain decimal byte-count string (`QString::number(...)`). Marked `[[nodiscard]]`.

#### void setMaxFileSize(const QString &maxFileSize)

Parses `maxFileSize` with `OptionConverter::toFileSize` (accepting unit suffixes such as `KB`, `MB`, `GB`). On a successful parse, forwards the resulting byte count to `setMaximumFileSize()`; on parse failure the value is left unchanged.

## 10. Protected Virtual Methods

This class overrides one virtual inherited from `TriggeringPolicy` (declared public there, not protected). It does **not** override `activateOptions()` (base no-op) or `isStartupTrigger()` (base returns `false`).

#### bool isTriggeringEvent(QIODevice *activeDevice, const LoggingEvent &event) override

Overrides `TriggeringPolicy::isTriggeringEvent()`. Returns `true` when `activeDevice` is non-null and `activeDevice->pos() > maximumFileSize`, i.e. the current write position has grown past the threshold. The `event` argument is unused. Reading `pos()` is cheap (a cached value, no syscall), so this check is inexpensive to run on every append.

## 11. Ownership and Lifecycle

Held by `RollingFileAppender` through a `TriggeringPolicySharedPtr` (`Log4QtSharedPtr<TriggeringPolicy>`); reference-counted ownership keeps it alive while the appender references it. Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 12. Thread Safety

The class documents all its functions as thread-safe.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

- `RollingFileAppender::append()` calls `isTriggeringEvent()` after each event, passing `writer()->device()` as `activeDevice`; a `true` result drives `rollOver()`.
- `OptionConverter::toFileSize()` performs string-to-byte parsing for the `maxFileSize` property.
- `Factory` registers this class under `"Log4Qt::SizeBasedTriggeringPolicy"` and `"SizeBasedTriggeringPolicy"` for configuration-driven creation.

## 15. External Communication

None.

## 16. Usage Example

```cpp
using namespace Log4Qt;

auto appender = RollingFileAppenderSharedPtr(
    new RollingFileAppender(layout, u"app.log"_s));

auto sizePolicy = TriggeringPolicySharedPtr(new SizeBasedTriggeringPolicy);
qobject_cast<SizeBasedTriggeringPolicy *>(sizePolicy.data())
    ->setMaxFileSize(u"10MB"_s);

appender->setTriggeringPolicy(sizePolicy);
appender->activateOptions();
```
