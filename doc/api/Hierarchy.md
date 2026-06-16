# Hierarchy

## 1. Class Overview

`Hierarchy` is the concrete logger repository shipped with Log4Qt and the one `LogManager` creates by default. It stores `Logger` objects in a name-keyed hash, organises them into a parent/child tree based on their dotted (`::`-separated) names, holds the single root logger, and enforces a repository-wide threshold.

When code requests `logger("a::b::c")`, `Hierarchy` materialises the full ancestor chain — creating `a`, `a::b` and `a::b::c` if they do not yet exist and wiring each child to its parent — so that level inheritance and appender additivity work up the tree. A developer interacts with `Hierarchy` almost always through `LogManager`; direct use is reserved for embedding a private repository.

## 2. Project Structure and Dependencies

`Hierarchy` is instantiated by `LogManager` (`new Hierarchy()` as the default repository) and could be instantiated directly by code that wants an isolated repository. It uses `Logger` (which it creates and owns) and `OptionConverter` (to normalise Java-style dotted names to C++ `::` separators).

Internal types: `LoggerRepository` (the abstract base it implements), `Logger`, `Level`, `OptionConverter` — all from the Log4Qt library.

- **Qt module dependency:** Qt Core (`QHash`, `QReadWriteLock`, `QList`, `QString`).
- **Build requirement:** part of the `log4qt` target linking `Qt6::Core`; exported via `LOG4QT_EXPORT`.

## 3. Class Hierarchy and Role

`Hierarchy` derives publicly from `LoggerRepository` and overrides every pure-virtual method of that interface. `LoggerRepository` contributes the repository contract (logger lookup, threshold, reset/shutdown) and a virtual destructor for safe polymorphic deletion. `Hierarchy` is not a `QObject` and has no meta-object features. It is a `friend` of `Logger`, which lets its destructor reach `Logger`'s protected destructor to delete the loggers it owns.

## 4. Q_PROPERTY Declarations

None.

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### Hierarchy()

Constructs an empty hierarchy. Initialises the guard as a recursive `QReadWriteLock`, sets the threshold to `Level::NULL_INT` (effectively no threshold), and creates the root logger by calling `logger(QString())`. The root logger is created with level `DEBUG_INT`, name `"root"` and no parent.

#### ~Hierarchy() override

Deletes every owned `Logger` (reaching `Logger`'s protected destructor as a friend), clears the hash and nulls the root pointer. Logs a warning ("Unexpected destruction of Hierarchy") because, like `LogManager`, the default instance is intended to outlive normal teardown. Acquires the write lock during cleanup.

#### bool exists(const QString &name) const override

Returns true if a logger with the exact name is already present in the hash, without creating it. Takes a read lock.

#### Logger *logger(const QString &name) override

Returns the logger for the given name, creating it and all missing ancestors on demand. Acquires the write lock directly (rather than a read-then-upgrade pattern) because the method is called re-entrantly while the write lock is already held — Qt's recursive `QReadWriteLock` only recognises recursion within the same lock mode, so taking a read lock while holding the write lock would deadlock. Logger lookups are cached at every call site, so this is not a hot path.

#### QList<Logger *> loggers() const override

Returns every logger held by the repository. Takes a read lock.

#### Logger *rootLogger() const override

Returns the root logger (inline accessor). The root is the ancestor of all named loggers.

#### Level threshold() const override

Returns the repository-wide threshold (inline accessor reading an atomic `Level`).

#### void setThreshold(Level level) override

Sets the repository-wide threshold (inline). Levels below it are disabled for all loggers.

#### void setThreshold(const QString &threshold) override

Parses the level name via `Level::fromString()` and forwards to the `Level` overload.

#### bool isDisabled(Level level) const override

Returns whether the given level is strictly below the threshold (inline), i.e. suppressed.

#### void resetConfiguration() override

Resets every logger under the write lock. Regular loggers are reset first (appenders removed, additivity restored, level set to `NULL_INT`); the special loggers are reset last so shutdown can still be logged — the `Qt` and internal (empty-name) loggers to `NULL_INT`, and the root logger to `DEBUG_INT`.

#### void shutdown() override

Logs a debug message and delegates to `resetConfiguration()`.

## 10. Protected Virtual Methods

`Hierarchy` declares no protected members. All overridden virtuals are public (listed in Section 9). The private helpers `createLogger()` (recursive create-or-fetch that links each logger to its parent) and `resetLogger()` (clears appenders, restores additivity, sets level) are implementation details.

## 11. Ownership and Lifecycle

`Hierarchy` owns every `Logger` it creates; they are allocated with `new` and stored in the hash, and the destructor deletes them all. Callers receive raw `Logger *` pointers but never own or delete them — their lifetime is bound to the repository. The repository itself is normally owned by `LogManager` and intentionally outlives the program's teardown path; explicit destruction is treated as unexpected and logged.

## 12. Thread Safety

Fully thread-safe, as stated in the header. A single `mutable QReadWriteLock` constructed in recursive mode guards all logger storage: read operations (`exists`, `loggers`, `rootLogger` reads) take read locks, while mutating and create-on-demand operations (`logger`, `resetConfiguration`) take write locks. The recursive mode is deliberate to support re-entrant lookups that occur while the write lock is held (e.g. the warning logged during `resetConfiguration` resolves a logger). The threshold is stored as `std::atomic<Level>`.

## 13. QML Exposure

None.

## 14. Inter-Class Interactions

- Implements the `LoggerRepository` contract that `LogManager` depends on; `LogManager` creates and owns one `Hierarchy`.
- Creates, parents and owns `Logger` instances; relies on friendship with `Logger` for construction and destruction.
- Uses `OptionConverter::classNameJavaToCpp()` to normalise logger names before lookup.

## 15. External Communication

None.

## 16. Usage Example

```cpp
#include "log4qt/hierarchy.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

Hierarchy repository;

// Lookups create the full ancestor chain on demand.
Logger *child = repository.logger(QStringLiteral("net::http::client"));
// repository now also contains "net" and "net::http", each parented correctly.

repository.setThreshold(QStringLiteral("INFO"));
Logger *root = repository.rootLogger();

// All loggers are owned by the repository; do not delete them yourself.
repository.shutdown();
```
