# Properties

## 1. Class Overview

`Properties` implements a log4j/Java-style *properties* map: an associative store of string keys to string values, with support for loading from a `java.util.Properties`-format text stream or a `QSettings` object, and a *default-fallback chain* that lets one `Properties` object delegate lookups for unknown keys to another.

It is the in-memory representation of a Log4Qt configuration. The configurators populate a `Properties` object from a `.properties`/`.ini`-style file (or from `QSettings`), then read typed values out of it — usually via `OptionConverter`, which also performs `${...}` substitution against the same map.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/helpers/properties.h`
- Source: `src/log4qt/helpers/properties.cpp`

Header dependencies: `log4qt/log4qtshared.h` (export macro), `QHash`, `QStringList`. The header forward-declares `QIODevice` and `QSettings`.

Source dependencies: `logger.h` (for `LOG4QT_DECLARE_STATIC_LOGGER` and parse warnings), `QIODevice`, `QSettings`, `QTextStream`.

## 3. Class Hierarchy and Role

`Properties` holds a `QHash<QString, QString>` by **composition** (a private member). It is not a `QObject` and has no base class. It re-exposes the subset of the hash interface its callers rely on — `clear`, `insert`, `contains`, `value`, `count`, `size`, `isEmpty`, `keys` — and layers on property-file/`QSettings` loading, the default-properties fallback chain, and convenience accessors that honor that chain. It is tagged `Q_DECLARE_TYPEINFO(Log4Qt::Properties, Q_MOVABLE_TYPE)`.

(Earlier versions derived publicly from `QHash<QString, QString>`. That base was removed: a public base class with no virtual destructor is a slicing/undefined-behaviour hazard if an instance is ever deleted through a `QHash *`. Consumers that relied on `Properties` *being* a `QHash` — iteration, implicit upcast — must move to the explicit accessors listed below.)

The default-properties chain works by holding a (non-owning) pointer to another `Properties` instance: lookups that miss in this object fall through to the default object, recursively.

## 4. Q_PROPERTY Table

None. (`Properties` is not a `QObject`; "property" here refers to configuration entries, not Qt meta-object properties.)

## 5. Enumerations

None public. (A private `State` enum drives the line-parsing state machine in `parseProperty()`.)

## 6. Public Member Variables

None public. The default-chain pointer (`mpDefaultProperties`) is private and accessed via `defaultProperties()` / `setDefaultProperties()`.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

#### explicit Properties(Properties *pDefaultProperties = nullptr)

Constructs an empty map, optionally chained to `pDefaultProperties` as its fallback for unknown keys. The default object is referenced, not owned.

#### Properties *defaultProperties() const

Returns the chained default-properties object, or `nullptr` if none.

#### void setDefaultProperties(Properties *defaultProperties)

Sets (or clears, with `nullptr`) the default-properties object used for fallback lookups.

#### void setProperty(const QString &key, const QString &value)

Inserts or replaces the value for `key` in the contained hash. Equivalent to `insert(key, value)`.

#### void clear()

Removes all entries from this object's hash. Does not affect the chained default-properties object.

#### void insert(const QString &key, const QString &value)

Inserts or replaces the value for `key`. Synonym of `setProperty()`, provided for source compatibility with the former `QHash` base.

#### bool contains(const QString &key) const

Returns whether this object's hash holds `key`. Does **not** consult the default-properties chain (unlike `property()`).

#### QString value(const QString &key) const

Returns this object's stored value for `key`, or a default-constructed `QString` if absent. Does not consult the default chain. Marked `[[nodiscard]]`.

#### QString value(const QString &key, const QString &defaultValue) const

As above, returning `defaultValue` when `key` is absent from this object's hash. Marked `[[nodiscard]]`.

#### qsizetype count() const

Returns the number of entries in this object's hash. Marked `[[nodiscard]]`.

#### qsizetype size() const

Synonym of `count()`. Marked `[[nodiscard]]`.

#### bool isEmpty() const

Returns whether this object's hash has no entries. Marked `[[nodiscard]]`.

#### QList&lt;QString&gt; keys() const

Returns this object's keys (not including default-chain keys; use `propertyNames()` for the full reachable set). Marked `[[nodiscard]]`.

#### QString property(const QString &key) const

Looks up `key`. If this object contains the key, its value is returned (an empty `QString` rather than null is returned for a present-but-empty value, so an empty value is distinguishable from a missing one). If the key is absent, the lookup falls through to the default-properties object; if there is none, a *null* `QString` is returned to signal "not found".

#### QString property(const QString &key, const QString &defaultValue) const

As above, but returns `defaultValue` when the key resolves to a null string (i.e. is not found anywhere in the chain).

#### void load(QIODevice *pDevice)

Reads `java.util.Properties`-format text from `pDevice` and inserts the parsed entries. A `nullptr` device is logged as a warning and ignored. The reader supports:

- Leading whitespace trimming on each line.
- Line continuation: a line ending in a backslash (`\`) is joined with the following line.
- Comment lines beginning with `!` or `#` (detected during key parsing).
- `key = value` and `key : value` separators, as well as whitespace as a key/value separator.
- Escape sequences in keys (`\ `, `\:`, `\=`) and values (`\t`, `\n`, `\r`, `\\`, `\"`, `\'`, `\ `), plus `\uXXXX` Unicode escapes. Unknown escapes are logged as warnings and the character is taken literally.

