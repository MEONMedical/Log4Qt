# Log4Qt Configuration Guide

Log4Qt supports three configuration file formats: **Properties**, **JSON**, and
**XML**. All three produce the same flat key/value model and are handled by the
same PropertyConfigurator engine, so every feature available in one format is
available in the others.

## File Discovery

During automatic initialization the LogManager searches for configuration files
in the following order at several locations (application binary path, application
directory, current working directory):

1. `log4qt.properties`
2. `log4qt.json`
3. `log4qt.xml`

The first file found wins. You can also point to a specific file via the
`LOG4QT_CONFIGURATION` environment variable or the `Log4Qt/Configuration`
QSettings entry.

---

## Configuration Model

The configuration is built from four sections:

| Section | Description |
|---------|-------------|
| **Global settings** | `reset`, `status`, `threshold`, etc. |
| **Appenders** | `appender.<alias>.*` |
| **Root logger** | `rootLogger.*` |
| **Named loggers** | `logger.<alias>.*` |

Processing order: global settings &rarr; appenders &rarr; root logger &rarr; named loggers.

---

## Global Settings

| Key | Type | Description |
|-----|------|-------------|
| `reset` | bool | Reset the logging configuration before applying new settings. |
| `status` | Level | Internal Log4Qt logging level (for debugging the logging framework itself). |
| `threshold` | Level | Repository-wide threshold; events below this level are discarded. |
| `handleQtMessages` | bool | Redirect `qDebug()` / `qWarning()` / etc. through Log4Qt. |
| `watchThisFile` | bool | Watch the configuration file and reconfigure on changes. |
| `filterRules` | string | Qt logging filter rules (semicolons are converted to newlines). |
| `messagePattern` | string | Qt message pattern for `qSetMessagePattern()`. |

**Valid levels:** `ALL`, `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`,
`OFF`, `NULL`.

---

## Appenders

Each appender is defined under the `appender.<alias>` prefix. The alias is an
arbitrary identifier used to reference the appender from loggers.

### Required Keys

| Key | Description |
|-----|-------------|
| `appender.<alias>.type` | Appender class name (see table below). |
| `appender.<alias>.layout.type` | Layout class name (required if the appender needs a layout). |

### Optional Keys

| Key | Description |
|-----|-------------|
| `appender.<alias>.name` | Display name (defaults to the alias). |
| `appender.<alias>.threshold` | Appender-level threshold. |
| `appender.<alias>.layout.<property>` | Layout properties (e.g. `conversionPattern`, `dateFormat`). |
| `appender.<alias>.filter.<falias>.type` | Filter class name. |
| `appender.<alias>.filter.<falias>.<property>` | Filter properties. |
| `appender.<alias>.<property>` | Any Qt property on the appender object (e.g. `file`, `target`). |

#### Property Value Conversion

Property values are strings and are converted to the property's declared type:
`bool` (`true`/`false`, `enabled`/`disabled`, `1`/`0`), `int`, `Log4Qt::Level`
(level name), `QString`, and `QStringConverter::Encoding`.

Any **enum or flag property** (`Q_ENUM` / `Q_FLAG`) is also supported: write the
enum **key name**, e.g. `caseSensitivity=CaseInsensitive` for the
`Qt::CaseSensitivity` property of `StringMatchFilter`. Flag properties accept
`Key1|Key2`. An unknown key is reported as a configuration error and the
property is left unchanged.

### Built-in Appender Types

| Short Name | Class | Description |
|------------|-------|-------------|
| `Console` | ConsoleAppender | Writes to stdout/stderr. |
| `File` | FileAppender | Writes to a single file. |
| `RollingFile` | RollingFileAppender | File with policy/strategy-based rotation. |
| `DailyFile` | DailyRollingFileAppender | Daily file with configurable retention. |
| `Async` | AsyncAppender | Asynchronous wrapper appender. |
| `MainThread` | MainThreadAppender | Dispatches to the main thread. |
| `Signal` | SignalAppender | Emits a Qt signal per log event. |
| `SystemLog` | SystemLogAppender | Writes to the system log (syslog / Event Log). |
| `Debug` | DebugAppender | Appender for debugging purposes. |
| `Null` | NullAppender | Discards all events. |
| `List` | ListAppender | Stores events in a list (for testing). |
| `ColorConsole` | ColorConsoleAppender | Coloured console output (Windows only). |
| `WDC` | WDCAppender | OutputDebugString (Windows only). |
| `Database` | DatabaseAppender | Writes to a SQL database (optional). |
| `Telnet` | TelnetAppender | Serves log events over telnet (optional). |

