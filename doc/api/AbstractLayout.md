# AbstractLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of the Apache log4j logging framework. A logging pipeline in Log4Qt consists of *loggers* that emit `LoggingEvent` objects, *appenders* that deliver those events to a destination (console, file, database, socket, …), and *layouts* that turn each `LoggingEvent` into a textual (or other) representation the appender can write.

`AbstractLayout` is the root of the layout class hierarchy. It defines the abstract formatting contract — `QString format(const LoggingEvent &event)` — that every concrete layout must implement, and it centralises the cross-cutting concerns shared by all layouts: a content type, configurable header/footer strings, a pluggable header/footer provider mechanism (per-layout and global), an `activateOptions()` lifecycle hook, a `requiresLocation()` capability query, and the platform end-of-line helper.

Reach for `AbstractLayout` only as a base class: you derive from it (or, more usually, from `AbstractStringLayout`) when implementing a new layout. Concrete callers instantiate one of its subclasses (`PatternLayout`, `SimpleLayout`, `JsonLayout`, …) and assign it to an appender.

## 2. Project Structure and Dependencies

- **Instantiated / used by**: Every appender that produces formatted output holds a `LayoutSharedPtr` (alias for `Log4QtSharedPtr<AbstractLayout>`) and calls `format()` on it. Configurators (`PropertyConfigurator`, `XmlConfigurator`, `JsonConfigurator`) create concrete layouts via the factory and set their properties.
- **Subclassed by**: `AbstractStringLayout` (and through it `PatternLayout`, `TTCCLayout`, `SimpleLayout`, `SimpleTimeLayout`, `JsonLayout`, `XMLLayout`), and `DatabaseLayout` (which derives directly from `AbstractLayout`).
- **Qt modules**: Qt Core only (`QObject`, `QString`, `QReadWriteLock`). The `target_link_libraries` entry for `log4qt` links `Qt::Core` publicly.
- **Internal types**:
  - `LoggingEvent` (forward-declared) — the immutable event record passed to `format()`.
  - `HeaderFooterProviderSharedPtr` — alias for `Log4QtSharedPtr<HeaderFooterProvider>`, defined in `spi/headerfooterprovider.h`; supplies header/footer strings.
  - `Log4QtSharedPtr` — the project's managed shared-pointer type used for layout ownership.
  - `LOG4QT_EXPORT` — visibility/import-export macro from `log4qtshared.h`.

## 3. Class Hierarchy and Role

`AbstractLayout` inherits `QObject`. From `QObject` it gains the meta-object system (so subclass `Q_PROPERTY`, `Q_ENUM`, and the object name used by `name()`/`setName()` all work), `objectName` storage, and parent-based ownership. `AbstractLayout` itself adds the layout contract: an abstract `format()`, configurable header/footer, the provider chain, and lifecycle/capability hooks. Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `contentType` | `QString` | `contentType` | — | — | Read-only MIME type of the produced output. The base returns `text/plain`. |
| `footer` | `QString` | `footer` | `setFooter` | — | Static footer string emitted at the end of output. Resolution order on read: per-layout provider, then this static string, then global provider. |
| `header` | `QString` | `header` | `setHeader` | — | Static header string emitted at the start of output. Same three-source resolution order as `footer`. |

## 5. Enumerations

None.

## 6. Public Member Variables

None (all data members are private).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### QString contentType() const [virtual]
Returns the MIME content type of the formatted output. The base implementation returns `text/plain`. Subclasses override it to report a more specific type (e.g. `application/json; charset=UTF-8`).

#### QString header() const [virtual]
Returns the effective header string. Resolution order: if a per-layout provider is set and returns a non-empty header, that wins; otherwise the static `header` string if non-empty; otherwise the global provider's header if one is registered and non-empty; otherwise an empty string.

#### QString footer() const [virtual]
Returns the effective footer string using the same three-source priority as `header()`.

#### QString name() const
Returns the layout's name, which is its `QObject::objectName()`.

#### void setFooter(const QString &footer)
Sets the static footer string. Unguarded with respect to `activateOptions()`; should be set during configuration.

#### void setHeader(const QString &header)
Sets the static header string. Same lifecycle note as `setFooter()`.

#### void setName(const QString &name)
Sets the layout's name by setting its `objectName`.

#### void setHeaderFooterProvider(const HeaderFooterProviderSharedPtr &provider)
Sets a per-layout header/footer provider. When set and the provider returns a non-empty string, it takes priority over both pattern-based and static header/footer strings. Must be called *before* `activateOptions()`; calling it afterwards is a programming error asserted in debug builds.

#### HeaderFooterProviderSharedPtr headerFooterProvider() const
Returns the per-layout provider, or a null pointer if none has been set.

#### static void setGlobalHeaderFooterProvider(const HeaderFooterProviderSharedPtr &provider)
Registers a process-wide fallback provider used by every layout that has neither a per-layout provider nor a static header/footer string. Passing a null pointer clears it. Thread-safe; intended to be called once at application startup.

#### static HeaderFooterProviderSharedPtr globalHeaderFooterProvider()
Returns the currently registered global provider, or a null pointer. Thread-safe.

#### void activateOptions() [virtual]
Lifecycle hook called after all configurable properties have been set. The base implementation marks the layout as activated (locking out further `setHeaderFooterProvider()` calls) and forwards `activateOptions()` to the per-layout provider if present. Subclasses that override should call the base implementation.

#### bool requiresLocation() const [virtual]
Returns `true` if the layout uses caller location information (file, line, method). Appenders query this to decide whether capturing source location is worth the cost. The base implementation returns `false`.

#### static QString endOfLine()
Returns the end-of-line separator used by layouts. The implementation returns `"\n"` on all platforms.

## 10. Protected Virtual Methods / Event Handlers

`AbstractLayout` declares no `protected` members. The polymorphic surface intended for subclasses is the set of public `virtual` methods: the pure-virtual `format()` (must be implemented), and the overridable `contentType()`, `header()`, `footer()`, `activateOptions()`, and `requiresLocation()`. Subclasses overriding `activateOptions()` should call `AbstractLayout::activateOptions()`.

## 11. Ownership and Lifecycle

`AbstractLayout` is a `QObject` and accepts a `QObject *parent` (default `nullptr`); when a parent is set, Qt deletes the layout with the parent. In practice layouts are managed through `LayoutSharedPtr` (`Log4QtSharedPtr<AbstractLayout>`), and appenders hold that shared pointer. The destructor is virtual and defaulted. Header/footer providers are held by shared pointer (`HeaderFooterProviderSharedPtr`) and are not exclusively owned by the layout. The global provider is a static shared pointer that lives for the process lifetime until replaced or cleared.

## 12. Thread Safety

Instance methods (`setHeader`, `setFooter`, `header`, `footer`, `format`, …) are not internally synchronised and assume single-threaded configuration followed by use under the appender's own locking. The *global* provider, however, is guarded: `setGlobalHeaderFooterProvider()` takes a write lock and `globalHeaderFooterProvider()` plus the global-provider branch of `header()`/`footer()` take read locks on a shared static `QReadWriteLock`, so registering and reading the global provider is thread-safe.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Appenders hold a `LayoutSharedPtr` and call `format()` per event; some call `requiresLocation()` to decide on location capture.
- Reads the per-layout and global `HeaderFooterProvider` to resolve header/footer text.
- `activateOptions()` propagates activation to the attached provider.
- Configurators set the `header`, `footer`, and `contentType`-adjacent properties through the meta-object system.
