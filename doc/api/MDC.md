# MDC

## 1. Class Overview

In log4j-style logging, a *Mapped Diagnostic Context* (MDC) is a per-thread map of key/value strings that gets injected into log output. It lets an application stamp every log line produced on a thread with contextual data — a request id, user name, session token — without threading those values through every logging call.

`MDC` is Log4Qt's implementation of that concept. It is a process-wide singleton that holds a separate `QHash<QString, QString>` for each thread. Code calls `MDC::put()` at the start of a unit of work and `MDC::remove()` when done; layouts (for example via the pattern formatter's `X` conversion) read the values back through `MDC::get()` or `MDC::context()` when formatting an event. Because storage is thread-local, contexts on different threads never collide.

## 2. Project Structure and Dependencies

`MDC` is read by the layout/pattern-formatting machinery when rendering a `LoggingEvent`, and written by application code that wants contextual logging. It uses the `LOG4QT_IMPLEMENT_INSTANCE` macro (from `helpers/initialisationhelper.h`) to define its singleton accessor.

Internal types: the singleton-instance macro from `InitialisationHelper`. No other Log4Qt types are required by the header.

- **Qt module dependency:** Qt Core (`QString`, `QHash`, `QThreadStorage`).
- **Build requirement:** part of the `log4qt` target linking `Qt6::Core`; exported via `LOG4QT_EXPORT`.

## 3. Class Hierarchy and Role

`MDC` has no base class. It is a non-`QObject` singleton with a private constructor and `Q_DISABLE_COPY_MOVE(MDC)`, so it cannot be copied, moved or freely instantiated. Its sole purpose is to own the thread-local key/value storage and expose static accessors over it.

## 4. Q_PROPERTY Declarations

None.

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

All operational methods are `static` and thread-safe by virtue of operating on thread-local storage.

#### static void put(const QString &key, const QString &value)

Inserts or updates a key/value pair in the calling thread's context. Lazily allocates the thread's hash on first use. Inline.

#### static void remove(const QString &key)

Removes the key from the calling thread's context. Inline. Safe to call for a key that is not present.

#### static QString get(const QString &key)

Returns the value for the key in the calling thread's context, or an empty string if the thread has no context yet or the key is absent.

#### static QHash<QString, QString> context()

Returns a copy of the calling thread's entire context map, or an empty hash if the thread has no context. Used by layouts to render the full MDC.

#### static MDC *instance()

Returns the singleton instance (defined by `LOG4QT_IMPLEMENT_INSTANCE`). Rarely needed directly; the static accessors use it internally.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`MDC` is a leaked singleton created on first access by `instance()`; it is never explicitly destroyed. The per-thread `QHash` objects are heap-allocated lazily on first `put()`/`localData()` for a thread and are owned by the internal `QThreadStorage`, which deletes each thread's hash automatically when that thread exits. Callers never manage any of this memory; they only insert and remove string entries.

## 12. Thread Safety

Thread-safe. Each thread sees its own independent context through `QThreadStorage<QHash<QString, QString> *>`, so concurrent `put`/`get`/`remove`/`context` calls on different threads operate on separate maps and require no locking. There is intentionally no cross-thread sharing of context: a value put on one thread is not visible on another. Cleanup is handled per thread by `QThreadStorage` on thread exit.

## 13. QML Exposure

None.

## 14. Inter-Class Interactions

- Written by application code to attach contextual data to a thread.
- Read by Log4Qt layouts / the pattern formatter when rendering a `LoggingEvent` (the MDC conversion character pulls values via `get()`/`context()`).
- Complements `NDC`, which provides a per-thread *stack* of context strings rather than a key/value map.

## 15. External Communication

None.

## 16. Usage Example

```cpp
#include "log4qt/mdc.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

void handleRequest(const QString &requestId, const QString &user)
{
    MDC::put(QStringLiteral("requestId"), requestId);
    MDC::put(QStringLiteral("user"), user);

    Logger *logger = Logger::logger(QStringLiteral("server"));
    logger->info(QStringLiteral("processing request")); // layout can embed MDC values

    MDC::remove(QStringLiteral("requestId"));
    MDC::remove(QStringLiteral("user"));
}
```
