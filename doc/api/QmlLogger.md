# QmlLogger

## 1. Class Overview

`QmlLogger` is a thin QML-facing wrapper around Log4Qt's C++ `Logger`. It lets QML code emit log events through the same hierarchy, appenders and layouts used by the rest of the application, without exposing the full C++ logging API. A QML developer instantiates a `Logger` element, optionally sets its `name`, `context` and `level`, and calls invokable methods such as `trace()`, `debug()`, `info()`, `error()`, `fatal()` or the generic `log()`.

The effective C++ logger name is composed from the `context` and `name` properties as `context.name` (default context `"Qml"`). If no `name` is set, the parent object's `objectName` is used. The wrapper resolves and caches the underlying `Logger` lazily on first use, re-resolving whenever `name` or `context` changes.

This class is only compiled when the library is built with `BUILD_WITH_QML_LOGGING` (which defines `LOG4QT_QML_LOGGING_SUPPORT` and links `Qt6::Qml`).

## 2. Project Structure and Dependencies

`QmlLogger` is registered into a QML module and instantiated from QML rather than from C++ application code. It wraps the C++ `Logger` (obtained via `Logger::logger()`) and translates its own `Level` enum to/from `Log4Qt::Level`.

Internal types: `Logger` and `Level` from the Log4Qt library.

- **Qt module dependencies:** Qt Core (`QObject`, `QString`, `QPointer`) and Qt Qml (`qqmlregistration.h`, the QML element machinery). `QStringBuilder` and `QTimer` are included in the implementation.
- **Build requirement:** built only under the `BUILD_WITH_QML_LOGGING` CMake option, which adds `qmllogger.cpp/.h` to the target, links `Qt${QT_VERSION_MAJOR}::Qml`, defines `LOG4QT_QML_LOGGING_SUPPORT`, and registers the QML module `org.log4qt` version 1.0 via `qt_add_qml_module` (with `NO_PLUGIN`, resource prefix `/qt/qml`). Exported via `LOG4QT_EXPORT`.

## 3. Class Hierarchy and Role

`QmlLogger` derives from `QObject`, which provides the meta-object system, signals/slots, properties, and parent-based ownership — all required for a QML-instantiable type. The class declares `Q_OBJECT`, disables copy/move with `Q_DISABLE_COPY_MOVE(QmlLogger)`, and marks itself as a QML element with `QML_NAMED_ELEMENT(Logger)`, so it appears in QML as the type `Logger`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `name` | `QString` | `name` | `setName` | `nameChanged` | The logger's short name. If empty, the parent object's `objectName` is used when resolving the logger. Changing it invalidates and re-resolves the cached underlying `Logger`. |
| `context` | `QString` | `context` | `setContext` | `contextChanged` | The logger's context/prefix, default `"Qml"`. Combined with `name` as `context.name` to form the full logger name. Changing it invalidates and re-resolves the cached logger. |
| `level` | `Level` | `level` | `setLevel` | `levelChanged` | The effective logging level of the underlying logger, expressed via the QML-exposed `Level` enum. Reading returns the underlying logger's level; writing sets it. Valid values are listed in Section 5. |

## 5. Enumerations

`Level` (declared with `Q_ENUM(Level)`) mirrors `Log4Qt::Level`'s integer values and is used by the `level` property and the `log()` invokable.

| Value | Integer | Description |
|-------|---------|-------------|
| `Null` | `Log4Qt::Level::NULL_INT` | No level set; the logger inherits its effective level from its ancestors. |
| `All` | `Log4Qt::Level::ALL_INT` | Lowest threshold; enables every event. |
| `Trace` | `Log4Qt::Level::TRACE_INT` | Finest-grained tracing messages. |
| `Debug` | `Log4Qt::Level::DEBUG_INT` | Debugging messages. |
| `Info` | `Log4Qt::Level::INFO_INT` | Informational messages. |
| `Warn` | `Log4Qt::Level::WARN_INT` | Warning messages. |
| `Error` | `Log4Qt::Level::ERROR_INT` | Error messages. |
| `Fatal` | `Log4Qt::Level::FATAL_INT` | Fatal-error messages. |
| `Off` | `Log4Qt::Level::OFF_INT` | Highest threshold; disables all logging. |

## 6. Public Member Variables

None.

## 7. Signals

#### nameChanged(const QString &name)

Emitted when the `name` property changes. The cached underlying logger has been invalidated and will be re-resolved on next use.

#### contextChanged(const QString &context)

Emitted when the `context` property changes. The cached underlying logger has been invalidated and will be re-resolved on next use.

#### levelChanged(QmlLogger::Level level)

Emitted when the `level` property changes, after the new level has been applied to the underlying logger.

## 8. Public Slots and Q_INVOKABLE Methods

The `Q_INVOKABLE` methods and public slots below are all callable from QML.

`Q_INVOKABLE` logging methods (each resolves the underlying logger and forwards the message; all are `const`):