### Built-in Layout Types

| Short Name | Class | Description |
|------------|-------|-------------|
| `PatternLayout` | PatternLayout | Configurable pattern-based formatting. |
| `SimpleLayout` | SimpleLayout | `LEVEL - message` format. |
| `SimpleTimeLayout` | SimpleTimeLayout | Adds a timestamp to SimpleLayout. |
| `TTCCLayout` | TTCCLayout | Time, thread, category, context layout. |
| `XMLLayout` | XMLLayout | XML-formatted log events. |
| `JsonLayout` | JsonLayout | NDJSON output (one JSON object per line). |
| `DatabaseLayout` | DatabaseLayout | Layout for database appender (optional). |

### Built-in Filter Types

| Short Name | Class | Description |
|------------|-------|-------------|
| `DenyAll` | DenyAllFilter | Denies all events. |
| `LevelMatch` | LevelMatchFilter | Matches a specific level. Properties: `levelToMatch`, `acceptOnMatch`. |
| `LevelRange` | LevelRangeFilter | Matches a range of levels. Properties: `levelMin`, `levelMax`, `acceptOnMatch`. |
| `StringMatch` | StringMatchFilter | Matches a substring. Properties: `stringToMatch`, `acceptOnMatch`, `caseSensitivity` (`CaseSensitive` / `CaseInsensitive`, default `CaseSensitive`). |

### Triggering Policies (RollingFileAppender)

`RollingFileAppender` uses pluggable **triggering policies** to decide WHEN to
roll over and a **rollover strategy** to decide HOW. Multiple policies can be
attached (OR-combined automatically via `CompositeTriggeringPolicy`).

| Key | Description |
|-----|-------------|
| `appender.<alias>.policy.<palias>.type` | Triggering policy class name. |
| `appender.<alias>.policy.<palias>.<property>` | Policy properties. |
| `appender.<alias>.strategy.type` | Rollover strategy class name. |
| `appender.<alias>.strategy.<property>` | Strategy properties. |

If no strategy is specified, `DefaultRolloverStrategy` is used automatically.

### Built-in Triggering Policy Types

| Short Name | Class | Properties | Description |
|------------|-------|------------|-------------|
| `SizeBased` | SizeBasedTriggeringPolicy | `maxFileSize` (string, e.g. `10MB`), `maximumFileSize` (qint64) | Triggers when file size exceeds a threshold. Default: 10 MB. |
| `TimeBased` | TimeBasedTriggeringPolicy | `datePattern` (QString), `interval` (int, default 1), `modulate` (bool, default false), `maxRandomDelay` (int, default 0) | Triggers based on a date/time pattern. The rollover frequency (minutely through monthly) is automatically inferred from the pattern. `interval` multiplies the base frequency (e.g. interval=4 with hourly = every 4 hours). `modulate` aligns rollovers to calendar boundaries. `maxRandomDelay` adds random jitter in seconds to prevent thundering herd. Default pattern: `'.'yyyy-MM-dd` (daily). |
| `Cron` | CronTriggeringPolicy | `schedule` (QString) | Triggers on a Quartz-style cron schedule. 6-field format: `seconds minutes hours day-of-month month day-of-week`. Supports `*`, `?`, `,`, `-`, `/` specifiers and month/day-of-week names (JAN-DEC, SUN-SAT). Default: `0 0 0 * * ?` (midnight daily). |
| `OnStartup` | OnStartupTriggeringPolicy | _(none)_ | Triggers once at application startup if the log file already exists and is non-empty. |

### Built-in Rollover Strategy Types

