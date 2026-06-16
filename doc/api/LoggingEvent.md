# LoggingEvent

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. When a `Logger` decides a message should be logged, it packages everything about that occurrence into a `Log4Qt::LoggingEvent` and hands it to the attached appenders, which in turn pass it to layouts for formatting. `LoggingEvent` is therefore the canonical record that flows through the whole back end of the library.

An event captures the message text, its `Level`, the originating logger, a monotonically increasing sequence number, a timestamp (milliseconds since the Unix epoch, UTC), the nested diagnostic context (NDC), the mapped diagnostic context (MDC) / properties, the originating thread name, an optional source-location `MessageContext` (file/line/function), and an optional category name.

A developer rarely constructs a `LoggingEvent` directly — loggers do that internally — but appender and layout authors read its accessors constantly, and the type is also serializable for relaying events between processes.

## 2. Project Structure and Dependencies

`loggingevent.h` is included by `logger.h`, every appender and layout, and any code that relays events. It includes:

- `level.h` — the `Level` severity stored on the event.
- `<QHash>`, `<QStringList>` — for the MDC/properties map and key list.
- `<QEvent>` — the base class.
- `<atomic>` — for the process-wide sequence counter.
- `<source_location>` — when available, for the `MessageContext(std::source_location)` constructor.
- `<QSharedDataPointer>` — implicitly-shared storage of the event payload.

The implementation (`loggingevent.cpp`) draws context from `InitialisationHelper` (process start time), `DateTime` (current timestamp), `NDC` / `MDC` (diagnostic contexts captured at construction), `Logger` / `LogManager` (resolving the logger name and, on deserialization, the logger pointer), and `<QDataStream>` for serialization.

Build requirement: `Qt6::Core`. Exported via `LOG4QT_EXPORT`.

`MessageContext` is a small public helper struct declared in the same header (see Inter-Class Interactions / member notes below).

## 3. Class Hierarchy and Role

`LoggingEvent` derives from `QEvent`. It registers its own custom event type id (`eventId`, via `QEvent::registerEventType()`), which lets a logging event be posted into a Qt event loop and dispatched like any other `QEvent` — this is how `MainThreadAppender` marshals events onto the GUI thread. Beyond the event-type machinery, `QEvent` contributes no virtual behaviour that this class overrides.

The actual payload lives in a private, implicitly-shared `Data` struct held through a `QSharedDataPointer`. Copying an event is therefore a cheap, `noexcept` atomic refcount bump; the data is duplicated only on the first mutating access (copy-on-write).

## 4. Q_PROPERTY Declarations

None. `LoggingEvent` is not a `QObject` and declares no properties.

## 5. Enumerations

None of its own. The event carries a `Level` value (see the `Level` documentation).

## 6. Public Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `eventId` | `static const QEvent::Type` | The dynamically-registered `QEvent` type id for logging events. Compare `event->type()` against this to recognise a posted `LoggingEvent`. |

The `MessageContext` helper struct (public, declared in this header) exposes three public members used to carry source-location data:

| Variable | Type | Description |
|----------|------|-------------|
| `file` | `const char *` | Source file name; expected to point to a static string (e.g. `__FILE__`). May be `nullptr`. |
| `line` | `int` | Source line number; `-1` when unknown. |
| `function` | `const char *` | Function signature; expected to point to a static string (e.g. `Q_FUNC_INFO`). May be `nullptr`. |

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

### Constructors

#### LoggingEvent()

Default constructor. Builds an event with `Level::NULL_INT`, no logger, an empty message, the current NDC/MDC snapshot, a fresh sequence number, the current timestamp, and the current thread name.

#### LoggingEvent(const Logger *logger, Level level, const QString &message)

Primary constructor used by `Logger::forcedLog`. Captures the logger, level, and message together with the current NDC, MDC, sequence number, timestamp, and thread name.

#### LoggingEvent(const Logger *logger, Level level, const QString &message, const MessageContext &context, const QString &categoryName)

As above, plus a source-location `context` and a `categoryName`. Used by the location-aware logging paths.

#### LoggingEvent(const Logger *logger, Level level, const QString &message, qint64 timeStamp)

Variant that overrides the timestamp with an explicit value (still captures current thread name).

#### LoggingEvent(const Logger *logger, Level level, const QString &message, const QString &ndc, const QHash<QString, QString> &properties, const QString &threadName, qint64 timeStamp)

Full explicit constructor supplying NDC, properties (MDC), thread name, and timestamp — used when reconstructing an event from external data rather than capturing ambient context.

#### LoggingEvent(const Logger *logger, Level level, const QString &message, const QString &ndc, const QHash<QString, QString> &properties, qint64 timeStamp, const MessageContext &context, const QString &categoryName)

Explicit constructor that also carries a `MessageContext` and category name; captures the current thread name.

#### LoggingEvent(const Logger *logger, Level level, const QString &message, const QString &ndc, const QHash<QString, QString> &properties, const QString &threadName, qint64 timeStamp, const MessageContext &context, const QString &categoryName)

The most complete constructor, supplying every field explicitly including the thread name.

#### LoggingEvent(const Logger *logger, Level level, QString &&message)

Move-enabled overload that takes ownership of the message string for zero-copy construction.

#### LoggingEvent(const Logger *logger, Level level, QString &&message, const MessageContext &context, QString &&categoryName)

Move-enabled overload that also moves in the category name and copies a `MessageContext`.

### Copy / move

The copy constructor and copy assignment are `noexcept` (only an atomic refcount operation on the shared data). Move construction and move assignment are defaulted and `noexcept`. The destructor is `virtual` (inherited contract from `QEvent`).

