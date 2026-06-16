# XMLLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j; *layouts* turn a `LoggingEvent` into the representation an appender writes. `XMLLayout` serialises each event as a `log4j:event` XML fragment compatible with the log4j XML event format consumed by tools such as Apache Chainsaw.

Reach for `XMLLayout` when an XML-based log viewer or downstream parser expects the log4j event schema. Each call to `format()` produces one self-contained `log4j:event` element (not a full XML document with a single root), so a file of such fragments is a concatenation rather than one well-formed document; viewers that expect the log4j stream handle this.

### Produced structure

- `<log4j:event logger="…" timestamp="…" level="…" thread="…">`
  - `<log4j:message><![CDATA[ … ]]></log4j:message>`
  - `<log4j:NDC><![CDATA[ … ]]></log4j:NDC>` — only when the NDC is non-empty
  - `<log4j:properties>` with one `<log4j:data name="…" value="…"/>` per event property — only when properties exist

## 2. Project Structure and Dependencies

- **Instantiated by**: Configurators via the factory and application code assigning a layout to an appender.
- **Qt modules**: Qt Core (`QXmlStreamWriter`).
- **Internal types**: `AbstractStringLayout` (base), `LoggingEvent`, `Level`.

## 3. Class Hierarchy and Role

`XMLLayout` → `AbstractStringLayout` → `AbstractLayout` → `QObject`. It inherits the meta-object system, layout contract, header/footer provider chain, and charset/byte path, and overrides `contentType()` and `format()`. Output is always UTF-8. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

None of its own (inherits `charset`, `header`, `footer`, `contentType` from base classes).

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### XMLLayout(QObject *parent = nullptr)
Constructs the layout. Defined in the source; no configuration.

#### QString contentType() const [override]
Returns `"application/xml; charset=UTF-8"`.

#### QString format(const LoggingEvent &event) [override]
Writes a `log4j:event` element via `QXmlStreamWriter`, with `logger`, `timestamp`, `level`, and `thread` attributes; a `log4j:message` child holding the message in a CDATA section; a `log4j:NDC` child (CDATA) when the NDC is non-empty; and a `log4j:properties` block of `log4j:data` name/value pairs when the event carries properties. Returns the assembled fragment.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. Overrides of inherited virtuals are `contentType()` and `format()`.

## 11. Ownership and Lifecycle

A `QObject` accepting an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. The `QXmlStreamWriter` is a stack-local inside `format()` writing into a local `QString`; no owned heap resources. Copy/move disabled.

## 12. Thread Safety

Single-threaded by convention. `format()` uses only local state (a stack `QString` and `QXmlStreamWriter`) over the immutable event, with no shared mutable members, so it is effectively reentrant; concurrent use is mediated by the owning appender's lock.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Appenders call `format()` per event.
- Reads logger name, timestamp, level, thread, message, NDC, and properties from the `LoggingEvent`.
- Inherits the header/footer provider chain from `AbstractLayout`.

## 15. External Communication

`XMLLayout` performs no I/O itself; it produces strings only. The output conforms to the log4j XML event format and is intended for consumption by external log viewers (e.g. Apache Chainsaw) after an appender writes it.