| Short Name | Class | Properties | Description |
|------------|-------|------------|-------------|
| `Default` | DefaultRolloverStrategy | `minIndex` (int, default 1), `maxIndex` (int, default 7) | Fixed-window numbered rotation: deletes the oldest backup at `maxIndex`, shifts existing backups up by one, and renames the active file to `.minIndex`. |
| `Date` | DateRolloverStrategy | `datePattern` (QString, default `'.'yyyy-MM-dd`), `mode` (QString: `Suffix` or `Embedded`, default `Suffix`), `datedActiveFile` (bool, default `false`), `maxBackups` (int, default 0), `keepDays` (int, default 0) | Date-based rotation. In `Suffix` mode, renames the active file by appending a date suffix (e.g. `app.log.2026-03-28`). In `Embedded` mode, renames the active file to a date-embedded backup on rollover (e.g. `app_2026-03-28.log`). When `datedActiveFile=true`, the active file itself carries the date from the very first startup (built using `mode` — usually pair with `Embedded`), so each period writes directly to its own dated file and no rename happens on rollover. `maxBackups` limits retained backups (0 = unlimited); `keepDays` deletes backups older than N days (0 = disabled). |

### RollingFileAppender Examples

```properties
# Size-based rolling with 5 numbered backups
appender.rolling.type=RollingFile
appender.rolling.file=logs/app.log
appender.rolling.policy.SIZE.type=SizeBasedTriggeringPolicy
appender.rolling.policy.SIZE.maxFileSize=10MB
appender.rolling.strategy.type=DefaultRolloverStrategy
appender.rolling.strategy.maxIndex=5
appender.rolling.layout.type=PatternLayout
appender.rolling.layout.conversionPattern=%d %p %c - %m%n

# Cron-based rolling (midnight daily)
appender.cron.type=RollingFile
appender.cron.file=logs/app.log
appender.cron.policy.CRON.type=CronTriggeringPolicy
appender.cron.policy.CRON.schedule=0 0 0 * * ?
appender.cron.strategy.type=DefaultRolloverStrategy
appender.cron.strategy.maxIndex=30
appender.cron.layout.type=SimpleLayout

# Time-based rolling every 4 hours, aligned to clock boundaries, with jitter
appender.time4h.type=RollingFile
appender.time4h.file=logs/app.log
appender.time4h.policy.TIME.type=TimeBasedTriggeringPolicy
appender.time4h.policy.TIME.datePattern='.'yyyy-MM-dd-HH
appender.time4h.policy.TIME.interval=4
appender.time4h.policy.TIME.modulate=true
appender.time4h.policy.TIME.maxRandomDelay=30
appender.time4h.strategy.type=DefaultRolloverStrategy
appender.time4h.strategy.maxIndex=30
appender.time4h.layout.type=SimpleLayout

# Time-based rolling with date-suffix naming (replaces DailyRollingFileAppender)
appender.daily.type=RollingFile
appender.daily.file=logs/app.log
appender.daily.policy.TIME.type=TimeBasedTriggeringPolicy
appender.daily.policy.TIME.datePattern='.'yyyy-MM-dd
appender.daily.strategy.type=Date
appender.daily.strategy.datePattern='.'yyyy-MM-dd
appender.daily.strategy.maxBackups=30
appender.daily.layout.type=SimpleLayout

# Time-based rolling with date-in-filename naming (replaces DailyRollingFileAppender)
appender.embed.type=RollingFile
appender.embed.file=logs/app.log
appender.embed.policy.TIME.type=TimeBasedTriggeringPolicy
appender.embed.policy.TIME.datePattern=_yyyy_MM_dd
appender.embed.policy.STARTUP.type=OnStartupTriggeringPolicy
appender.embed.strategy.type=Date
appender.embed.strategy.datePattern=_yyyy_MM_dd
appender.embed.strategy.mode=Embedded
appender.embed.strategy.maxBackups=90
appender.embed.layout.type=SimpleLayout

# Dated active file: each day writes directly to its own file from startup
# (no rename on rollover). The configured `file` acts as a template; with
# mode=Embedded the date is inserted before the extension, so `logs/app.log`
# becomes `logs/app_2026-04-20.log` on 2026-04-20.
appender.dated.type=RollingFile
appender.dated.file=logs/app.log
appender.dated.policy.TIME.type=TimeBasedTriggeringPolicy
appender.dated.policy.TIME.datePattern=_yyyy_MM_dd
appender.dated.strategy.type=Date
appender.dated.strategy.datePattern=_yyyy_MM_dd
appender.dated.strategy.mode=Embedded
appender.dated.strategy.datedActiveFile=true
appender.dated.strategy.keepDays=30
appender.dated.layout.type=SimpleLayout

# Multiple policies (size + startup, OR-combined automatically)
appender.multi.type=RollingFile
appender.multi.file=logs/app.log
appender.multi.policy.SIZE.type=SizeBasedTriggeringPolicy
appender.multi.policy.SIZE.maxFileSize=50MB
appender.multi.policy.STARTUP.type=OnStartupTriggeringPolicy
appender.multi.strategy.type=DefaultRolloverStrategy
appender.multi.strategy.maxIndex=10
appender.multi.layout.type=SimpleLayout
```

