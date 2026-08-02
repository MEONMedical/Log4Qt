# Factory

## Class Overview

`Factory` is a process-wide singleton registry that creates Log4Qt building blocks — appenders, filters, layouts, triggering policies, rollover strategies, and header/footer providers — from their class names. Configurators use it to instantiate objects described in a configuration file: a class name such as `"org.apache.log4j.FileAppender"`, `"Log4Qt::FileAppender"`, or the short alias `"File"` is mapped to a factory function that heap-allocates the corresponding object.

The factory ships with default registrations for all built-in classes (under both their Log4j and Log4Qt names plus short aliases) and lets applications register, replace, and unregister their own factory functions at runtime. It also provides `setObjectProperty()`, which sets a `QObject` property from a string value with the type checking and conversion needed when applying string-based configuration.

## Project Structure and Dependencies

- Header: `src/log4qt/helpers/factory.h`
- Source: `src/log4qt/helpers/factory.cpp`
- Part of the `log4qt` library target (see `src/log4qt/CMakeLists.txt`, `log4qt_HEADERS_helpers`).

Dependencies:

- `QHash` — the per-category name→factory registries.
- `QMutex` — guards all registry state.
- `QStringList` — returned by the `registered…()` query methods.
- `<functional>` — `std::function` backing `HeaderFooterProviderFactoryFunc`.
- `QMetaObject` / `QMetaProperty` (cpp) — used by `setObjectProperty()` for property reflection.
- The concrete product headers (cpp): console/file/rolling/daily/list/null/signal/async/mainthread/systemlog appenders, optional database/telnet/Windows appenders, the layouts, the SPI triggering policies, rollover strategies, header/footer providers, and the `varia` filters.
- `helpers/optionconverter.h` (cpp) — string-to-type conversions in `setObjectProperty()`.
- `helpers/logerror.h` (cpp) — structured error reporting.
- `helpers/initialisationhelper.h` (cpp) — provides `LOG4QT_IMPLEMENT_INSTANCE`.

Conditional product registration depends on `LOG4QT_DB_LOGGING_SUPPORT`, `LOG4QT_TELNET_LOGGING_SUPPORT`, and `Q_OS_WIN`.

## Class Hierarchy and Role

`Factory` is a standalone (non-`QObject`) singleton: the constructor is private (it registers all defaults), copy/move are deleted via `Q_DISABLE_COPY_MOVE`, and the instance is created by `LOG4QT_IMPLEMENT_INSTANCE`. The entire public API is `static` and forwards to the single instance.

## Type Aliases

| Alias | Underlying type | Description |
|-------|-----------------|-------------|
| `AppenderFactoryFunc` | `Appender *(*)()` | Creates a heap `Appender` and returns it. |
| `FilterFactoryFunc` | `Filter *(*)()` | Creates a heap `Filter` and returns it. |
| `LayoutFactoryFunc` | `AbstractLayout *(*)()` | Creates a heap `AbstractLayout` and returns it. |
| `TriggeringPolicyFactoryFunc` | `TriggeringPolicy *(*)()` | Creates a heap `TriggeringPolicy` and returns it. |
| `RolloverStrategyFactoryFunc` | `RolloverStrategy *(*)()` | Creates a heap `RolloverStrategy` and returns it. |
| `HeaderFooterProviderFactoryFunc` | `std::function<HeaderFooterProvider *()>` | Creates a heap `HeaderFooterProvider`. Uses `std::function`, so stateful callables/lambdas are accepted. |

## Public Methods

All public methods are `static`. Each `create…` and `register…`/`unregister…` method has an overload taking either a `QString` or a `const char *` class name.

### Creating objects

#### static Appender *createAppender(const QString &appenderClassName)
#### static Appender *createAppender(const char *appenderClassName)

Creates an `Appender` for the registered class name on the heap and returns it. Returns `nullptr` (and logs a warning) if no factory is registered for the name.

#### static Filter *createFilter(const QString &filterClassName)
#### static Filter *createFilter(const char *filterClassName)

Creates a `Filter` for the registered class name, or `nullptr` if unregistered.

#### static AbstractLayout *createLayout(const QString &layoutClassName)
#### static AbstractLayout *createLayout(const char *layoutClassName)

Creates an `AbstractLayout` for the registered class name, or `nullptr` if unregistered.