#### Q_INVOKABLE void trace(const QString &message) const

Logs `message` at TRACE level.

#### Q_INVOKABLE void debug(const QString &message) const

Logs `message` at DEBUG level.

#### Q_INVOKABLE void info(const QString &message) const

Logs `message` at INFO level.

#### Q_INVOKABLE void error(const QString &message) const

Logs `message` at ERROR level.

#### Q_INVOKABLE void fatal(const QString &message) const

Logs `message` at FATAL level.

#### Q_INVOKABLE void log(QmlLogger::Level level, const QString &message) const

Logs `message` at the explicitly supplied `Level`. The QML enum value is converted to the corresponding `Log4Qt::Level`. (Note: there is no dedicated `warn()` invokable; use `log(Logger.Warn, ...)`.)

Public slots (property writers, callable from QML and connectable as slots):

#### void setName(const QString &name)

Sets the `name` property. If the value changes, invalidates the cached logger and emits `nameChanged`.

#### void setContext(const QString &context)

Sets the `context` property. If the value changes, invalidates the cached logger and emits `contextChanged`.

#### void setLevel(QmlLogger::Level level)

Sets the underlying logger's level. If the value differs from the current level, applies it and emits `levelChanged`.

## 9. Public Methods

#### explicit QmlLogger(QObject *parent = nullptr)

Constructs the wrapper, defaulting `context` to `"Qml"` and leaving the underlying logger unresolved (resolved lazily). `explicit` prevents implicit conversion from `QObject *`.

#### QString name() const

Returns the current `name` property value.

#### QString context() const

Returns the current `context` property value.

#### QmlLogger::Level level() const

Returns the underlying logger's current level mapped to the QML `Level` enum.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`QmlLogger` is a `QObject`. When created from QML it is owned by the QML engine / its parent item according to standard QML ownership rules, and is destroyed with its parent; when created in C++ with a `parent`, that parent deletes it. The underlying `Logger` is **not owned** by `QmlLogger` — it is held through a `QPointer<Logger>` (a non-owning, auto-nulling weak reference) and is owned by the logger repository (`Hierarchy`). The pointer is set to null whenever `name` or `context` changes and re-resolved on next use, so a `QmlLogger` can safely outlive or precede logger creation.

## 12. Thread Safety

GUI/QML-thread oriented. `QmlLogger` is a `QObject` intended to live on the thread that owns the QML engine (normally the GUI thread) and should be used from that thread, like any QML object. The underlying `Logger` and the repository it comes from are themselves thread-safe, so the actual event dispatch is safe, but the wrapper's property mutation and signal emission follow normal `QObject` threading rules.

## 13. QML Exposure

- **QML type / module:** registered as the element `Logger` (via `QML_NAMED_ELEMENT(Logger)`) in the QML module **`org.log4qt` version 1.0**, set up by `qt_add_qml_module` when `BUILD_WITH_QML_LOGGING` is enabled. The documentation comment also shows the legacy registration `qmlRegisterType<Log4Qt::QmlLogger>("org.log4qt", 1, 0, "Logger")`.
- **Properties usable from QML:** `name`, `context`, `level` (all readable, writable and bindable through their NOTIFY signals).
- **Invokables usable from QML:** `trace`, `debug`, `info`, `error`, `fatal`, and `log(level, message)`.
- **Signals usable from QML:** `nameChanged`, `contextChanged`, `levelChanged`.
- **Enum usable from QML:** `Level` (`Logger.Null`, `Logger.All`, `Logger.Trace`, `Logger.Debug`, `Logger.Info`, `Logger.Warn`, `Logger.Error`, `Logger.Fatal`, `Logger.Off`).
- **Usage constraints:** the effective logger name is `context.name`; if `name` is left empty it defaults to the parent object's `objectName`. The underlying `Logger` is shared, not owned, so multiple `QmlLogger` instances resolving the same name address the same logger.

## 14. Inter-Class Interactions

- Resolves and forwards to a C++ `Logger` obtained via `Logger::logger()`, which lives in the repository owned by `LogManager` / `Hierarchy`.
- Translates between its QML `Level` enum and `Log4Qt::Level` when reading/writing the level and when calling `log()`.
- Emits property-change signals that QML bindings can observe.

## 15. External Communication

None. Output destinations are determined by the appenders attached to the underlying logger, not by `QmlLogger` itself.

## 16. Usage Example

QML usage of the registered `Logger` element:

```qml
import QtQuick
import org.log4qt 1.0

Item {
    objectName: "MainView"

    // Defaults to logger name "Qml.MainView" (context "Qml" + parent objectName).
    Logger {
        id: logger
        name: "MainView"     // -> logger name "Qml.MainView"
        level: Logger.Debug
    }

    Component.onCompleted: {
        logger.info("Component completed.")
        logger.log(Logger.Warn, "Heads up: using the generic log() for warn.")
    }
}
```
