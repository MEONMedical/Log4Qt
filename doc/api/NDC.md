# NDC

## 1. Class Overview

A *Nested Diagnostic Context* (NDC) is a per-thread stack of context strings used to tag log output with the nesting of the work in progress — for example pushing a stage name on entry to a routine and popping it on exit, so every log line emitted in between carries the surrounding context. It is the stack-based counterpart to the key/value `MDC`.

`NDC` is Log4Qt's implementation. It is a process-wide singleton holding one `QStack<QString>` per thread. Application code calls `push()`/`pop()` to bracket a scope, and layouts read the current context (via the pattern formatter's NDC conversion) when formatting events. Because the stacks are thread-local, the nesting context on one thread is independent of every other thread.

## 2. Project Structure and Dependencies

`NDC` is written by application code to bracket units of work and read by Log4Qt's layout/pattern-formatting machinery when rendering a `LoggingEvent`. It uses `LOG4QT_IMPLEMENT_INSTANCE` (from `helpers/initialisationhelper.h`) for its singleton accessor and `Logger` (via the `LOG4QT_DECLARE_STATIC_LOGGER` macro) to warn on misuse.

Internal types: the singleton-instance macro from `InitialisationHelper`; `Logger` for the internal warning logger.

- **Qt module dependency:** Qt Core (`QString`, `QStack`, `QThreadStorage`).
- **Build requirement:** part of the `log4qt` target linking `Qt6::Core`; exported via `LOG4QT_EXPORT`.

## 3. Class Hierarchy and Role

`NDC` has no base class. It is a non-`QObject` singleton with a private constructor and `Q_DISABLE_COPY_MOVE(NDC)`, so it cannot be copied, moved or freely instantiated. It exists solely to own the thread-local stack storage and expose static accessors over it. As the header notes, there is no `remove()` method — `QThreadStorage` cleans up each thread's stack automatically on thread exit.

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

#### static void push(const QString &message)

Pushes a context string onto the calling thread's stack, lazily allocating the stack on first use. Call on entry to a scope you want reflected in subsequent log output.

#### static QString pop()

Removes and returns the top context string from the calling thread's stack. If the stack is empty (or not yet allocated), logs a warning ("Requesting pop from empty NDC stack") and returns an empty string. Call on exit from the scope established by the matching `push()`.

#### static QString peek()

Returns the top context string without removing it, or an empty string if the stack is empty or not yet allocated.

#### static int depth()

Returns the number of entries on the calling thread's stack, or 0 if no stack exists yet.

#### static void clear()

Empties the calling thread's stack. No-op if no stack exists yet.

#### static void setMaxDepth(int maxDepth)

Truncates the calling thread's stack to at most `maxDepth` entries, discarding the deepest (most recently pushed) beyond that limit. No-op if no stack exists or it is already within the limit. Useful for restoring a known depth after a series of pushes.

#### static NDC *instance()

Returns the singleton instance (defined by `LOG4QT_IMPLEMENT_INSTANCE`). Rarely needed directly; the static accessors use it internally.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`NDC` is a leaked singleton created on first access by `instance()` and never explicitly destroyed. The per-thread `QStack<QString>` objects are heap-allocated lazily on first `push()` for a thread and owned by the internal `QThreadStorage`, which deletes each thread's stack automatically on thread exit — which is why no manual `remove()` exists. Callers never manage this memory; they only push and pop string values.

## 12. Thread Safety

Thread-safe. Each thread sees its own independent stack through `QThreadStorage<QStack<QString> *>`, so concurrent `push`/`pop`/`peek`/`depth`/`clear`/`setMaxDepth` calls on different threads operate on separate stacks and need no locking. Context pushed on one thread is never visible on another. Per-thread cleanup happens automatically when the thread exits.

## 13. QML Exposure

None.

## 14. Inter-Class Interactions

- Written by application code to bracket nested units of work.
- Read by Log4Qt layouts / the pattern formatter when rendering a `LoggingEvent` (the NDC conversion character reads the current stack).
- Uses an internal `Logger` to warn when `pop()` is called on an empty stack.
- Complements `MDC`, which provides a per-thread key/value map rather than a stack.

## 15. External Communication

None.

## 16. Usage Example

```cpp
#include "log4qt/ndc.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

void importFile(const QString &path)
{
    NDC::push(QStringLiteral("import:") + path);

    Logger *logger = Logger::logger(QStringLiteral("io"));
    logger->info(QStringLiteral("starting"));   // layout can embed the NDC stack
    // ... nested work, each scope may push/pop further context ...
    logger->info(QStringLiteral("finished"));

    NDC::pop();
}
```
