# SystemLogAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `SystemLogAppender` writes events to the host operating system's native logging facility: the **Windows Event Log** on Windows, and **syslog** on Unix-like systems (`*nix`).

A developer uses it to integrate an application's logs with the platform's standard log collection and monitoring tooling (Event Viewer, journald/rsyslog, log aggregation agents) rather than (or in addition to) writing private log files. The log4j-style level of each event is mapped onto the platform's native severity scheme.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class), `<string>` (for the cached `std::string` identifier).
- **Implementation includes:** `abstractlayout.h`, `level.h`, `loggingevent.h`, `<QCoreApplication>`. Platform branches add `qt_windows.h` + `<QLibrary>` on Windows, or the POSIX syslog headers (`syslog.h`, `pwd.h`, `unistd.h`, etc.) elsewhere.
- **Qt module:** Qt Core only.
- **Platform notes:** on Windows the Event Log functions (`RegisterEventSource`, `ReportEvent`, `DeregisterEventSource`) are resolved dynamically from `advapi32` via `QLibrary` (Unicode "W" variants), so the library does not hard-link them. On Unix the standard `openlog`/`syslog`/`closelog` C API is used.
- **Project-internal types:**
  - `Layout` / `AbstractLayout` — formats the event into the message text; obtained via `layout()`.
  - `Level` — supplies the log4j level whose integer value is mapped to a native severity.

## 3. Class Hierarchy and Role

`SystemLogAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It overrides `requiresLayout()` and `append()`. Its role is an OS-native log sink.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `serviceName` | `QString` | `serviceName` | `setServiceName` | — | The source/identity the events are logged under: the event source name on Windows, the `ident` passed to `openlog` on Unix. Defaults to `QCoreApplication::applicationName()`. On Unix the value is sanitised (lower-cased, restricted to alphanumerics) into the cached identifier. |

## 5. Enumerations

None.

## 6. Public Member Variables

The header declares two members in the `protected` section:

| Variable | Type | Description |
|----------|------|-------------|
| `mServiceName` | `QString` | The configured service/source name backing the `serviceName` property. |
| `mIdent` | `std::string` | Cached, sanitised identifier passed to `openlog()` on Unix. Unused on Windows. |

(These are protected rather than `m_`-prefixed private members, so they are documented here.)

## 7. Signals

None declared.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### explicit SystemLogAppender(QObject *parent = nullptr)

Constructs the appender and initialises `serviceName` to `QCoreApplication::applicationName()` (which also computes the Unix `mIdent`).

#### ~SystemLogAppender() override

Defaulted destructor; no special teardown (each `append()` opens and closes its own native handle/session).

#### bool requiresLayout() const override

Returns `true` — a layout is required to render the message text.

#### QString serviceName() const

Returns the configured service/source name.

#### void setServiceName(const QString &serviceName)

Sets the service/source name. On Unix it recomputes `mIdent` by passing the name through `encodeName()` (lower-case, alphanumeric-only, upper-case allowed) and storing the local-8-bit bytes.

## 10. Protected Virtual Methods

#### void append(const Log4Qt::LoggingEvent &event) override

Invoked from `doAppend()` under `mObjectGuard`. Formats the event via `layout()->format(event)`, then dispatches per platform:

- **Windows:** lazily resolves the `advapi32` Event Log functions (returns early if resolution fails). Maps the level to an event type — `WARN` → `EVENTLOG_WARNING_TYPE`, `ERROR`/`FATAL` → `EVENTLOG_ERROR_TYPE`, `OFF` → `EVENTLOG_SUCCESS`, everything else → `EVENTLOG_INFORMATION_TYPE`. Registers an event source for `serviceName()`, reports the (wide-string) message with `ReportEvent`, then deregisters the source.
- **Unix:** maps the level to a syslog priority — `WARN` → `LOG_WARNING`, `ERROR`/`FATAL` → `LOG_ERR`, everything else → `LOG_INFO`. Calls `openlog(mIdent, LOG_PID, LOG_DAEMON)`, emits each non-empty line of the message as a separate `syslog()` call, then `closelog()`.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr`.
- No persistent native handle is retained: on Windows each `append()` registers and deregisters the event source within the call; on Unix each `append()` opens and closes the syslog connection. There is therefore nothing to clean up at destruction.
- The dynamically resolved Windows function pointers are file-scope statics resolved once on first use.

## 12. Thread Safety

All public functions are thread-safe. `append()` runs serialised under `AppenderSkeleton`'s `mObjectGuard`, so only one thread at a time touches the native logging API. Because each call self-contains its register/report/deregister (Windows) or open/log/close (Unix) sequence, no cross-call native state is shared. The one-time lazy resolution of the Windows function pointers happens under the same serialised append path.

## 14. Inter-Class Interactions

- Uses a `Layout` (via `layout()`) to render the message.
- Reads `Level` from the event to choose a native severity.
- Reads `QCoreApplication::applicationName()` to seed the default service name.

## 15. External Communication

`SystemLogAppender` communicates with the host operating system's logging subsystem — out-of-process, OS-managed sinks.

- **Windows — Event Log:** via `RegisterEventSource` / `ReportEvent` / `DeregisterEventSource` (resolved from `advapi32.dll`, Unicode variants). **Outbound** only. The message is sent as a single wide-string insertion string under the `serviceName` event source, with an event type derived from the log level. Events surface in the Windows Event Viewer.
- **Unix — syslog:** via `openlog` / `syslog` / `closelog`. **Outbound** only. The identifier is the sanitised `serviceName`; facility is `LOG_DAEMON` and `LOG_PID` is set. Each non-empty line of the formatted message is sent as a separate syslog record at the mapped priority. Delivery to files/journals is handled by the system syslog daemon.
- **Error handling:** on Windows, if `advapi32` functions cannot be resolved or the event source cannot be registered, the call silently returns without logging. On Unix the C syslog calls do not report failures back to the caller. There is no reconnection logic because each call is self-contained.
- **Threading implications:** all native calls happen synchronously on the logging thread under `mObjectGuard`; there are no callbacks or background threads.

## 16. Usage Example

```cpp
#include "log4qt/systemlogappender.h"
#include "log4qt/patternlayout.h"
#include "log4qt/logger.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(QStringLiteral("%p %c - %m")));
layout->activateOptions();

auto *syslog = new SystemLogAppender;
syslog->setName(QStringLiteral("syslog"));
syslog->setLayout(layout);
syslog->setServiceName(QStringLiteral("MyService"));   // event source / syslog ident
syslog->activateOptions();

Logger::rootLogger()->addAppender(AppenderSharedPtr(syslog));
Logger::rootLogger()->warn(QStringLiteral("written to the OS log facility"));
// Windows: visible in Event Viewer. Unix: routed by the syslog daemon.
```