---

## Header/Footer Providers

A `HeaderFooterProvider` supplies a **header** string written when a log file is
opened and a **footer** string written when it is closed. This is useful for
embedding session metadata — timestamps, software version, device serial number
— in every log file without hardcoding text in the layout.

### Priority Chain

For each `header()` or `footer()` call on a layout, sources are checked in order
and the first non-empty result wins:

1. **Per-layout provider** (`appender.X.layout.headerFooterProvider.*`)
2. **PatternLayout `headerPattern` / `footerPattern`** (PatternLayout only)
3. **Static `header` / `footer` string** on the layout
4. **Global provider** (`headerFooterProvider.*`)

### Global Provider

The global provider acts as a fallback for every layout that has no other
header/footer configured.

```properties
headerFooterProvider.type=Pattern
headerFooterProvider.headerPattern=--- Started %d{yyyy-MM-dd HH:mm:ss} ---
headerFooterProvider.footerPattern=--- Stopped %d{yyyy-MM-dd HH:mm:ss} ---
```

### Per-Layout Provider

A per-layout provider is attached to one specific appender's layout. It takes
precedence over the global provider and over `PatternLayout` patterns.

```properties
appender.json.layout.headerFooterProvider.type=Pattern
appender.json.layout.headerFooterProvider.headerPattern={"start":"%d{yyyy-MM-ddTHH:mm:ss}"}
appender.json.layout.headerFooterProvider.footerPattern={"end":"%d{yyyy-MM-ddTHH:mm:ss}"}
```

### Built-in Provider Types

| Alias | Class | Properties | Description |
|-------|-------|------------|-------------|
| `PatternHeaderFooterProvider`, `Pattern` | PatternHeaderFooterProvider | `headerPattern`, `footerPattern` | Formats header/footer with conversion patterns evaluated at file-open/close time. Supports all `PatternLayout` specifiers plus `%P{key}`. |

### Conversion Specifiers in Patterns

`PatternHeaderFooterProvider` accepts the same conversion specifiers as
`PatternLayout`. Patterns are evaluated at file-open / file-close time so `%d`
reflects the actual timestamp:

| Specifier | Description |
|-----------|-------------|
| `%d{format}` | Current date/time at file open/close. Uses the same format strings as `PatternLayout` (`ISO8601`, `yyyy-MM-dd HH:mm:ss`, etc.). |
| `%P{key}` | Value of the named property on the provider object (see below). |
| `%r`, `%t`, `%%`, `%n`, … | All standard `PatternLayout` specifiers. |

### Injecting Runtime Values with `%P{key}`

`%P{key}` resolves the named key through Qt's property system
(`QObject::property(key)`) on the provider at format time. Both static
`Q_PROPERTY` members and dynamic properties set via `QObject::setProperty()`
are supported.

**Option 1 — Subclass with `Q_PROPERTY` (recommended for factory-created providers)**

Define a provider subclass with the data you want to expose:

```cpp
class MyProvider : public Log4Qt::PatternHeaderFooterProvider {
    Q_OBJECT
    Q_PROPERTY(QString serialNumber READ serialNumber WRITE setSerialNumber)
public:
    QString serialNumber() const { return mSn; }
    void setSerialNumber(const QString &v) { mSn = v; }
private:
    QString mSn;
};
```

Register with the Factory before loading the config, capturing the runtime
value in the lambda so it is available before `activateOptions()` writes the
header:

```cpp
const QString sn = readSerialFromHardware();
Log4Qt::Factory::registerHeaderFooterProvider(
    "MyProvider",
    [sn]() -> Log4Qt::HeaderFooterProvider * {
        auto *p = new MyProvider;
        p->setSerialNumber(sn);
        return p;
    });
Log4Qt::PropertyConfigurator::configureAndWatch(configFile);
```

Config file — the pattern and any other `Q_PROPERTY` values are declared here;
the serial number is injected by the application:

```properties
headerFooterProvider.type=MyProvider
headerFooterProvider.headerPattern=S/N: %P{serialNumber} — started %d{yyyy-MM-dd HH:mm:ss}
headerFooterProvider.footerPattern=S/N: %P{serialNumber} — stopped %d{yyyy-MM-dd HH:mm:ss}
```

**Option 2 — Dynamic property (no subclass)**

```cpp
auto *p = new Log4Qt::PatternHeaderFooterProvider;
p->setHeaderPattern(u"S/N: %P{serialNumber}"_s);
p->setProperty("serialNumber", u"SN-001"_s);   // QObject::setProperty
Log4Qt::AbstractLayout::setGlobalHeaderFooterProvider(
    Log4Qt::HeaderFooterProviderSharedPtr(p));
```

### Custom Provider

Implement `HeaderFooterProvider` (or `PatternHeaderFooterProvider`) and register
it with the Factory before the configuration file is loaded:

```cpp
Log4Qt::Factory::registerHeaderFooterProvider(
    "MyCustomProvider",
    []() -> Log4Qt::HeaderFooterProvider * {
        return new MyCustomProvider;
    });
```

Then reference it by name in the config file. All `Q_PROPERTY` members are set
automatically from matching configuration keys — the same mechanism used for
appenders and layouts:

```properties
headerFooterProvider.type=MyCustomProvider
headerFooterProvider.myProperty=value
```

### Complete Example

```properties
# Global provider: plain text header/footer for all log files
headerFooterProvider.type=Pattern
headerFooterProvider.headerPattern=### Session started %d{yyyy-MM-dd HH:mm:ss} ###
headerFooterProvider.footerPattern=### Session stopped %d{yyyy-MM-dd HH:mm:ss} ###

# JSON appender with a per-layout provider for machine-readable records
appender.json.type=File
appender.json.file=${logpath}/app.json
appender.json.layout.type=JsonLayout
appender.json.layout.headerFooterProvider.type=Pattern
appender.json.layout.headerFooterProvider.headerPattern={"event":"start","time":"%d{yyyy-MM-ddTHH:mm:ss}"}
appender.json.layout.headerFooterProvider.footerPattern={"event":"end","time":"%d{yyyy-MM-ddTHH:mm:ss}"}
```

---

## Root Logger

| Key | Description |
|-----|-------------|
| `rootLogger.level` | Log level for the root logger. |
| `rootLogger.appenderRef.<alias>.ref` | Name of an appender to attach. |

You can attach multiple appenders by using different aliases:

```properties
rootLogger.appenderRef.console.ref=console
rootLogger.appenderRef.daily.ref=daily
```

---

## Named Loggers

Each named logger is defined under the `logger.<alias>` prefix.

| Key | Description |
|-----|-------------|
| `logger.<alias>.name` | The actual logger name (required). |
| `logger.<alias>.level` | Log level (`INHERITED` to inherit from parent). |
| `logger.<alias>.additivity` | `true` (default) or `false`. |
| `logger.<alias>.appenderRef.<refalias>.ref` | Appender to attach. |

The alias is an arbitrary identifier; the `name` property is the real logger
name used at runtime. This separation allows logger names that contain dots or
special characters.

---

## Variable Substitution

String values support `${key}` substitution. The key is looked up in the
properties themselves first, then in environment variables prefixed with
`LOG4QT_`.

```properties
logpath=/var/log/myapp
appender.daily.file=${logpath}/app.log
```

---

## Properties Format

Standard Java-style `.properties` file. Lines starting with `#` or `!` are
comments. Keys and values are separated by `=` or `:`.

### Complete Example