#### void load(const QSettings &settings)

Reads all *child keys* of `settings` and inserts them, converting each value with `QVariant::toString()`. Values whose type does not support `toString()` become empty strings. Only direct child keys are read (group/section keys such as `Help/Language` are not flattened).

#### QStringList propertyNames() const

Returns the union of this object's keys and (recursively) the default object's keys, with duplicates removed — i.e. every key reachable through the fallback chain.

## 10. Protected Virtual Methods

None.

## 11. Ownership and Lifecycle

`Properties` owns its own key/value entries (in the contained `QHash` member). It does **not** own the default-properties object referenced through `setDefaultProperties()` / the constructor — that pointer is a non-owning reference, and the pointed-to object must outlive any `Properties` that chains to it. Devices and `QSettings` passed to `load()` are used transiently and not retained. As a value type (a `QHash` member plus a non-owning pointer), `Properties` is copyable and movable.

## 12. Thread Safety

`Properties` provides no internal synchronization. Like `QHash`, concurrent reads of an unmodified instance are safe, but any mutation (`load`, `setProperty`, `insert`) concurrent with another read or write requires external locking. Because the default-chain pointer is followed during reads, a chained default object must also be free of concurrent mutation while lookups are in progress.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

`Properties` is the configuration data structure consumed by the configurators (for example `PropertyConfigurator`), which call `load()` to populate it and then read entries to construct appenders and layouts. `OptionConverter::findAndSubst()` takes a `const Properties &` and resolves `${name}` references against it (falling back to `LOG4QT_*` environment variables); the other `OptionConverter::to*` helpers convert the resulting strings to typed values. Parse problems during `load()` are reported through the framework `Logger` as warnings.

## 15. External Communication

Inbound only. `load(QIODevice *)` reads configuration text from any `QIODevice` (typically a file on disk, but also buffers or sockets), and `load(const QSettings &)` reads from a `QSettings` backing store (registry, INI file, or platform-native store). The class itself opens no device — the caller supplies an already-opened `QIODevice` or a constructed `QSettings`.

## 16. Usage Example

```cpp
using namespace Log4Qt;

// Load from a .properties file:
QFile file(u"log4qt.properties"_s);
if (file.open(QIODevice::ReadOnly | QIODevice::Text))
{
    Properties props;
    props.load(&file);

    // Direct lookup distinguishes missing (null) from empty:
    const QString threshold = props.property(u"log4j.threshold"_s, u"DEBUG"_s);

    // Resolve ${...} references against the same map:
    const QString resolved = OptionConverter::findAndSubst(props, u"log4j.appender.A1.File"_s);
}

// Default-fallback chaining:
Properties systemDefaults;
systemDefaults.setProperty(u"log4j.rootLogger"_s, u"INFO, A1"_s);

Properties userProps(&systemDefaults);   // chains to systemDefaults
// userProps.property("log4j.rootLogger") falls back to "INFO, A1" if not overridden.

// Load from QSettings:
QSettings settings;
Properties fromSettings;
fromSettings.load(settings);
```