### Accessors

#### Level level() const

Returns the event's severity.

#### const Logger *logger() const

Returns the originating logger pointer (may be `nullptr`, e.g. after deserialization of an unknown logger). Not owned by the caller.

#### QString message() const

Returns the fully-substituted log message text.

#### QString loggername() const

Returns the originating logger's name, or an empty string if there is no logger.

#### QHash<QString, QString> mdc() const

Returns the mapped diagnostic context captured at construction. (Currently backed by the same map as `properties()`.)

#### QString ndc() const

Returns the nested diagnostic context string captured at construction.

#### QHash<QString, QString> properties() const

Returns the event's property map (MDC).

#### QString property(const QString &key) const

Returns the value for a single property key, or an empty string if absent.

#### QStringList propertyKeys() const

Returns the list of property keys.

#### void setProperty(const QString &key, const QString &value)

Inserts or updates a property. Triggers copy-on-write of the shared data.

#### qint64 sequenceNumber() const

Returns this event's unique, monotonically increasing sequence number.

#### QString threadName() const

Returns the name of the thread that produced the event (the thread's `objectName`, or a hex pointer string if unnamed).

#### qint64 timeStamp() const

Returns the event timestamp in milliseconds since the Unix epoch (UTC).

#### QString toString() const

Returns a short rendering of the form `"<LEVEL>:<message>"`.

#### MessageContext context() const

Returns the source-location context.

#### void setContext(const MessageContext &context)

Replaces the source-location context.

#### int lineNumber() const · void setLineNumber(int lineNumber)

Get/set the source line number stored in the context.

#### QString fileName() const

Returns the context's file name decoded from UTF-8.

#### void setFileName(const QString &fileName)

No-op. The context stores a `const char *` assumed to reference a static string, so it cannot adopt a runtime `QString`; the setter is intentionally inert.

#### QString functionName() const

Returns the context's function name decoded from UTF-8.

#### void setMethodName(const QString &functionName)

No-op, for the same reason as `setFileName`.

#### QString categoryName() const · void setCategoryName(const QString &categoryName)

Get/set the optional category name associated with the event.

### Static helpers

#### static qint64 sequenceCount()

Returns the current value of the process-wide sequence counter (the number of events created so far).

#### static qint64 startTime()

Returns the process/library start time (delegates to `InitialisationHelper::startTime()`), useful for relative-time layouts.

## 10. Protected Virtual Methods

None overridden. (The private `setThreadNameToCurrent()` and `nextSequenceNumber()` helpers are implementation details.)

## 11. Ownership and Lifecycle

`LoggingEvent` is a value type with implicitly-shared (copy-on-write) data. Copying is cheap and `noexcept`; mutating a shared instance detaches it. As a `QEvent` subclass it can be heap-allocated and posted into an event loop (the receiver takes ownership per Qt's event-posting rules), but it is also routinely passed and stored by value. The contained `Logger` pointer is borrowed — the event never owns or deletes the logger. The `MessageContext` `file`/`function` pointers are non-owning and must reference strings with static storage duration.

## 12. Thread Safety

A single `LoggingEvent` instance is not internally synchronised; treat it as immutable once handed to appenders and do not mutate the same instance from multiple threads. The implicitly-shared data uses atomic reference counting, so independent copies in different threads are safe. The static sequence counter (`msSequenceCount`) is a `std::atomic<qint64>`, so sequence-number assignment is thread-safe. Thread-name capture caches per-thread state (thread-local) and reacts to `objectName` changes via a property notifier.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- **`Logger`** constructs events (via `forcedLog`) and dispatches them through `callAppenders`.
- **Appenders** receive events in `doAppend` and forward them to **layouts**, which read the accessors to format output.
- **`NDC` / `MDC`** are snapshotted into the event at construction time.
- **`DateTime` / `InitialisationHelper`** supply the timestamp and start time.
- **`LogManager` / `Logger`** are consulted on deserialization to resolve the logger by name.

## 15. External Communication

#### QDataStream &operator<<(QDataStream &out, const LoggingEvent &loggingEvent)

Free function (friend, available unless `QT_NO_DATASTREAM`). Serializes the event with a leading `quint16` version (0): level, logger name, message, NDC, properties, sequence number, thread name, and timestamp. Source-location context and category name are not part of the wire format. Used to relay events between processes or persist them.

#### QDataStream &operator>>(QDataStream &in, LoggingEvent &loggingEvent)

Free function (friend). Reads a versioned event back. If the version does not match the current version (0) the stream is flagged `ReadCorruptData`. For safety it does **not** auto-create loggers from an untrusted stream: the logger pointer is resolved only if a logger of that name already exists (`LogManager::exists`), otherwise it is left `nullptr`.

These data-stream operators are the only external-communication surface; the class itself opens no sockets, pipes, or devices.

## 16. Usage Example

```cpp
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

// Typically built by the framework, but you can construct one to inject
// or relay it through Logger::log(const LoggingEvent&).
Logger *log = Logger::logger(QStringLiteral("net.relay"));

LoggingEvent event(log, Level::INFO_INT, u"Connection accepted from %1"_s.arg("10.0.0.7"));
event.setProperty(QStringLiteral("peer"), QStringLiteral("10.0.0.7"));

// Read back fields (as an appender/layout would).
qDebug() << event.level().toString()   // "INFO"
         << event.timeStamp()
         << event.sequenceNumber()
         << event.message();

// Re-dispatch the pre-built event.
log->log(event);
```