```properties
# Variable for log directory
logpath=./logs

# Global settings
reset=true
status=WARN
threshold=NULL
handleQtMessages=true
watchThisFile=false

# Console appender
appender.console.type=Console
appender.console.target=STDOUT_TARGET
appender.console.threshold=ALL
appender.console.layout.type=TTCCLayout
appender.console.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz
appender.console.layout.contextPrinting=true

# Daily rolling file appender
appender.daily.type=DailyFile
appender.daily.file=${logpath}/app.log
appender.daily.appendFile=true
appender.daily.datePattern=_yyyy_MM_dd
appender.daily.keepDays=90
appender.daily.layout.type=TTCCLayout
appender.daily.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz
appender.daily.layout.contextPrinting=true

# Rolling file with size-based policy and filter
appender.rolling.type=RollingFile
appender.rolling.file=${logpath}/errors.log
appender.rolling.policy.SIZE.type=SizeBasedTriggeringPolicy
appender.rolling.policy.SIZE.maxFileSize=10MB
appender.rolling.strategy.type=DefaultRolloverStrategy
appender.rolling.strategy.maxIndex=5
appender.rolling.layout.type=PatternLayout
appender.rolling.layout.conversionPattern=%-5p %d{ISO8601} [%t] %c - %m%n
appender.rolling.filter.level.type=LevelMatch
appender.rolling.filter.level.levelToMatch=ERROR
appender.rolling.filter.level.acceptOnMatch=true

# Root logger
rootLogger.level=ALL
rootLogger.appenderRef.console.ref=console
rootLogger.appenderRef.daily.ref=daily

# Named loggers
logger.db.name=Database
logger.db.level=ERROR
logger.db.additivity=false
logger.db.appenderRef.rolling.ref=rolling

logger.network.name=Network
logger.network.level=WARN
```

---

## JSON Format

The JSON structure maps directly to the flat property keys. Nested objects
produce dot-separated keys. Boolean and numeric values are automatically
stringified, and `null` becomes an empty value.

**Arrays** flatten to numeric key segments, exactly as an object with `"0"`,
`"1"`, … keys would. So an appender reference list can be written naturally:

```json
"rootLogger": { "level": "ALL", "appenderRef": [ { "ref": "console" } ] }
```

which produces `rootLogger.appenderRef.0.ref=console`. The object form with
named aliases (as in the complete example below) remains equally valid.

> **Selecting the appender/layout/filter class:** use the `type` key — **not**
> `class` or `@Class`. Unlike Log4j's XML `class="..."` attribute, Log4Qt uses a
> `type` key in every format. So write `"type": "org.apache.log4j.ConsoleAppender"`
> (the legacy `org.apache.log4j.*` names, the `Log4Qt::*` names, and the short
> aliases such as `Console` are all accepted). Any other key — including `@Class`
> — is treated as an ordinary property and ignored for class selection, which
> produces a *"Missing appender type"* error.
>
> The Log4j1 class-as-value form (`log4j.appender.X = classname`) cannot be
> expressed in JSON, because a JSON object cannot be both a string and hold
> nested keys. The `type` key is the JSON-native replacement.

### Complete Example

```json
{
    "logpath": "./logs",
    "reset": "true",
    "status": "WARN",
    "threshold": "NULL",
    "handleQtMessages": "true",
    "watchThisFile": "false",
    "appender": {
        "console": {
            "type": "Console",
            "target": "STDOUT_TARGET",
            "threshold": "ALL",
            "layout": {
                "type": "TTCCLayout",
                "dateFormat": "dd.MM.yyyy hh:mm:ss.zzz",
                "contextPrinting": true
            }
        },
        "daily": {
            "type": "DailyFile",
            "file": "${logpath}/app.log",
            "appendFile": "true",
            "datePattern": "_yyyy_MM_dd",
            "keepDays": 90,
            "layout": {
                "type": "TTCCLayout",
                "dateFormat": "dd.MM.yyyy hh:mm:ss.zzz",
                "contextPrinting": true
            }
        },
        "rolling": {
            "type": "RollingFile",
            "file": "${logpath}/errors.log",
            "policy": {
                "SIZE": {
                    "type": "SizeBasedTriggeringPolicy",
                    "maxFileSize": "10MB"
                }
            },
            "strategy": {
                "type": "DefaultRolloverStrategy",
                "maxIndex": 5
            },
            "layout": {
                "type": "PatternLayout",
                "conversionPattern": "%-5p %d{ISO8601} [%t] %c - %m%n"
            },
            "filter": {
                "level": {
                    "type": "LevelMatch",
                    "levelToMatch": "ERROR",
                    "acceptOnMatch": "true"
                }
            }
        }
    },
    "rootLogger": {
        "level": "ALL",
        "appenderRef": {
            "console": { "ref": "console" },
            "daily": { "ref": "daily" }
        }
    },
    "logger": {
        "db": {
            "name": "Database",
            "level": "ERROR",
            "additivity": "false",
            "appenderRef": {
                "rolling": { "ref": "rolling" }
            }
        },
        "network": {
            "name": "Network",
            "level": "WARN"
        }
    }
}
```

