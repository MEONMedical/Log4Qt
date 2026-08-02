# ConfiguratorHelper

## Class Overview

`ConfiguratorHelper` is a process-wide singleton that supports the configurator classes by:

- Tracking the currently active configuration file and watching it for on-disk changes.
- Holding a user-supplied configure callback that is invoked when the watched file changes, enabling live ("configure and watch") reloading.
- Storing the error information produced by the most recent configuration operation.
- Emitting a signal after each processed file change so observers can react to a (re)configuration.

It is used internally by configurators such as `PropertyConfigurator` (via `configureAndWatch()`), which install the callback and the file to watch.

## Project Structure and Dependencies

- Header: `src/log4qt/helpers/configuratorhelper.h`
- Source: `src/log4qt/helpers/configuratorhelper.cpp`
- Part of the `log4qt` library target (see `src/log4qt/CMakeLists.txt`, `log4qt_HEADERS_helpers`).

Dependencies:

- `QObject` — base class (the singleton is a `QObject` so it can own a watcher and emit a signal).
- `QFileSystemWatcher` (forward declared; included in the cpp) — watches the configuration file and its directory.
- `QFileInfo` — stores the configuration file path.
- `QMutex` — guards the singleton state.
- `QTimer` (cpp only) — debounces re-adding the watched path after directory changes.
- `log4qt/loggingevent.h` — `LoggingEvent`, the element type of the stored error list.
- `helpers/initialisationhelper.h` (cpp only) — provides `LOG4QT_IMPLEMENT_INSTANCE`.

## Class Hierarchy and Role

`ConfiguratorHelper : public QObject`. It is a singleton (private constructor/destructor, `Q_DISABLE_COPY_MOVE`, instance created by `LOG4QT_IMPLEMENT_INSTANCE`). All of its public API is `static` and forwards to the single instance, so callers never construct it directly.

## Type Aliases

| Alias | Underlying type | Description |
|-------|-----------------|-------------|
| `ConfigureFunc` | `bool (*)(const QString &fileName)` | Prototype for the configure callback invoked on a file change. Returns success/failure; receives the changed file name. Typically `PropertyConfigurator::configure(const QString &)`. |

## Signals

#### void configurationFileChanged(const QString &fileName, bool error)

Emitted after a change to `fileName` has been processed (i.e. after the configure callback has run). `error` is `true` if the configuration produced errors (the stored error list from `configureError()` is non-empty), `false` otherwise. Connect to this signal to be notified of live reloads and their outcome.

This signal is emitted from the slot handling the file-system watcher notification, which always runs in the thread the singleton's `QObject` lives in (normally the main thread) — the watcher is explicitly moved into that thread, so the delivery thread does not depend on which thread called `setConfigurationFile()` / `configureAndWatch()`.

## Public Methods

All public methods are `static`.

#### static QList&lt;LoggingEvent&gt; configureError()

Returns the error information captured for the most recent configuration operation, whether triggered by an explicit `configure()` call or by a watched-file change. Returns a snapshot copy under the mutex.

#### static QString configurationFile()

Returns the absolute path of the current configuration file, or an empty string if none is set. Snapshot under the mutex.

#### static ConfiguratorHelper *instance()

Returns the singleton instance, creating it on first use.

#### static void setConfigureError(const QList&lt;LoggingEvent&gt; &configureError)

Stores the error information for the most recent configuration operation. Called by configurators (and from within the configure callback) to record results; `configureError()` reads it back. Updates the state under the mutex.

#### static void setConfigurationFile(const QString &fileName = QString(), ConfigureFunc pConfigureFunc = nullptr)

Sets the configuration file to watch and the callback to invoke when it changes. Behaviour:

- Passing an empty `fileName` (the default) stops watching and clears the callback.
- If `fileName` does not exist on disk, watching is not started.
- Otherwise a `QFileSystemWatcher` is created watching both the file and its containing directory; on a file change the callback runs and `configurationFileChanged()` is emitted.
- The new watcher is then moved into the helper's thread with `moveToThread()`. This is required for correctness rather than convenience: change notifications need a running event loop in the watcher's own thread (the thread that called `setConfigurationFile()` may have none, or may exit), and `tryToReAddConfigurationFile()` runs on the helper's thread and calls watcher methods directly.
- A previously installed watcher is disconnected and retired with `deleteLater()` instead of being deleted inline — it lives in the helper's thread, so a plain delete from the calling thread would destroy a `QObject` cross-thread, possibly while its own `fileChanged` emission is still on the call stack.

