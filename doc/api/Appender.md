# Appender

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging framework. In that architecture an **Appender** is the object responsible for taking a fully constructed `LoggingEvent` and delivering it to a destination — a console, a file, a network endpoint, an in-memory list, and so on. Loggers decide *whether* an event should be logged; appenders decide *where* it goes.

`Appender` is the root of the appender hierarchy. It defines the contract every appender must satisfy: it can be named, can carry a `Layout` (which turns an event into formatted text), can host a chain of `Filter` objects, and exposes a single entry point — `doAppend()` — through which logging events flow.

Although log4j models the appender as a Java interface, Log4Qt deliberately makes `Appender` a concrete `QObject` subclass rather than a pure interface. The class comment explains the reason: making the whole hierarchy descend from `QObject` requires `Appender` itself to be a `QObject`, so it is implemented as an abstract base class with pure virtual methods rather than a `Q_DECLARE_INTERFACE` interface. Concrete behaviour lives in `AppenderSkeleton` and its descendants.

## 2. Project Structure and Dependencies

`Appender` is declared in `appender.h` and is the base type referenced throughout the library wherever appenders are stored, configured, or attached to loggers. The shared-pointer alias `AppenderSharedPtr` (defined at the end of the header) is the canonical way the rest of the library holds appenders.

Build requirement: **Qt Core** (`QObject`). The library links `Qt::Core` publicly (see `src/log4qt/CMakeLists.txt`).

Project-internal types it depends on:

- **`AbstractLayout` / `LayoutSharedPtr`** (`abstractlayout.h`) — the layout that formats a `LoggingEvent` into a string. `LayoutSharedPtr` is a `Log4QtSharedPtr<Layout>`.
- **`Filter` / `FilterSharedPtr`** (`spi/filter.h`) — a single link in the filter chain; returns an `Accept` / `Deny` / `Neutral` decision per event.
- **`LoggingEvent`** (`loggingevent.h`, forward-declared) — the immutable record of a single log call.
- **`Log4QtSharedPtr`** (`log4qtsharedptr.h`) — the reference-counted smart pointer used for managed ownership.
- **`ClassLogger`** (`helpers/classlogger.h`) — lazily provides a `Logger` named after the concrete class, used for internal error reporting.
- **`LOG4QT_EXPORT`** (`log4qtdefs.h`) — the import/export visibility macro for the shared library.

## 3. Class Hierarchy and Role

`Appender` derives directly from **`QObject`**, which contributes the meta-object system (`Q_OBJECT`), signals/slots support, runtime type information via `qobject_cast`, the `objectName` used to back the appender's `name`, and parent-based ownership.

`Appender` is **abstract**: it declares nine pure virtual methods that every concrete appender must implement. It does not use `Q_DECLARE_INTERFACE` / `Q_INTERFACES`; the class comment notes the QObject-base requirement as the reason it is an abstract base class rather than a Qt plugin interface.

Copy and move are disabled via `Q_DISABLE_COPY_MOVE`, as is standard for `QObject` subclasses.

Its direct concrete descendant is `AppenderSkeleton`, which implements the general functionality (layout, filter chain, threshold, activation/close state, and the `doAppend()` lifecycle).

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `layout` | `LayoutSharedPtr` | `layout` | `setLayout` | — | The layout that formats each `LoggingEvent` into text. |
| `name` | `QString` | `name` | `setName` | — | The appender's name, used to look it up in configuration and to identify it in error messages. |
| `requiresLayout` | `bool` | `requiresLayout` | — | — | Read-only (`CONSTANT`). Indicates whether this appender needs a layout to function. Console and file appenders return `true`; appenders such as null/list appenders return `false`. |

## 5. Public Methods

All of the following except the constructor and destructor are **pure virtual** — they define the contract every appender implementation must fulfil.

#### Appender(QObject *parent = nullptr)

Constructs the appender with an optional `QObject` parent. The parent (if set) owns the appender for Qt-parent lifetime purposes.

#### virtual ~Appender()

Virtual destructor, enabling safe polymorphic deletion through an `Appender *` or `AppenderSharedPtr`.

#### FilterSharedPtr filter() const

Contract: returns the first `Filter` in the appender's filter chain (the head), or a null pointer if no filter is set. Marked `[[nodiscard]]`.

#### QString name() const

Contract: returns the appender's name. Marked `[[nodiscard]]`.

#### LayoutSharedPtr layout() const

Contract: returns the layout currently attached to the appender, or null if none is set. Marked `[[nodiscard]]`.

#### bool requiresLayout() const

Contract: returns `true` if the appender cannot operate without a layout. Callers (and `AppenderSkeleton::checkEntryConditions()`) use this to reject use of an appender that requires a layout but has none. Marked `[[nodiscard]]`.

#### void setLayout(const LayoutSharedPtr &layout)

Contract: attaches `layout` as the formatter for subsequent events. Passing a null pointer clears the layout.

#### void setName(const QString &name)

Contract: sets the appender's name.

#### void addFilter(const FilterSharedPtr &filter)

Contract: appends `filter` to the end of the filter chain. Implementations are expected to ignore a null filter.

#### void clearFilters()

Contract: removes every filter from the chain.

#### void close()

Contract: releases any resources held by the appender (open files, streams, sockets) and marks the appender as closed so that further events are rejected. Must be idempotent.

#### void doAppend(const LoggingEvent &event)

Contract: the single entry point through which logging events are delivered. An implementation must perform all entry checks (active, not closed, threshold, layout present), run the filter chain, and only then write the event to its destination. This is the method loggers call.

## 6. Protected Methods

#### Logger *logger() const

Returns a pointer to a `Logger` named after the concrete object's class, obtained from the internal `ClassLogger`. Used by subclasses to report internal errors (missing layout, I/O failures, etc.) through the logging framework itself. The returned `Logger` is owned by the logger repository, **not** by the appender.

## 7. Ownership and Lifecycle

`Appender` is a `QObject`, so an instance passed a non-null `parent` is destroyed when its parent is destroyed. In practice the library manages appenders through `AppenderSharedPtr` (a `Log4QtSharedPtr<Appender>`): loggers and the log manager hold reference-counted shared pointers, and the appender is destroyed when the last reference is released. This is the "managed ownership" referred to in the class documentation. Concrete subclasses close their resources from their destructors.

## 8. Thread Safety

The class documentation states that **all functions declared in this class are thread-safe**. `Appender` itself adds no locking — the contract is fulfilled by the implementing class. `AppenderSkeleton` provides the synchronisation via a recursive mutex (`QRecursiveMutex mObjectGuard`) plus atomics for the active/closed flags, and concrete appenders honour the same guarantee. Callers may therefore invoke any public method, including `doAppend()`, concurrently from multiple threads.

## 9. Inter-Class Interactions

- **Loggers** hold `AppenderSharedPtr` instances and invoke `doAppend()` for every event that passes level filtering.
- **`Layout`** is queried (via `layout()`) to format the event text.
- **`Filter`** chain is consulted to accept, deny, or remain neutral on each event.
- **`Logger` (internal)** obtained through `logger()` is used to surface the appender's own errors back into the logging system.
- Configurators (property, JSON, XML) read and write the `name`, `layout`, and `requiresLayout` properties when building appenders from configuration.