---

## XML Format

The XML structure uses element names as key segments. The root element (e.g.
`<configuration>`) is skipped; its children become top-level keys. XML
attributes on an element are flattened as child properties.

> Element names are used **verbatim** — they are never translated — so the
> Log4j2 XML dialect (`<Configuration><Appenders><Console name="...">`) is not
> supported: it flattens to keys Log4Qt ignores and configures nothing. Write
> the elements as the property keys themselves, as in the example below.

> **Selecting the appender/layout/filter class:** use a `type` element — **not**
> a `class` attribute as in Log4j's XML format. Write
> `<type>org.apache.log4j.ConsoleAppender</type>`, not
> `<console class="org.apache.log4j.ConsoleAppender">`. Because attributes
> flatten to child properties, `type="..."` as an attribute works too
> (`<console type="Console">`), but `class="..."` does not — it becomes a
> `.class` property and is ignored for class selection.

### Complete Example

```xml
<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <logpath>./logs</logpath>
    <reset>true</reset>
    <status>WARN</status>
    <threshold>NULL</threshold>
    <handleQtMessages>true</handleQtMessages>
    <watchThisFile>false</watchThisFile>

    <appender>
        <console>
            <type>Console</type>
            <target>STDOUT_TARGET</target>
            <threshold>ALL</threshold>
            <layout>
                <type>TTCCLayout</type>
                <dateFormat>dd.MM.yyyy hh:mm:ss.zzz</dateFormat>
                <contextPrinting>true</contextPrinting>
            </layout>
        </console>
        <daily>
            <type>DailyFile</type>
            <file>${logpath}/app.log</file>
            <appendFile>true</appendFile>
            <datePattern>_yyyy_MM_dd</datePattern>
            <keepDays>90</keepDays>
            <layout>
                <type>TTCCLayout</type>
                <dateFormat>dd.MM.yyyy hh:mm:ss.zzz</dateFormat>
                <contextPrinting>true</contextPrinting>
            </layout>
        </daily>
        <rolling>
            <type>RollingFile</type>
            <file>${logpath}/errors.log</file>
            <policy>
                <SIZE>
                    <type>SizeBasedTriggeringPolicy</type>
                    <maxFileSize>10MB</maxFileSize>
                </SIZE>
            </policy>
            <strategy>
                <type>DefaultRolloverStrategy</type>
                <maxIndex>5</maxIndex>
            </strategy>
            <layout>
                <type>PatternLayout</type>
                <conversionPattern>%-5p %d{ISO8601} [%t] %c - %m%n</conversionPattern>
            </layout>
            <filter>
                <level>
                    <type>LevelMatch</type>
                    <levelToMatch>ERROR</levelToMatch>
                    <acceptOnMatch>true</acceptOnMatch>
                </level>
            </filter>
        </rolling>
    </appender>

    <rootLogger>
        <level>ALL</level>
        <appenderRef>
            <console>
                <ref>console</ref>
            </console>
            <daily>
                <ref>daily</ref>
            </daily>
        </appenderRef>
    </rootLogger>

    <logger>
        <db>
            <name>Database</name>
            <level>ERROR</level>
            <additivity>false</additivity>
            <appenderRef>
                <rolling>
                    <ref>rolling</ref>
                </rolling>
            </appenderRef>
        </db>
        <network>
            <name>Network</name>
            <level>WARN</level>
        </network>
    </logger>
</configuration>
```