If the watcher fails to add the file path, a warning is logged and no watch is established. Delegates to the private `doSetConfigurationFile()`.

## Private Slots

These are `private Q_SLOTS` (not part of the public API) but document the live-reload mechanism:

- `doConfigurationFileChanged(const QString &fileName)` — handles the watcher's `fileChanged` signal: reads the callback and config path under the lock, then (if the callback is set and the file still exists) runs the callback **outside** the lock, re-checks the error state, and emits `configurationFileChanged()`.
- `doConfigurationFileDirectoryChanged(const QString &path)` — handles `directoryChanged`; schedules `tryToReAddConfigurationFile()` after a 100 ms single-shot timer (debounce).
- `tryToReAddConfigurationFile()` — re-adds the file path to the watcher if it is no longer being watched (handles editors that replace the file via rename/delete-and-recreate).

## Ownership and Lifecycle

`ConfiguratorHelper` is a process singleton with a non-trivial private destructor; the single instance is created on first `instance()` call and intentionally lives for the duration of the process. It owns its `QFileSystemWatcher` via `std::unique_ptr` (`mConfigurationFileWatch`), which is created in `setConfigurationFile()` and lives in the helper's thread. When watching is stopped or reconfigured the pointer is `release()`d and the watcher is handed to Qt for deferred destruction via `deleteLater()` (see above), so ownership transfers to the event loop rather than the `unique_ptr` deleting it in place. The stored `ConfigureFunc` is a plain function pointer (no ownership). The `LoggingEvent` error list is owned by value.

## Thread Safety

All public functions are thread-safe. State is guarded by a non-recursive `QMutex` (`mObjectGuard`). Accessors (`configureError()`, `configurationFile()`, `setConfigureError()`) hold the mutex only for the duration of the snapshot/assignment.

The watcher's thread affinity is part of the thread-safety contract: `setConfigurationFile()` may be called from any thread, but the watcher is always moved into the helper's thread, so `fileChanged` / `directoryChanged` — and therefore the configure callback and the `configurationFileChanged()` emission — are consistently delivered there.

A deliberate locking subtlety: in `doConfigurationFileChanged()` the user callback is invoked **outside** the mutex. The callback commonly calls back into `setConfigureError()` (which locks the same non-recursive mutex); running it under the lock would self-deadlock. State needed by the callback is copied out under the lock first, the callback runs unlocked, and the error flag is re-read under the lock before emitting.

## External Communication

`ConfiguratorHelper` is the library's inbound bridge to the filesystem. Via `QFileSystemWatcher` it monitors:

- The configuration file itself (`fileChanged`) — triggers a reconfigure.
- The file's directory (`directoryChanged`) — used to recover the watch when the file is replaced (many text editors save by writing a temp file and renaming over the original, which removes the original from the watcher).

No network or other I/O is performed; the only external interaction is reading the watched file's existence/change notifications. The actual file parsing is done by the supplied callback (e.g. a configurator), not by this class.

## Inter-Class Interactions

- `PropertyConfigurator` / other configurators call `setConfigurationFile()` to enable live reload and `setConfigureError()`/`configureError()` to surface results.
- Application code connects to `configurationFileChanged()` to learn when the configuration was reloaded and whether it succeeded.
- Uses `LOG4QT_IMPLEMENT_INSTANCE` from `InitialisationHelper` to define the singleton accessor.

## Usage Example

```cpp
// Enable configure-and-watch with a custom configure callback.
Log4Qt::ConfiguratorHelper::setConfigurationFile(
    QStringLiteral("/etc/myapp/log4qt.properties"),
    [](const QString &fileName) -> bool {
        // parse and apply configuration here; return success
        return Log4Qt::PropertyConfigurator::configure(fileName);
    });

// React to live reloads.
QObject::connect(Log4Qt::ConfiguratorHelper::instance(),
                 &Log4Qt::ConfiguratorHelper::configurationFileChanged,
                 [](const QString &file, bool error) {
                     if (error)
                         qWarning() << "Reload of" << file << "had errors:"
                                    << Log4Qt::ConfiguratorHelper::configureError().size();
                 });

// Stop watching.
Log4Qt::ConfiguratorHelper::setConfigurationFile();
```
