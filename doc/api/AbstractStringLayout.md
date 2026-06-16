# AbstractStringLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of the Apache log4j logging framework, where *layouts* turn a `LoggingEvent` into the representation an appender writes. `AbstractStringLayout` sits between the root `AbstractLayout` and the concrete text-producing layouts (`PatternLayout`, `TTCCLayout`, `SimpleLayout`, `SimpleTimeLayout`, `JsonLayout`, `XMLLayout`).

It adds three capabilities inspired by the log4j 2 `AbstractStringLayout` design:

- **Charset management** — a `charset` property (default `"UTF-8"`) reported in `contentType()` and used by the default byte-encoding path.
- **Direct byte-encoding path** — `formatTo()` writes the encoded event directly into a caller-supplied `QByteArray`, avoiding the temporary `QString` that `format().toUtf8()` would produce.
- **Thread-local scratch buffer** — `threadLocalBuffer()` returns a per-thread `QByteArray` that appenders can fill outside their lock and consume inside it.

Reach for this class as the base when writing a new text layout that should participate in charset reporting and the efficient byte path.

## 2. Project Structure and Dependencies

- **Subclassed by**: `PatternLayout`, `TTCCLayout`, `SimpleLayout`, `SimpleTimeLayout`, `JsonLayout`, `XMLLayout`.
- **Used by**: Byte-oriented appenders (e.g. `RandomAccessFileAppender`) call `formatTo()` into `threadLocalBuffer()` rather than `format().toUtf8()`; see `AppenderSkeleton::preAppend()`.
- **Qt modules**: Qt Core (`QObject`, `QString`, `QByteArray`).
- **Internal types**: `LoggingEvent` (event record, included in the `.cpp`); base class `AbstractLayout`; `LOG4QT_EXPORT` from `log4qtshared.h`.

## 3. Class Hierarchy and Role

`AbstractStringLayout` derives from `AbstractLayout`, which derives from `QObject`. From `QObject` it inherits the meta-object system and parent ownership; from `AbstractLayout` it inherits the layout contract (abstract `format()`, header/footer, provider chain, lifecycle, `requiresLocation()`). It adds the `charset` property and the byte-oriented `formatTo()` / `threadLocalBuffer()` facilities. It does not implement `format()` itself, so it remains abstract. Copy and move are disabled via `Q_DISABLE_COPY_MOVE`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `charset` | `QString` | `charset` | `setCharset` | — | IANA encoding name used when encoding the formatted string to bytes in `formatTo()` and reported in `contentType()`. Default `"UTF-8"`. Note that some subclasses (e.g. `JsonLayout`, `XMLLayout`) hard-code UTF-8 and ignore this property. |

## 5. Enumerations

None.

## 6. Public Member Variables

None (the `charset` member is private behind the property).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### QString charset() const
Returns the configured charset name. Defined inline; default `"UTF-8"`.

#### void setCharset(const QString &charset)
Sets the charset name used by `contentType()` and the default `formatTo()`.

#### QString contentType() const [override]
Overrides `AbstractLayout::contentType()` to return `"text/plain; charset=<charset>"`. Subclasses override further to return a more specific MIME type.

#### void formatTo(const LoggingEvent &event, QByteArray &dest) [virtual]
Formats `event` and appends the encoded bytes to `dest`. The default implementation calls `format(event).toUtf8()` and appends the result. `dest` is *not* cleared first — the caller clears it when a fresh buffer is needed. Subclasses may override to write directly into the byte array, skipping the intermediate `QString`.

#### static QByteArray &threadLocalBuffer()
Returns a reference to the calling thread's `thread_local` scratch buffer, which lives for the thread's lifetime. Callers must call `QByteArray::clear()` before reuse. Intended for appenders that fill the buffer via `formatTo()` outside the appender lock and consume it under the lock in `append()`.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. The overridable surface is `contentType()` (overridden here from `AbstractLayout`) and the new `virtual formatTo()`. The inherited pure-virtual `format()` remains unimplemented.

## 11. Ownership and Lifecycle

A `QObject` accepting a `QObject *parent` (default `nullptr`); parent-owned when a parent is given, otherwise typically held by a `LayoutSharedPtr`. The `threadLocalBuffer()` storage is owned by the thread, not by any instance, and outlives individual layout objects. Copy/move disabled.

## 12. Thread Safety

Per-instance configuration (`setCharset`) is single-threaded by convention. `formatTo()` is reentrant insofar as `format()` is. The `threadLocalBuffer()` mechanism is the explicit thread-safety affordance: each thread gets its own buffer, so concurrent formatting on different threads never shares the scratch storage. The buffer reference must not be passed between threads.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Concrete subclasses implement `format()`; the byte path here delegates to that override.
- Byte-oriented appenders call `formatTo()` and `threadLocalBuffer()` (coordinated with `AppenderSkeleton::preAppend()`), then write the bytes to their destination.
- `contentType()` (including the `charset`) is read by appenders/configurators that record output metadata.
