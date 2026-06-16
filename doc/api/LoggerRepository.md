# LoggerRepository

## 1. Class Overview

Log4Qt organises logging around named `Logger` objects in a dotted-name hierarchy. A *logger repository* is the container that creates, stores, parents and configures those loggers and holds the root logger plus a global threshold.

`LoggerRepository` is the abstract base class (pure-virtual interface) for any such repository. It defines the contract that `LogManager` uses to manage loggers without depending on a concrete storage strategy. The library ships one concrete implementation, `Hierarchy`, which stores loggers in a hash keyed by name and links each to its parent. A developer implements this interface only to provide an alternative repository; ordinary code uses the repository indirectly through `LogManager`.

## 2. Project Structure and Dependencies

`LoggerRepository` is implemented by `Hierarchy` (`hierarchy.h`). It is consumed by `LogManager`, which owns a repository instance and delegates logger management to it, and by the configurator classes that operate against a repository pointer.

Internal types: `Logger` (forward-declared; the managed objects) and `Level` (the level/threshold value type), both from the Log4Qt library.

- **Qt module dependency:** Qt Core (`QString`, `QList`).
- **Build requirement:** part of the `log4qt` target linking `Qt6::Core`; exported via `LOG4QT_EXPORT`.

## 3. Class Hierarchy and Role

`LoggerRepository` has no base class. It is a non-`QObject` abstract interface: copy construction and copy assignment are explicitly deleted, the destructor is `virtual` (signalling subclassing intent and ensuring correct polymorphic deletion), and every operational method is pure virtual. Concrete subclasses (such as `Hierarchy`) must implement all of them.

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

#### LoggerRepository()

Default constructor. Defaulted in the implementation; available for subclasses to chain.

#### virtual ~LoggerRepository()

Virtual destructor, defaulted. Guarantees that deleting a derived repository through a base pointer runs the derived destructor.

#### virtual bool exists(const QString &name) const = 0

Returns whether a logger with the given name already exists, without creating it. Implementations must not insert a new logger.

#### virtual Logger *logger(const QString &name) = 0

Returns the logger for the given dotted name, creating it and any missing ancestors if necessary. The primary lookup operation.

#### virtual QList<Logger *> loggers() const = 0

Returns all loggers currently held by the repository.

#### virtual Logger *rootLogger() const = 0

Returns the root logger — the implicit ancestor of every named logger.

#### virtual Level threshold() const = 0

Returns the repository-wide threshold. Events below it are suppressed regardless of an individual logger's level.

#### virtual void setThreshold(Level level) = 0

Sets the repository-wide threshold to a `Level` value.

#### virtual void setThreshold(const QString &threshold) = 0

Sets the threshold from a level name string (e.g. `"INFO"`). Overloads `setThreshold` for convenience when configuring from text.

#### virtual bool isDisabled(Level level) const = 0

Returns whether the given level is below the repository threshold and therefore disabled. Used as a fast pre-check before assembling a logging event.

#### virtual void resetConfiguration() = 0

Resets every logger to its default state: removes all appenders, restores additivity, and resets levels. Implementations are expected to leave the special loggers (root, internal, Qt) for last.

#### virtual void shutdown() = 0

Shuts the repository down, typically by resetting configuration so that buffered/asynchronous appenders flush.

## 10. Protected Virtual Methods

None beyond the pure-virtual public contract above.

## 11. Ownership and Lifecycle

A repository owns all the `Logger` objects it creates and is responsible for deleting them (the concrete `Hierarchy` does so in its destructor). Ownership of the repository itself rests with whoever creates it; in normal use `LogManager` creates and owns the single default repository for the process. The `virtual` destructor ensures derived-class cleanup runs through a `LoggerRepository *`.

## 12. Thread Safety

The interface itself imposes no synchronisation, but it is designed to be implemented thread-safely. The shipped `Hierarchy` implementation is fully thread-safe (it uses a recursive `QReadWriteLock`), and `LogManager` relies on that guarantee.

## 13. QML Exposure

None.

## 14. Inter-Class Interactions

- `LogManager` holds a `LoggerRepository *` and forwards logger management calls to it.
- Configurator classes (`PropertyConfigurator`, `JsonConfigurator`, `XmlConfigurator`, `BasicConfigurator`) operate on a repository to attach appenders and set levels.
- Produces and parents `Logger` instances and exposes them to callers.

## 15. External Communication

None.

## 16. Usage Example

This is an abstract interface; the example shows implementing the contract.

```cpp
#include "log4qt/loggerrepository.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

class MyRepository : public LoggerRepository
{
public:
    bool exists(const QString &name) const override { return mLoggers.contains(name); }
    Logger *logger(const QString &name) override { /* create-or-return */ return nullptr; }
    QList<Logger *> loggers() const override { return mLoggers.values(); }
    Logger *rootLogger() const override { return mRoot; }
    Level threshold() const override { return mThreshold; }
    void setThreshold(Level level) override { mThreshold = level; }
    void setThreshold(const QString &t) override { setThreshold(Level::fromString(t)); }
    bool isDisabled(Level level) const override { return level < mThreshold; }
    void resetConfiguration() override { /* clear appenders, reset levels */ }
    void shutdown() override { resetConfiguration(); }

private:
    QHash<QString, Logger *> mLoggers;
    Logger *mRoot = nullptr;
    Level mThreshold = Level(Level::NULL_INT);
};
```
