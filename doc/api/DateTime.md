# DateTime

## 1. Class Overview

Log4Qt is a Qt port of the Apache log4j logging library. Every `LoggingEvent` carries a timestamp, and layouts must render that timestamp as text — often for high-volume log streams. `DateTime` is the helper that makes this fast and consistent.

`DateTime` extends `QDateTime` with the timestamp formats log4j uses (`ABSOLUTE`, `DATE`, `ISO8601`, `NONE`, `RELATIVE`), a fast static formatter that works directly from an epoch-millisecond value, a thread-local cache for `currentMSecsSinceEpoch()`, and an injectable global time provider for testing. A developer reaches for `DateTime` whenever a log timestamp needs to be produced or formatted, or whenever code needs a current-time reading that is both cheap and substitutable in tests.

## 2. Project Structure and Dependencies

- **Used by:** layout classes that render the event timestamp (date/time conversion patterns), `LoggingEvent` timestamp handling, and time-based rollover components such as `DailyRollingFileAppender`, `DateRolloverStrategy`, and `CronTriggeringPolicy` (which feeds `DateTime::currentDateTime()` into `CronExpression::nextFireTime()`).
- **Qt module dependency:** Qt Core — `QDateTime`, `QDate`, `QTime`, `QTimeZone`, `QElapsedTimer`, `QReadWriteLock`, `QString`.
- **Standard library:** `<functional>` (`std::function` for the `Provider` type), `<atomic>` (`std::atomic<qint64>` cache-window state).
- **Project headers:** `log4qt/log4qtshared.h` (the `LOG4QT_EXPORT` macro) and, in the implementation, `helpers/initialisationhelper.h` (program start time used by the `RELATIVE` format).
- **Build requirement:** part of the `log4qt` target; `helpers/datetime.cpp` and `helpers/datetime.h` are listed in `src/log4qt/CMakeLists.txt`. The class is exported via `LOG4QT_EXPORT`. `Q_DECLARE_TYPEINFO(Log4Qt::DateTime, Q_MOVABLE_TYPE)` tells Qt containers the type is relocatable.

## 3. Class Hierarchy and Role

`DateTime` derives publicly from `QDateTime`.

- `QDateTime` → contributes the entire date/time value type: calendar arithmetic, time-zone handling, comparison, and the base `toString()`. `DateTime` adds formatting conventions and static helpers on top, and remains a drop-in `QDateTime` (it can be passed anywhere a `QDateTime` is expected and constructed from one implicitly).

It is **not** a `QObject` — no meta-object, signals, or slots. It is a value type with full copy and move support (declared explicitly to preserve `noexcept` and the `QDateTime` base assignment).

## 4. Q_PROPERTY Declarations

None.

## 5. Enumerations

None. The named format strings (`ABSOLUTE`, `DATE`, `ISO8601`, `NONE`, `RELATIVE`) are passed as `QString` rather than an enum.

## 6. Public Member Variables

None. The only data is inherited from `QDateTime`; the sole private member is `formatDateTime()`'s helper logic.

### Public type aliases

| Name | Definition | Description |
|------|------------|-------------|
| `Provider` | `std::function<QDateTime()>` | An injectable wall-clock source. Components needing testable time access accept a `Provider` so tests can substitute a lambda returning a controlled `QDateTime`. The canonical default is `[]{ return QDateTime::currentDateTime(); }`. |

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

### Constructors

#### DateTime()

Constructs a null (invalid) date/time, mirroring `QDateTime::QDateTime()`.

#### DateTime(const QDateTime &other)

Constructs a `DateTime` copy of any `QDateTime`. Implicit, so a `QDateTime` result can be wrapped transparently as a `DateTime`.

#### DateTime(const DateTime &other) noexcept

Copy constructor.

#### DateTime(DateTime &&other) noexcept

Move constructor (defaulted).

#### DateTime(QDate date, QTime time, QTimeZone timeZone = QTimeZone(QTimeZone::LocalTime))

Constructs from a `date` and `time` in the given `timeZone` (local time by default), forwarding to `QDateTime`.

#### ~DateTime()

Destructor (defaulted).

### Assignment

#### DateTime &operator=(const DateTime &other) noexcept

Copy assignment; delegates to `QDateTime::operator=`.

#### DateTime &operator=(DateTime &&other) noexcept

Move assignment (defaulted).

### Formatting

#### QString toString(const QString &format) const

Returns this datetime formatted according to `format`. Returns an empty string if the datetime is invalid. `format` may be a normal `QDateTime` format string **or** one of the named formats:

| Named format | Result |
|--------------|--------|
| `ABSOLUTE` | `HH:mm:ss.zzz` |
| `DATE` | `dd MMM YYYY HH:mm:ss.zzz` (documented form; the implementation uses `dd MM yyyy HH:mm:ss.zzz`) |
| `ISO8601` | `yyyy-MM-dd hh:mm:ss.zzz` |
| `NONE` | empty string |
| `RELATIVE` | milliseconds since program start |

Internally this forwards to `formatMsecs(toMSecsSinceEpoch(), format)`, so it shares the same thread-local cache.

#### static QString formatMsecs(qint64 msecs, const QString &format)

