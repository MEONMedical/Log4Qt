# log4qtsharedptr.h

## A. Overview

`log4qtsharedptr.h` defines `Log4QtSharedPtr`, a thin `QSharedPointer` subclass tailored for managing Log4Qt's `QObject`-derived objects (appenders, layouts, filters). It exists to solve an ownership problem described in the library's design notes: appenders, layouts, and filters can be created explicitly or by a configurator, can be shared by multiple owners (e.g. one layout used by several appenders), and may be created and destroyed across threads and during static (de)initialization. A plain `QObject` parent/child tree cannot express shared ownership and forces all children into one thread, so the library reference-counts these objects instead.

`Log4QtSharedPtr` adds two guarantees over a bare `QSharedPointer`:

- **Deferred deletion.** When the last reference is released, the managed object is destroyed via `QObject::deleteLater()` rather than an immediate `delete`. This lets the object finish processing queued events and be torn down on its own thread's event loop, which is important for cross-thread cleanup.
- **Type safety.** A compile-time `static_assert` rejects any type that is not derived from `QObject`.

A developer reaches for this template (typically through the library's `AppenderSharedPtr`, `LayoutSharedPtr`, `FilterSharedPtr` aliases) whenever they transfer ownership of a Log4Qt `QObject` into the reference-counted ownership model — for example when adding an appender to a logger.

## B. Namespaces

| Namespace | Groups |
|-----------|--------|
| `Log4Qt` | Encloses the `Log4QtSharedPtr` class template alongside the rest of the package. |

## C. Types and Type Aliases

| Name | Kind | Description |
|------|------|-------------|
| `Log4Qt::Log4QtSharedPtr<Log4QtClass>` | `class template` (publicly derives from `QSharedPointer<Log4QtClass>`) | A shared pointer that deletes its target via `deleteLater()` and restricts `Log4QtClass` to `QObject`-derived types. |

### Template parameter

| Parameter | Constraint | Description |
|-----------|------------|-------------|
| `Log4QtClass` | Must derive from `QObject` (enforced by `static_assert`) | The managed object type. |

## D. Constants

None.

## E. Functions / Macros

Because `Log4QtSharedPtr` publicly inherits from `QSharedPointer`, it exposes the full `QSharedPointer` interface (`data()`, `operator->`, `operator*`, `isNull()`, `reset()`, `staticCast()`, etc.). Only the members it adds or overrides are documented here.

#### explicit Log4QtSharedPtr(Log4QtClass *ptr)

Takes ownership of the raw pointer `ptr` and arranges for the object to be destroyed with `Log4QtClass::deleteLater` when the last reference is released. The constructor is **explicit**: a raw pointer will not implicitly convert to a `Log4QtSharedPtr`. This is deliberate — an implicit conversion would be a use-after-free hazard when the source pointer is on the stack, is a member of another object, or is already owned elsewhere. Callers must spell the ownership transfer explicitly, e.g. `AppenderSharedPtr(new ConsoleAppender(...))`.

When to use: at the single point where ownership of a freshly created `QObject`-derived Log4Qt object passes into the reference-counted model.

Preconditions: `ptr` must be a heap-allocated object that is not already managed by another smart pointer or owned by a `QObject` parent that will also delete it; `Log4QtClass` must derive from `QObject` (otherwise compilation fails via `static_assert`).

Thread-safety: the reference count is atomic, matching `QSharedPointer`. Final destruction is dispatched to the managed object's thread through `deleteLater()`.

#### Log4QtSharedPtr()

Default-constructs a null shared pointer that owns nothing. Use it to declare an empty handle to be assigned later. Holds no object, so no deletion occurs on destruction. Thread-safety: as `QSharedPointer`.

#### Log4QtSharedPtr(const QSharedPointer<Log4QtClass> &other)

Constructs from an existing `QSharedPointer`, sharing its reference count and deleter. This converting constructor lets a plain `QSharedPointer<Log4QtClass>` be adopted into a `Log4QtSharedPtr` without changing the original deletion behaviour. Thread-safety: as `QSharedPointer`.

#### Log4QtSharedPtr(const QWeakPointer<Log4QtClass> &other)

Constructs a strong reference by promoting a `QWeakPointer`. If the weak pointer's target has already expired, the result is null. Use it to obtain a temporary owning handle from a weak observer. Thread-safety: as `QSharedPointer`.

## F. Dependencies

| Include | Provides |
|---------|----------|
| `<QSharedPointer>` | The `QSharedPointer` / `QWeakPointer` base machinery and atomic reference counting. |
| `<QObject>` | `QObject` and its `deleteLater()` slot used as the deleter and as the `static_assert` base-class constraint. |
| `<type_traits>` | `std::is_base_of_v`, used by the `static_assert` that enforces the `QObject` constraint. |

## G. Usage Example

```cpp
#include <log4qt/log4qtsharedptr.h>
#include <log4qt/consoleappender.h>
#include <log4qt/logger.h>

using namespace Log4Qt;

// Library type aliases such as AppenderSharedPtr are instantiations of
// Log4QtSharedPtr; here the template is used directly for illustration.
Log4QtSharedPtr<ConsoleAppender> appender(new ConsoleAppender);
appender->activateOptions();

Logger *logger = Logger::logger("MyClass");
logger->addAppender(appender);   // shared ownership; ref-counted

// When the last reference is dropped, the appender is torn down via
// deleteLater() on its own thread's event loop.
```
