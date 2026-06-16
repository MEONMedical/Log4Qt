# AppenderAttachable

## Class Overview

`AppenderAttachable` is a reusable mix-in base class that gives an object a thread-safe collection of attached appenders. It implements the bookkeeping for adding, querying, and removing `Appender` instances and is intended to be inherited by any object that needs to fan log events out to one or more appenders. In Log4Qt it backs `Logger` (which forwards logging events to its attached appenders) and `AsyncAppender`.

The class mirrors the `org.apache.log4j.spi.AppenderAttachable` interface from Apache log4j, but is provided here as a concrete implementation rather than a pure interface.

## Project Structure and Dependencies

- Header: `src/log4qt/helpers/appenderattachable.h`
- Source: `src/log4qt/helpers/appenderattachable.cpp`
- Part of the `log4qt` library target (see `src/log4qt/CMakeLists.txt`, `log4qt_HEADERS_helpers`).

Dependencies:

- `log4qt/appender.h` — provides `Appender` and the `AppenderSharedPtr` alias (`Log4QtSharedPtr<Appender>`).
- `QList` — storage for the attached appenders.
- `QReadWriteLock` — guards concurrent access.
- `varia/listappender.h` (cpp only) — referenced in documentation of `removeAllAppenders()` regarding configurator-list appenders.
- `<algorithm>` (cpp only) — `std::find_if` used by `appender(const QString &)`.

## Class Hierarchy and Role

`AppenderAttachable` has no base class and is not a `QObject`. It is designed to be used as an additional (non-polymorphic-root) base class for objects that own a set of appenders. The destructor is `virtual`, and all member functions are `virtual`, so derived classes may override the attachment behaviour. There are no derived classes declared in this file; `Logger` and `AsyncAppender` are the principal users elsewhere in the library.

## Public Methods

#### void addAppender(const AppenderSharedPtr &appender)

Adds `appender` to the collection. A null pointer is ignored. If the appender is already attached (pointer-equality), it is not added a second time. Acquires the write lock.

#### QList&lt;AppenderSharedPtr&gt; appenders() const

Returns a snapshot copy of all currently attached appenders. Acquires the read lock.

#### AppenderSharedPtr appender(const QString &name) const

Returns the first attached appender whose `name()` equals `name`, or a null `AppenderSharedPtr` if none matches. Acquires the read lock.

#### bool isAttached(const AppenderSharedPtr &appender) const

Returns `true` if `appender` is present in the collection (pointer-equality), `false` otherwise. Acquires the read lock.

#### void removeAllAppenders()

Removes all attached appenders. Acquires the write lock.

Note: the header documents an intended behaviour whereby `ListAppender` instances with the configurator-list property set are preserved so configurators can collect events during configuration. The current implementation clears the entire list unconditionally; callers relying on the documented exception should verify behaviour.

#### void removeAppender(const AppenderSharedPtr &appender)

Removes all occurrences of `appender` from the collection. A null pointer is ignored. Acquires the write lock.

#### void removeAppender(const QString &name)

Looks up the appender by `name` and, if found, removes it. Internally calls `appender(name)` (read lock) followed by the pointer overload of `removeAppender` (write lock).

## Protected Member Variables

| Member | Type | Description |
|--------|------|-------------|
| `mAppenders` | `QList<AppenderSharedPtr>` | The attached appenders, held by ref-counted shared pointers. |
| `mAppenderGuard` | `mutable QReadWriteLock` | Recursive read/write lock guarding `mAppenders`. |

`mAppenderGuard` is exposed as `protected` so that derived classes (for example a `Logger`) can extend their own operations under the same lock that protects the appender collection.

## Ownership and Lifecycle

Appenders are held via `AppenderSharedPtr` (a ref-counted `Log4QtSharedPtr<Appender>`). `AppenderAttachable` shares ownership rather than taking exclusive ownership: an appender stays alive as long as any shared pointer (this collection or another holder) references it. Removing an appender or destroying the `AppenderAttachable` releases this object's references; the appender is destroyed only when the last shared reference drops. The destructor is defaulted and simply releases the held references when `mAppenders` is destroyed.

## Thread Safety

All public functions are thread-safe. The class uses a single `QReadWriteLock` constructed in **recursive** mode:

- Read operations (`appenders()`, `appender()`, `isAttached()`) take a `QReadLocker`.
- Mutating operations (`addAppender()`, `removeAllAppenders()`, `removeAppender()`) take a `QWriteLocker`.

Recursive mode allows `removeAppender(const QString &)` to call `appender()` (read lock) and then `removeAppender(const AppenderSharedPtr &)` (write lock) from the same thread without self-deadlock. The lock is `mutable`, so `const` query methods can still acquire it.

## Inter-Class Interactions

- `Logger` inherits this class to manage the appenders that receive its logging events.
- `AsyncAppender` inherits this class to manage the downstream appenders it dispatches to.
- Configurators (e.g. `PropertyConfigurator`) call `addAppender()`/`removeAllAppenders()` while applying configuration.

## Usage Example

```cpp
// A type that can have appenders attached to it.
class EventSink : public Log4Qt::AppenderAttachable
{
public:
    void emitEvent(const Log4Qt::LoggingEvent &event)
    {
        const auto targets = appenders(); // thread-safe snapshot
        for (const auto &appender : targets)
            appender->doAppend(event);
    }
};

EventSink sink;
sink.addAppender(Log4Qt::AppenderSharedPtr(new Log4Qt::ConsoleAppender));
if (auto a = sink.appender(QStringLiteral("console")))
    sink.removeAppender(a);
```