---

## Legacy Log4j1 Format (Backward Compatibility)

Property files using the old Log4j1-style format (keys prefixed with `log4j.`)
are automatically detected and translated to the new format. No configuration
changes are needed — existing property files continue to work.

**Note:** Mixing legacy and new-format keys in the same file is not supported.

### Key Mapping

| Legacy Key | New Key |
|------------|---------|
| `log4j.appender.X=classname` | `appender.X.type=classname` |
| `log4j.appender.X.layout=classname` | `appender.X.layout.type=classname` |
| `log4j.appender.X.filter.F=classname` | `appender.X.filter.F.type=classname` |
| `log4j.appender.X.<prop>=value` | `appender.X.<prop>=value` |
| `log4j.rootLogger=LEVEL, A1, A2` | `rootLogger.level=LEVEL` + `rootLogger.appenderRef.N.ref=AN` |
| `log4j.rootCategory=...` | Same as `rootLogger` (deprecated alias) |
| `log4j.logger.name=LEVEL, A1` | `logger.<alias>.name=name` + `.level` + `.appenderRef` |
| `log4j.category.name=...` | Same as `logger` (deprecated alias) |
| `log4j.additivity.name=bool` | `logger.<alias>.additivity=bool` |
| `log4j.Debug` / `log4j.configDebug` | `status` |
| `log4j.reset` | `reset` |
| `log4j.threshold` | `threshold` |
| `log4j.handleQtMessages` | `handleQtMessages` |
| `log4j.watchThisFile` | `watchThisFile` |
| `log4j.qtLogging.filterRules` | `filterRules` |
| `log4j.qtLogging.messagePattern` | `messagePattern` |

### Legacy Format Example

```properties
# Variable (no log4j. prefix — passed through as-is)
logpath=./logs

# Global settings
log4j.reset=true
log4j.Debug=WARN
log4j.threshold=NULL
log4j.handleQtMessages=true

# Appender (class name as value)
log4j.appender.console=org.apache.log4j.ConsoleAppender
log4j.appender.console.target=STDOUT_TARGET
log4j.appender.console.layout=org.apache.log4j.TTCCLayout
log4j.appender.console.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz

log4j.appender.daily=Log4Qt::DailyRollingFileAppender
log4j.appender.daily.file=${logpath}/app.log
log4j.appender.daily.appendFile=true
log4j.appender.daily.datePattern=_yyyy_MM_dd
log4j.appender.daily.layout=org.apache.log4j.TTCCLayout

# Root logger (level + comma-separated appender list)
log4j.rootLogger=ALL, console, daily

# Named logger
log4j.logger.MyApp=ERROR, console
log4j.additivity.MyApp=false
```

---

## Programmatic Configuration via QSettings

You can also configure Log4Qt from QSettings. The keys are stored under the
`Log4Qt/Properties` group:

```cpp
QSettings s;
s.beginGroup("Log4Qt");
s.setValue("Debug", "TRACE");
s.beginGroup("Properties");
s.setValue("appender.A1.type", "File");
s.setValue("appender.A1.file", "C:/myapp.log");
s.setValue("appender.A1.layout.type", "TTCCLayout");
s.setValue("appender.A1.layout.dateFormat", "ISO8601");
s.setValue("rootLogger.level", "TRACE");
s.setValue("rootLogger.appenderRef.0.ref", "A1");
```

---

## Custom Components

Register custom appenders, layouts, filters, triggering policies, or rollover
strategies with the Factory before configuration:

```cpp
Factory::registerAppender("MyAppender", []() -> Appender* {
    return new MyAppender;
});
Factory::registerTriggeringPolicy("MyPolicy", []() -> TriggeringPolicy* {
    return new MyTriggeringPolicy;
});
Factory::registerRolloverStrategy("MyStrategy", []() -> RolloverStrategy* {
    return new MyRolloverStrategy;
});
```

Then use the registered name in any configuration file:

```properties
appender.custom.type=MyAppender
appender.custom.myProperty=value
appender.custom.policy.P.type=MyPolicy
appender.custom.strategy.type=MyStrategy
```

Properties are set via Qt's meta-object system, so any `Q_PROPERTY` declared
on your class is automatically available as a configuration key.