Formats an epoch-millisecond timestamp directly, without constructing a `DateTime` instance. This is the preferred fast path for callers that already hold a raw `qint64` (e.g. `LoggingEvent::timeStamp()`), avoiding a redundant `toMSecsSinceEpoch()` round-trip. For the named formats `ISO8601`, `ABSOLUTE`, and `DATE` the result is cached per calling thread (thread-local, keyed by the epoch millisecond): repeated calls within the same millisecond cost only a `qint64` comparison and a string copy. `NONE` and an empty format return an empty string; `RELATIVE` returns milliseconds since program start (`InitialisationHelper::startTime()`). Any other (custom) format delegates straight to `QDateTime::toString()` with no caching.

### Current-time helpers

#### static DateTime currentDateTime()

Returns the current datetime in the local time zone, obtained through the global `Provider` (so tests can override it). Equivalent to `QDateTime::currentDateTime()` by default.

#### static qint64 currentMSecsSinceEpoch()

Returns the current time as milliseconds since the Unix epoch, read through the global `Provider`. Uses a thread-local cache keyed by a monotonic `QElapsedTimer`: within the configured cache window (default 1 ms) repeated calls return the cached value and skip the wall-clock read. A cache window of `0` disables caching for maximum precision.

#### static void setCacheWindow(qint64 cacheWindowMs)

Sets the cache window for `currentMSecsSinceEpoch()`, in milliseconds. `0` disables caching; values of 1–100 give the best balance of performance and precision. The default is 1 ms. The setting is global and applies to all threads.

#### static qint64 cacheWindow()

Returns the current cache window in milliseconds.

#### static void setProvider(Provider provider)

Sets the global time source used by `currentDateTime()` and `currentMSecsSinceEpoch()`. Passing a null (default-constructed) `Provider` resets to the built-in `QDateTime::currentDateTime()` default. Thread-safe. Intended for tests — set once before any logging threads start, and reset during cleanup.

### Conversions

#### static DateTime fromMSecsSinceEpoch(qint64 msecs, QTimeZone timeZone)

Constructs a `DateTime` from an epoch-millisecond value interpreted in the given `timeZone`.

#### static DateTime fromMSecsSinceEpoch(qint64 msecs)

Constructs a `DateTime` from an epoch-millisecond value in the local time zone.

## 10. Protected Virtual Methods / Event Handlers

None. (`QDateTime` is not polymorphic; `DateTime` adds no virtual methods.)

## 11. Ownership and Lifecycle

`DateTime` is a **value type** with full copy/move semantics and no heap ownership or parent — instances are created on the stack or as members and destroyed automatically. Slicing to a `QDateTime` is harmless because all extra behaviour lives in non-virtual methods and statics; no per-instance state is added beyond the `QDateTime` base.

The global time provider and cache state are **process-global**, not per-instance: `setProvider()`, `setCacheWindow()`, and the thread-local caches persist for the lifetime of the loaded library. Tests that install a provider should reset it afterwards.

## 12. Thread Safety

- **Instances** follow `QDateTime` value semantics: separate instances are independent and safe to use concurrently; a single instance must not be mutated and read concurrently without external synchronisation.
- **Static methods are thread-safe by design:**
  - The global `Provider` is guarded by a `QReadWriteLock`. `setProvider()` takes the write lock; `currentDateTime()` and the wall-clock read inside `currentMSecsSinceEpoch()` take the read lock.
  - The `currentMSecsSinceEpoch()` cache (`s_cachedTimestamp`, `s_lastCounterValue`) is `thread_local`, so each thread has its own cache with no contention; the monotonic `QElapsedTimer` is started once at library load.
  - The cache window (`s_cacheWindowMs`) is a `std::atomic<qint64>` accessed with relaxed ordering.
  - The per-format timestamp caches inside `formatMsecs()` are `thread_local`, so concurrent formatting on different threads never shares state.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- **Layouts and `LoggingEvent`** call `toString()` / `formatMsecs()` to render event timestamps, and `currentMSecsSinceEpoch()` to stamp events cheaply.
- **`CronTriggeringPolicy`** passes `DateTime::currentDateTime()` to `CronExpression::nextFireTime()`.
- **Time-based rollover components** (`DailyRollingFileAppender`, `DateRolloverStrategy`) accept a `DateTime::Provider` so their clock can be injected in tests; the same global provider is what `setProvider()` controls.
- **`InitialisationHelper`** supplies the program start time used for the `RELATIVE` format.

## 15. External Communication

None. The only outside interaction is reading the system wall clock via `QDateTime::currentDateTime()` (through the provider), which is local to the process.

## 16. Usage Example

```cpp
#include "helpers/datetime.h"

using namespace Log4Qt;

// Format a raw event timestamp on the hot path (cached per thread/ms).
qint64 ts = event.timeStamp();
QString line = DateTime::formatMsecs(ts, u"ISO8601"_s);  // "2026-05-30 14:03:12.481"

// Format a DateTime instance with a named or custom format.
DateTime now = DateTime::currentDateTime();
QString a = now.toString(u"ABSOLUTE"_s);        // "14:03:12.481"
QString c = now.toString(u"yyyy/MM/dd"_s);      // custom format

// Cheap current-time reading with a 5 ms cache window.
DateTime::setCacheWindow(5);
qint64 ms = DateTime::currentMSecsSinceEpoch();

// In a test: install a fixed clock, then reset.
DateTime::setProvider([] {
    return QDateTime(QDate(2026, 1, 1), QTime(0, 0, 0));
});
// ... run code under test ...
DateTime::setProvider({});   // reset to system clock
```
