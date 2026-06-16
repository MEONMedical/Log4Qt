# LogStream

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. While the `Logger` class offers direct message-and-arguments logging, it also provides an `ostream`-style alternative for building a log line piece by piece. `Log4Qt::LogStream` is that helper: it wraps a `QTextStream` and accumulates everything streamed into it with `operator<<`, then emits the accumulated text as a single log message at the bound level when the stream object is destroyed.

A developer obtains a `LogStream` by calling a no-argument logging method on a logger — `logger->debug()`, `logger->warn()`, `logger->log(level)`, etc. — and then chains `<<` operators. This is convenient for composing messages from heterogeneous values (numbers, `QString`, any type `QTextStream` understands) without manual formatting or `arg` placeholders.

## 2. Project Structure and Dependencies

`logstream.h` is included by `logger.h` (whose no-argument level methods return a `LogStream`). It includes:

- `level.h` — the `Level` the stream logs at.
- `<QTextStream>` — the underlying formatter that backs `operator<<`.
- `<QString>` — the accumulation buffer.
- `<QPointer>` — a guarded weak reference to the originating `Logger`.
- `<memory>` — for the `std::shared_ptr` holding the internal stream state.
- `<concepts>` — when available (`__cpp_concepts`), to constrain `operator<<` with the `QTextStreamable` concept.

The implementation (`logstream.cpp`) calls back into `Logger::isEnabledFor` (at construction) and `Logger::log` (at destruction). Build requirement: `Qt6::Core`. Exported via `LOG4QT_EXPORT`.

The header also defines a file-scope concept used to constrain the streaming operator:

| Name | Kind | Description |
|------|------|-------------|
| `QTextStreamable<T>` | concept (when `__cpp_concepts`) | Satisfied when `ts << t` is valid for a `QTextStream &ts` and yields a `QTextStream &`. Constrains `LogStream::operator<<` so only types `QTextStream` can format are accepted. |

## 3. Class Hierarchy and Role

`LogStream` has no base class — it is a small, movable/copyable handle whose state lives in a private, shared `Stream` struct. It is not a `QObject` and participates in no inheritance. Its role is purely to defer and aggregate a log message: construction decides whether logging is enabled, streaming fills a buffer, and destruction of the last owner flushes that buffer to the logger.

The private `Stream` struct owns a `QTextStream` writing into a `QString buffer`, a `QPointer<const Logger>` back to the originating logger, and the target `Level`.

## 4. Q_PROPERTY Declarations

None.

## 5. Enumerations

None of its own. It carries a `Level` (see the `Level` documentation).

## 6. Public Member Variables

None. All state is private.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### LogStream(const Logger &iLogger, Level iLevel)

Constructs a stream bound to `iLogger` and `iLevel`. The constructor immediately checks `iLogger.isEnabledFor(iLevel)`: if the level is **disabled**, no internal `Stream` is allocated, so subsequent `operator<<` calls are cheap no-ops and nothing is logged. If enabled, it allocates the shared `Stream` that will buffer the message. Normally called indirectly through `Logger::debug()`, `Logger::warn()`, `Logger::log(level)`, and the other no-argument level methods, rather than directly.

#### template<QTextStreamable T> LogStream &operator<<(const T &t)

Streams `t` into the internal `QTextStream` buffer and returns `*this` for chaining. When the stream was constructed in the disabled state (no internal `Stream`), the call does nothing. The accepted types are exactly those a `QTextStream` can format (constrained by the `QTextStreamable` concept where concepts are available). The message is **not** logged here — only on destruction.

## 10. Protected Virtual Methods

None. The internal `Stream` struct (with its constructor and flushing destructor) is a private implementation detail.

## 11. Ownership and Lifecycle

`LogStream` is a value type. Its internal state is held in a `std::shared_ptr<Stream>`, so copies share the same buffer and the flush happens when the **last** copy is destroyed (the `Stream` destructor logs the accumulated buffer if its logger pointer is still valid). In practice a `LogStream` is a short-lived temporary: `logger->info() << "x" << y;` builds the message and flushes it at the end of the full expression.

The back-reference to the logger is a `QPointer<const Logger>`; if the logger were destroyed before the flush (not expected, since loggers live for the process lifetime), the pointer would be null and the buffered message simply dropped — guarding against use-after-free.

## 12. Thread Safety

A single `LogStream` instance is intended to be used by one thread for the duration of one log expression; it is not designed for concurrent streaming from multiple threads. The flush-on-destruction path calls `Logger::log`, which is itself thread-safe, so producing log streams on different threads is safe as long as each instance stays on one thread.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- **`Logger`** is both the producer and the consumer: `Logger::debug()` / `warn()` / `error()` / `fatal()` / `info()` / `trace()` / `log(Level)` (and `MessageLogger::log()`) return a `LogStream`, and the stream's destructor calls back into `Logger::log(level, buffer)` to actually emit the message.
- **`Level`** selects the severity at which the buffered message is logged and gates allocation via `isEnabledFor`.
- Indirectly, the flushed message becomes a `LoggingEvent` inside `Logger`, which is then dispatched to appenders.

## 15. External Communication

None. `LogStream` only buffers text and hands it to a `Logger`; it performs no I/O of its own.

## 16. Usage Example

```cpp
#include "log4qt/logger.h"

using Log4Qt::Logger;

Logger *log = Logger::logger(QStringLiteral("example.stream"));

// ostream-style composition; the message is logged when the temporary
// LogStream returned by warn() is destroyed at the end of the statement.
log->warn() << "Retry " << 3 << " of " << 5 << " failed for host " << "db-01";

// If WARN is disabled on this logger, the whole expression is a cheap no-op:
// no buffer is allocated and nothing is formatted.
```
