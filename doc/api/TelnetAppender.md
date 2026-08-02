# TelnetAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `TelnetAppender` runs a small TCP server and streams formatted log lines to every connected client, so an operator can watch a live log feed simply by `telnet`-ing to the configured host and port.

A developer uses it for remote, ad-hoc, read-only log monitoring of a running process — no file access needed, just a TCP connection. Clients connect inbound; the appender pushes log text outbound to all of them. An optional welcome message greets each new client.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class), `<QString>`, `<QHostAddress>`; forward declarations of `QTcpServer` and `QTcpSocket`.
- **Implementation includes:** `abstractlayout.h`, `loggingevent.h`, `<QTcpServer>`, `<QTcpSocket>`, `<QHostAddress>`.
- **Qt module:** Qt Network plus Qt Core. `Qt::Network` is linked **`PUBLIC`** (only when `BUILD_WITH_TELNET_LOGGING` is enabled), because the installed `telnetappender.h` includes QtNetwork headers.
- **Project-internal types:**
  - `Layout` / `AbstractLayout` — formats each `LoggingEvent` into the text streamed to clients; obtained via the lock-free `layoutSnapshot()` from `AppenderSkeleton`.
  - `LoggingEvent` — the event to format.

## 3. Class Hierarchy and Role

`TelnetAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It is constructed inactive (`AppenderSkeleton(false, ...)`). It overrides `requiresLayout()`, `activateOptions()`, `close()`, `append()`, and `checkEntryConditions()`, and defines two private slots for connection management.

Its role is a TCP-server log fan-out sink.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `port` | `int` | `port` | `setPort` | — | TCP port the server listens on. Default 23 (the telnet port). `setPort()` rejects values outside 1..65535 with a logged error. |
| `immediateFlush` | `bool` | `immediateFlush` | `setImmediateFlush` | — | When `true`, each socket is `flush()`ed after every write. Default `false`. Backed by a `std::atomic<bool>`. |
| `address` | `QHostAddress` | `address` | `setAddress` | — | Local interface address the server binds to. Default `QHostAddress::Any` (all interfaces). |

## 5. Enumerations

None.

## 6. Public Member Variables

None.

## 7. Signals

None declared.

## 8. Public Slots and Q_INVOKABLE Methods

No public slots. The class defines two **private** slots used internally:

- `onNewConnection()` — accepts a pending connection, tracks the socket, wires its `disconnected` signal, and sends the welcome message.
- `onClientDisconnected()` — removes the disconnected socket from the tracked list and schedules it for deletion.

## 9. Public Methods

#### TelnetAppender(QObject *parent = nullptr)

Constructs with defaults: bind `QHostAddress::Any`, port 23, no server yet, `immediateFlush` false.

#### TelnetAppender(const LayoutSharedPtr &layout, QObject *parent = nullptr)

As above, with a layout supplied.

#### TelnetAppender(const LayoutSharedPtr &layout, const QHostAddress &address, int port, QObject *parent = nullptr)

Constructs with layout, bind address, and port.

#### TelnetAppender(const LayoutSharedPtr &layout, int port, QObject *parent = nullptr)

Constructs with layout and port, binding `QHostAddress::Any`.

#### ~TelnetAppender() override

Calls `close()`, which stops the server and tears down all client sockets.

#### bool requiresLayout() const override

Returns `true` — a layout is required to format the streamed text.

#### void activateOptions() override

Under `mObjectGuard`, tears down any existing server (`closeServer()`) and starts a fresh one (`openServer()`), then chains to `AppenderSkeleton::activateOptions()`.

#### void close() override

Under `mObjectGuard`, if not already closed, chains to `AppenderSkeleton::close()` and then `closeServer()` to release the listening socket and all clients.

#### void setPort(int port) / int port() const

Set/get the listening port. `setPort()` validates the 1..65535 range. Takes effect at the next `activateOptions()`.

#### void setImmediateFlush(bool immediateFlush) / bool immediateFlush() const

Set/get per-write flushing.

#### void setAddress(const QHostAddress &address) / QHostAddress address() const

Set/get the bind address. Takes effect at the next `activateOptions()`.

#### void setWelcomeMessage(const QString &welcomeMessage)

Sets a message sent once to each newly connected client. If empty, no welcome is sent.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Invoked from `doAppend()` under `mObjectGuard`. Formats the event using the `layoutSnapshot()` layout to `toLocal8Bit()` bytes. It then iterates a **copy** of the client list, not the member itself: when the logging call happens on the appender's own thread the write lambda runs synchronously, and a failed write calls `abort()`, which emits `disconnected()` synchronously — so `onClientDisconnected()` re-enters through the recursive mutex and removes the socket from `mTcpSockets` while the loop is running. Iterating the member directly would invalidate the iterators mid-loop. For each socket in the snapshot it marshals the write onto that socket's owner thread with `QMetaObject::invokeMethod(..., Qt::AutoConnection)`: a `write()` failure triggers `abort()` (which drops the client and fires `onClientDisconnected`), and on success an optional `flush()` is performed when `immediateFlush` is set. Marshalling is necessary because a `QTcpSocket` must be used only from its owning thread.

#### bool checkEntryConditions() const override

Returns `false` (logging `AppenderTelnetServerNotRunning`) if there is no server or it is not listening; otherwise chains to `AppenderSkeleton::checkEntryConditions()`.

#### void openServer()

Creates a `QTcpServer` parented to `this`, connects its `newConnection` signal to `onNewConnection()`, and calls `listen(address, port)`. A listen failure is logged with the server's `errorString()` attached as a causing error.

#### void closeServer()

Disconnects this appender from each client socket, `abort()`s and `deleteLater()`s each, clears the client list, then `close()`s and `deleteLater()`s the server. Sockets are reclaimed by the event loop rather than `delete`d immediately, to avoid crashing if the network layer is mid-callback.

#### QList<QTcpSocket *> clients() const

Returns the current list of connected client sockets.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr`.
- The `QTcpServer` is parented to the appender (`new QTcpServer(this)`) and additionally `deleteLater()`d in `closeServer()`.
- Client `QTcpSocket`s are produced by `nextPendingConnection()` (parented to the server). The appender tracks raw pointers in `mTcpSockets` and disposes of them via `deleteLater()` on disconnect or teardown — never raw `delete`, to stay safe against in-flight socket callbacks.
- The destructor calls `close()`, guaranteeing the server stops listening and all clients are torn down before destruction.