#### static TriggeringPolicy *createTriggeringPolicy(const QString &className)
#### static TriggeringPolicy *createTriggeringPolicy(const char *className)

Creates a `TriggeringPolicy`, or `nullptr` if unregistered.

#### static RolloverStrategy *createRolloverStrategy(const QString &className)
#### static RolloverStrategy *createRolloverStrategy(const char *className)

Creates a `RolloverStrategy`, or `nullptr` if unregistered.

#### static HeaderFooterProvider *createHeaderFooterProvider(const QString &className)
#### static HeaderFooterProvider *createHeaderFooterProvider(const char *className)

Creates a `HeaderFooterProvider`, or `nullptr` if unregistered.

### Registering / unregistering

#### static void registerAppender(const QString &appenderClassName, AppenderFactoryFunc appenderFactoryFunc)
#### static void registerAppender(const char *appenderClassName, AppenderFactoryFunc appenderFactoryFunc)

Registers (or replaces) the appender factory for the given class name. A registration with an empty class name is rejected with a warning.

#### static void registerFilter(const QString &filterClassName, FilterFactoryFunc filterFactoryFunc)
#### static void registerFilter(const char *filterClassName, FilterFactoryFunc filterFactoryFunc)

Registers (or replaces) a filter factory. Empty name rejected.

#### static void registerLayout(const QString &layoutClassName, LayoutFactoryFunc layoutFactoryFunc)
#### static void registerLayout(const char *layoutClassName, LayoutFactoryFunc layoutFactoryFunc)

Registers (or replaces) a layout factory. Empty name rejected.

#### static void registerTriggeringPolicy(const QString &className, TriggeringPolicyFactoryFunc func)
#### static void registerTriggeringPolicy(const char *className, TriggeringPolicyFactoryFunc func)

Registers (or replaces) a triggering-policy factory. Empty name rejected.

#### static void registerRolloverStrategy(const QString &className, RolloverStrategyFactoryFunc func)
#### static void registerRolloverStrategy(const char *className, RolloverStrategyFactoryFunc func)

Registers (or replaces) a rollover-strategy factory. Empty name rejected.

#### static void registerHeaderFooterProvider(const QString &className, HeaderFooterProviderFactoryFunc func)
#### static void registerHeaderFooterProvider(const char *className, HeaderFooterProviderFactoryFunc func)

Registers (or replaces) a header/footer-provider factory. Empty name rejected.

#### static void unregisterAppender(const QString &appenderClassName)
#### static void unregisterAppender(const char *appenderClassName)

Removes the appender factory for the class name. Logs a warning if it was not registered.

#### static void unregisterFilter(const QString &filterClassName)
#### static void unregisterFilter(const char *filterClassName)

Removes a filter factory; warns if absent.

#### static void unregisterLayout(const QString &layoutClassName)
#### static void unregisterLayout(const char *layoutClassName)

Removes a layout factory; warns if absent.

#### static void unregisterTriggeringPolicy(const QString &className)
#### static void unregisterTriggeringPolicy(const char *className)

Removes a triggering-policy factory; warns if absent.

#### static void unregisterRolloverStrategy(const QString &className)
#### static void unregisterRolloverStrategy(const char *className)

Removes a rollover-strategy factory; warns if absent.

#### static void unregisterHeaderFooterProvider(const QString &className)
#### static void unregisterHeaderFooterProvider(const char *className)

Removes a header/footer-provider factory; warns if absent.

### Querying registrations

#### static QStringList registeredAppenders()
#### static QStringList registeredFilters()
#### static QStringList registeredLayouts()
#### static QStringList registeredTriggeringPolicies()
#### static QStringList registeredRolloverStrategies()
#### static QStringList registeredHeaderFooterProviders()

Each returns the list of class names with a registered factory in the corresponding category (snapshot under the mutex; order unspecified).

### Property setting

#### static void setObjectProperty(QObject *object, const QString &property, const QString &value)
#### static void setObjectProperty(QObject *object, const char *property, const QString &value)

Sets the named property of `object` to `value`, performing validation and string-to-type conversion. Behaviour:

