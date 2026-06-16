# JsonLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j; *layouts* turn a `LoggingEvent` into the representation an appender writes. `JsonLayout` serialises each event as a single-line JSON object followed by a newline, producing NDJSON / JSON Lines output suitable for log aggregators such as Logstash, Fluentd, and Vector.

Reach for `JsonLayout` when logs are consumed by machines rather than read by humans. The set of emitted fields is controlled by `include*` properties; the timestamp is a raw epoch-milliseconds number to avoid locale and parsing issues.

To produce a single valid JSON *array* across a whole file, set the inherited `header` property to `"["` and `footer` to `"]"` and optionally enable `prettyPrint`.

### Output fields

| Key | Source | Default included |
|-----|--------|------------------|
| `timestamp` | epoch ms (`event.timeStamp()`), JSON number | yes |
| `level` | level name | yes |
| `logger` | logger name (falls back to event category name) | yes |
| `thread` | thread name | yes |
| `message` | message text | yes |
| `ndc` | nested diagnostic context (omitted when empty) | no |
| `mdc` | mapped diagnostic context as a nested object (omitted when empty) | no |
| `file`, `line`, `function` | caller location | no |

## 2. Project Structure and Dependencies

- **Instantiated by**: Configurators via the factory and application code assigning a layout to an appender.
- **Qt modules**: Qt Core (`QJsonObject`, `QJsonDocument`).
- **Internal types**: `AbstractStringLayout` (base), `LoggingEvent`, `Logger` (read via `event.logger()->name()`), `MessageContext` (the `event.context()` location record), `Level`.

## 3. Class Hierarchy and Role

`JsonLayout` → `AbstractStringLayout` → `AbstractLayout` → `QObject`. It inherits the meta-object system, layout contract, header/footer provider chain, and charset/byte path. It overrides `contentType()`, `format()`, and `requiresLocation()`. Output is always UTF-8, so the inherited `charset` property has no effect. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `includeTimestamp` | `bool` | `includeTimestamp` | `setIncludeTimestamp` | — | Include the `timestamp` field (epoch ms). Default `true`. |
| `includeLevel` | `bool` | `includeLevel` | `setIncludeLevel` | — | Include the `level` field. Default `true`. |
| `includeLogger` | `bool` | `includeLogger` | `setIncludeLogger` | — | Include the `logger` field. Default `true`. |
| `includeThread` | `bool` | `includeThread` | `setIncludeThread` | — | Include the `thread` field. Default `true`. |
| `includeMessage` | `bool` | `includeMessage` | `setIncludeMessage` | — | Include the `message` field. Default `true`. |
| `includeNdc` | `bool` | `includeNdc` | `setIncludeNdc` | — | Include the `ndc` field; omitted when the NDC is empty. Default `false`. |
| `includeMdc` | `bool` | `includeMdc` | `setIncludeMdc` | — | Include the `mdc` object; omitted when the MDC is empty. Default `false`. |
| `includeLocation` | `bool` | `includeLocation` | `setIncludeLocation` | — | Include `file`, `line`, and `function`. Default `false`. Drives `requiresLocation()`. |
| `prettyPrint` | `bool` | `prettyPrint` | `setPrettyPrint` | — | Emit indented JSON instead of compact. Default `false`. |

## 5. Enumerations

None.

## 6. Public Member Variables

None (all `m*` flags are private behind properties).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### JsonLayout(QObject *parent = nullptr)
Constructs the layout with the default field selection (the five core fields on; NDC, MDC, location off; compact output).

#### Property getters and setters
`includeTimestamp()`/`setIncludeTimestamp(bool)`, `includeLevel()`/`setIncludeLevel(bool)`, `includeLogger()`/`setIncludeLogger(bool)`, `includeThread()`/`setIncludeThread(bool)`, `includeMessage()`/`setIncludeMessage(bool)`, `includeNdc()`/`setIncludeNdc(bool)`, `includeMdc()`/`setIncludeMdc(bool)`, `includeLocation()`/`setIncludeLocation(bool)`, `prettyPrint()`/`setPrettyPrint(bool)`. All defined inline; getters are `[[nodiscard]]`, setters are trivial assignments.

#### QString contentType() const [override]
Returns `"application/json; charset=UTF-8"`.

#### QString format(const LoggingEvent &event) [override]
Builds a `QJsonObject` containing the enabled fields, serialises it with `QJsonDocument::toJson()` (indented when `prettyPrint` is set, otherwise compact), and appends a platform end-of-line. NDC, MDC, and location fields are skipped when their source is empty/absent.

#### bool requiresLocation() const [override]
Returns `true` exactly when `includeLocation` is `true`.

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. Overrides of inherited virtuals are `contentType()`, `format()`, and `requiresLocation()`.

## 11. Ownership and Lifecycle

A `QObject` accepting an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. No owned heap resources beyond transient `QJsonObject`/`QJsonDocument` locals in `format()`. Copy/move disabled.

## 12. Thread Safety

Single-threaded by convention. `format()` builds local JSON objects from the immutable event and reads only the boolean flags, with no shared mutable state, so it is effectively reentrant; concurrent use is mediated by the owning appender's lock.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Appenders call `format()` per event and may call `requiresLocation()` to decide on capturing source location.
- Reads logger name from `Logger`, location from `MessageContext`, and NDC/MDC/level/message/thread/timestamp from the `LoggingEvent`.
- Inherits the header/footer provider chain from `AbstractLayout` (used for the JSON-array bracketing pattern).

## 15. External Communication

`JsonLayout` itself performs no I/O; it only produces strings. The JSON-Lines format is, however, designed for downstream ingestion by external log-aggregation systems (Logstash, Fluentd, Vector) once an appender has written the output.

## 16. Usage Example

```cpp
#include "log4qt/jsonlayout.h"
#include "log4qt/rollingfileappender.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto *json = new JsonLayout();
json->setIncludeMdc(true);
json->setIncludeLocation(true);
auto layout = LayoutSharedPtr(json);
layout->activateOptions();

auto appender = AppenderSharedPtr(new RollingFileAppender(layout, u"app.ndjson"_s));
appender->activateOptions();

Logger::rootLogger()->addAppender(appender);
Logger::logger("MyApp")->info("Service started");
// -> {"timestamp":1748600000000,"level":"INFO","logger":"MyApp",...}
```