## 12. Thread Safety

All public functions are thread-safe. `activateOptions()`, `close()`, and the connection slots take `mObjectGuard`; `append()` runs under the same guard via `doAppend()`. The critical cross-thread concern is that `QTcpSocket` is thread-affine: `append()` may run on any logging thread, so it does **not** touch the sockets directly. Instead it posts the write to each socket's owning thread via `QMetaObject::invokeMethod(..., Qt::AutoConnection)` — a same-thread call runs inline, a cross-thread call is queued. `immediateFlush` is a `std::atomic<bool>`. The server and sockets are created and destroyed on the appender's thread under the guard.

## 14. Inter-Class Interactions

- Uses a `Layout` (via `layoutSnapshot()`) to render each event to text.
- Drives a `QTcpServer` and its `QTcpSocket` children, connecting their `newConnection`/`disconnected` signals to its own private slots.
- Reports listen/state failures through the internal `logger()` as `LogError`s.

## 15. External Communication

`TelnetAppender` exposes a TCP server for inbound client connections and streams log text outbound.

- **Network class / role:** `QTcpServer` listening on `address`:`port` (default `0.0.0.0:23`); each accepted connection is a `QTcpSocket`. The server is the listener; remote telnet/TCP clients initiate connections.
- **Direction:** clients connect **inbound**; log data flows **outbound** from the appender to all connected clients. The appender does not read client input — it is a read-only feed from the client's perspective.
- **Protocol / format:** raw bytes over TCP. Each log event is the layout-formatted string encoded with `toLocal8Bit()` and written to every client. An optional one-time welcome message (also `toLocal8Bit()`) is sent when a client connects. There is no telnet option negotiation — it is a plain byte stream that a telnet client can display.
- **Error handling:** a failed `listen()` is logged and leaves the server non-listening, after which `checkEntryConditions()` blocks appends. A failed `write()` aborts that client's socket, which triggers `onClientDisconnected()` to drop it from the list. Other clients are unaffected.
- **Threading implications:** sockets are accessed only on their owning thread; writes from other threads are marshalled via `QMetaObject::invokeMethod`. Acceptance and disconnect handling run in the appender's thread under `mObjectGuard`. A running event loop is required for connection acceptance and queued writes to be delivered.

## 16. Usage Example

```cpp
#include "log4qt/telnetappender.h"
#include "log4qt/patternlayout.h"
#include "log4qt/logger.h"

#include <QHostAddress>

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(QStringLiteral("%d{HH:mm:ss} %p %c - %m%n")));
layout->activateOptions();

auto *telnet = new TelnetAppender(layout, QHostAddress::Any, 9023);
telnet->setName(QStringLiteral("telnet"));
telnet->setWelcomeMessage(QStringLiteral("--- live log feed ---\r\n"));
telnet->setImmediateFlush(true);
telnet->activateOptions();                  // starts listening on 0.0.0.0:9023

Logger::rootLogger()->addAppender(AppenderSharedPtr(telnet));
Logger::rootLogger()->info(QStringLiteral("connect with: telnet localhost 9023"));
// Requires a running Qt event loop to accept connections and stream data.
```
