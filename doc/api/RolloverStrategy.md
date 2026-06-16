# RolloverStrategy

## 1. Class Overview

`RolloverStrategy` is the abstract base class for all rollover strategies. A rollover strategy decides *how* a `RollingFileAppender` rotates, renames, prunes, and (optionally) compresses log files when a rollover is triggered. It is deliberately separated from the orthogonal concern of *when* a rollover happens, which is the responsibility of a `TriggeringPolicy`.

The design is inspired by log4j2's `RolloverStrategy` interface. Concrete subclasses such as `DefaultRolloverStrategy` (fixed-window numbered backups) and `DateRolloverStrategy` (date-stamped names) implement the rotation algorithm by overriding the pure-virtual `rollover()` method.

A strategy is responsible only for manipulating *backup* files on disk. It must **not** close or open the active file — the appender always closes the active file before calling `rollover()` and reopens the file the strategy returns afterwards.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/rolloverstrategy.h`
- Source: `src/log4qt/spi/rolloverstrategy.cpp`

Direct dependencies:

- `QObject` — base class.
- `Log4QtSharedPtr<RolloverStrategy>` (`log4qtsharedptr.h`) — the managed shared-pointer type used to hold strategies; aliased as `RolloverStrategySharedPtr`.
- `QFile` — used by the protected file helpers.
- `LogError` / `Logger` (`helpers/logerror.h`, `logger.h`) — used to report rename/remove failures during rollover.

Consumed by `RollingFileAppender`, which owns a strategy and invokes it on rollover.

## 3. Class Hierarchy and Role

`QObject` → **`RolloverStrategy`** (abstract)

`RolloverStrategy` is an abstract `QObject` base. It is non-copyable and non-movable (`Q_DISABLE_COPY_MOVE`). It defines the contract a `RollingFileAppender` relies on and provides two protected static file helpers shared by all subclasses. Known concrete subclasses:

- `DefaultRolloverStrategy` — numbered fixed-window rotation (`.1`, `.2`, …).
- `DateRolloverStrategy` — date-stamped backups.

## 4. Q_PROPERTY

None declared on the base class. Concrete subclasses add their own configuration properties.

## 5. Enumerations

None.

## 6. Public Member Variables

None. The base class declares no data members.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

#### explicit RolloverStrategy(QObject *parent = nullptr)
Constructs the strategy with an optional QObject parent.

#### ~RolloverStrategy() override
Destroys the strategy. Defaulted; declared `override` because the base `QObject` destructor is virtual.

#### virtual void activateOptions()
Applies configuration after all properties have been set. The default implementation is a no-op. Subclasses override it to precompute derived state (for example `DateRolloverStrategy` captures the current date suffix here). Called by `RollingFileAppender::activateOptions()`.

#### virtual QString initialFileName(const QString &fileName) const
Returns the initial active file path the appender should open on startup. The default implementation returns `fileName` unchanged. Strategies may override this to provide a dated or indexed initial filename without requiring an actual rollover — for example a date-embedded active file from the very first startup.

#### virtual QString rollover(const QString &fileName) = 0
Pure virtual. Performs the rollover for the given base log file. The strategy rotates, renames, and/or deletes old backup files and returns the file path the appender should open next (usually `fileName` itself, but a date-naming strategy may return a freshly dated name). See section 10 for the contract.

## 10. Protected Virtual Methods

`rollover()` is the single overridable algorithm hook (declared pure virtual in section 9). Its contract:

- Input is always the **base** log file path. The appender records the configured base name and passes it unchanged so a strategy never sees an already-transformed filename.
- The strategy performs all backup-file disk operations.
- It must not open or close the active file.
- It returns the path the appender opens next.

Two protected static helpers are provided for subclasses to perform disk operations with consistent error reporting:

#### static bool removeFile(const QString &fileName)
Removes `fileName`. Returns `true` if the file did not exist or was removed successfully. On failure, logs a `LogError` with constant `AppenderRemoveFileError` (including the underlying `QFile` error as a causing error) and returns `false`.

#### static bool renameFile(const QString &source, const QString &target)
Renames `source` to `target`. Returns `true` on success. On failure, logs a `LogError` with constant `AppenderRenamingFileError` (including the underlying `QFile` error) and returns `false`.

## 11. Ownership and Lifecycle

Strategies are held by `RollingFileAppender` through a `RolloverStrategySharedPtr` (`Log4QtSharedPtr<RolloverStrategy>`), so lifetime is managed by reference counting rather than the QObject parent tree. The class is non-copyable and non-movable. A `RollingFileAppender` installs a `DefaultRolloverStrategy` automatically if none is set when its options are activated.

## 12. Thread Safety

The base class itself holds no mutable state and is therefore inherently safe. The static helpers `removeFile()`/`renameFile()` are reentrant (each operates on a local `QFile`). Thread-safety guarantees for `rollover()` are defined by each concrete subclass and by the fact that the appender serializes rollover through its own locking.

## 13. QML Exposure

Not registered for QML. No `QML_ELEMENT` / `qmlRegisterType` exists for this class.

## 14. Inter-Class Interactions

- `RollingFileAppender::activateOptions()` calls `activateOptions()` and `initialFileName()` on the strategy, then opens the (possibly renamed) file.
- `RollingFileAppender::rollOver()` closes the active file, calls `rollover(baseName)`, and reopens whatever path is returned.
- `TriggeringPolicy` determines *when* `rollOver()` runs; `RolloverStrategy` determines *how* it is carried out — the two collaborate but never reference each other.

## 15. External Communication

Through subclass implementations and the protected helpers, the strategy renames and deletes files on the local filesystem during a rollover. The base class performs no disk I/O on its own.