- Validates that `object` is non-null, the property name is non-empty, the property exists, and is writable. If the exact name is not found it retries with the first character lower-cased (Java property names are upper-case-first), accommodating Log4j-style configuration keys.
- Converts `value` based on the property's declared type. Named types: `bool`, `int`, `Log4Qt::Level`, `QString`, and `QStringConverter::Encoding` (converted via `OptionConverter`).
- **Any `Q_ENUM`/`Q_FLAG` property** is handled by a generic fallback: when `QMetaProperty::isEnumType()` is true, the string is resolved through the property's `QMetaEnum` — `keysToValue()` for flags (so `"A|B"` works), `keyToValue()` otherwise. Values are therefore written as the enum **key name**, e.g. `caseSensitivity=CaseInsensitive` for the `Qt::CaseSensitivity` property of `StringMatchFilter`. An unknown key (or an invalid `QMetaEnum`) logs a `ConfiguratorUnknownTypeError` naming the value, type, property and class, and nothing is written.
- Any other type produces a logged `LogError` and no write.
- On a successful conversion, writes the value via `QMetaProperty::write()`.

### Singleton accessor

#### static Factory *instance()

Returns the singleton instance, creating it (and registering all defaults) on first use.

## Default Registrations

The private constructor registers all built-in products via `registerDefault…()` helpers. Each product is registered under multiple keys — the Apache `org.apache.log4j.*` name, the `Log4Qt::*` name, and a short alias. Examples:

- Appenders: `Console`, `Debug`, `File`, `List`, `Null`, `RollingFile`, `Signal`, `Async`, `MainThread`, `SystemLog`, `DailyFile`, plus `Database`/`Telnet` (when compiled in) and `ColorConsole`/`WDC` (Windows).
- Filters: `DenyAll`, `LevelMatch`, `LevelRange`, `StringMatch`.
- Layouts: `PatternLayout`, `SimpleLayout`, `TTCCLayout`, `SimpleTimeLayout`, `XMLLayout`, `JsonLayout`, plus `DatabaseLayout` (when compiled in).
- Triggering policies: `SizeBased`, `TimeBased`, `Cron`, `OnStartup`.
- Rollover strategies: `Default`, `Date`.
- Header/footer providers: `Pattern`.

## Ownership and Lifecycle

`Factory` is a process singleton created on first `instance()` use and intended to live for the duration of the process. It does **not** own the objects it creates: each `create…` method returns a raw owning pointer that the caller (typically a configurator, which wraps appenders in `AppenderSharedPtr` or parents `QObject`s appropriately) is responsible for managing. The registries store factory function pointers / `std::function`s, not instances.

## Thread Safety

All public functions are thread-safe. A single non-recursive `QMutex` (`mObjectGuard`) guards every registry. Create, register, unregister, and query methods each lock the mutex for the duration of the operation. Note that the user-supplied factory functions themselves run while the mutex is held inside the `create…` methods; they should not call back into `Factory` to avoid self-deadlock on the non-recursive mutex.

## Inter-Class Interactions

- Configurators (`PropertyConfigurator`, `JsonConfigurator`, `XmlConfigurator`) drive `Factory` to instantiate the objects named in a configuration and call `setObjectProperty()` to populate their properties.
- `setObjectProperty()` relies on `OptionConverter` for type conversion and on `LogError` for error reporting.
- Uses `LOG4QT_IMPLEMENT_INSTANCE` from `InitialisationHelper`.
- A static logger (`LOG4QT_DECLARE_STATIC_LOGGER`) named after `Log4Qt::Factory` emits warnings/errors for unknown classes, failed conversions, and unwritable properties.

## Usage Example

```cpp
// Create a built-in appender by its short alias and configure it from strings.
Log4Qt::Appender *appender = Log4Qt::Factory::createAppender(QStringLiteral("File"));
if (appender)
{
    Log4Qt::Factory::setObjectProperty(appender, "file", QStringLiteral("/var/log/app.log"));
    Log4Qt::Factory::setObjectProperty(appender, "threshold", QStringLiteral("INFO"));
}

// Register a custom appender factory under a custom name.
Log4Qt::Factory::registerAppender(QStringLiteral("MyAppender"),
                                  []() -> Log4Qt::Appender * { return new MyAppender; });

// Later, query and remove it.
const QStringList names = Log4Qt::Factory::registeredAppenders();
Log4Qt::Factory::unregisterAppender(QStringLiteral("MyAppender"));
```
