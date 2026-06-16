# ConsoleAppender

## 1. Class Overview

`ConsoleAppender` writes formatted log events to the process console — either **standard output** (`stdout`) or **standard error** (`stderr`). It is the simplest "visible" appender and the one most configurations start with. It wraps the chosen C standard stream in a `QTextStream`, hands that stream to its `WriterAppender` base, and on Windows additionally routes output to the debugger via `OutputDebugString` when no console is attached.

A developer reaches for `ConsoleAppender` whenever log output should appear in the terminal or be captured by a parent process's stdout/stderr.

## 2. Project Structure and Dependencies

Declared in `consoleappender.h`; implemented in `consoleappender.cpp`. It is the base of `ColorConsoleAppender` (Windows-only build addition).

Build requirement: **Qt Core** — `QTextStream`. On Windows it also includes `<windows.h>` for `GetConsoleWindow` and `OutputDebugString`.

Project-internal types:

- **`WriterAppender`** (`writerappender.h`) — base that owns the `append()`/flush logic and the writer plumbing.
- **`OptionConverter`** (`helpers/optionconverter.h`) — converts a string target name (e.g. `"STDOUT_TARGET"`) to the `Target` enum in `setTarget(const QString &)`.
- **`AbstractLayout` / `LayoutSharedPtr`** — formats events.
- **`LoggingEvent`** — the event being written.

Standard library: `<memory>` (`std::unique_ptr<QTextStream>`), `<atomic>` (`std::atomic<Target>`).

## 3. Class Hierarchy and Role

`QObject` → `Appender` → `AppenderSkeleton` → `WriterAppender` → **`ConsoleAppender`**.

It adds the `target` property and `Target` enum, owns the `QTextStream` it creates over the chosen standard stream, and overrides `activateOptions()`, `close()`, and `append()`. Copy/move are disabled via `Q_DISABLE_COPY_MOVE`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `target` | `QString` | `target` | `setTarget` | — | The output destination as a string: `"STDOUT_TARGET"` or `"STDERR_TARGET"`. Defaults to standard out. The string form is primarily for configuration; an enum overload of the setter is also available. |

## 5. Enumerations (Q_ENUM)

`Target` — selects the console stream. Used by `target()` / `setTarget()`.

| Value | Integer | Description |
|-------|---------|-------------|
| `StdOut` | 0 | Output goes to standard output (`stdout`). The default. |
| `StdErr` | 1 | Output goes to standard error (`stderr`). |

## 6. Public Methods

#### ConsoleAppender(QObject *parent = nullptr)

Constructs an appender targeting `StdOut` with no stream yet (created in `activateOptions()`).

#### ConsoleAppender(const LayoutSharedPtr &pLayout, QObject *parent = nullptr)

As above, with the given layout. Targets `StdOut`.

#### ConsoleAppender(const LayoutSharedPtr &pLayout, const QString &target, QObject *parent = nullptr)

Constructs with a layout and a string target (`"STDOUT_TARGET"` / `"STDERR_TARGET"`), parsed via `OptionConverter`.

#### ConsoleAppender(const LayoutSharedPtr &pLayout, Target target, QObject *parent = nullptr)

Constructs with a layout and an explicit `Target` enum value.

#### ~ConsoleAppender() override

Closes the stream via the internal close path.

#### QString target() const

Returns `"STDOUT_TARGET"` or `"STDERR_TARGET"` depending on the current target.

#### void setTarget(const QString &target)

Sets the target from a string; the string is converted through `OptionConverter::toTarget`. If the string is not recognised, the target is left unchanged.

#### void setTarget(Target target)

Sets the target from the enum. Inline atomic store. Note: changing the target takes effect on the next `activateOptions()`, which (re)creates the stream over the chosen standard stream.

#### void activateOptions() override

Closes any existing stream, creates a fresh `QTextStream` over `stdout` or `stderr` according to the current target, and installs it as the writer via `setWriter()`. On Windows it then decides whether to route to `OutputDebugString`: this is enabled when there is no console window (`GetConsoleWindow() == nullptr`) and the `QT_ASSUME_STDERR_HAS_CONSOLE` environment variable is not set. Finally it chains to `WriterAppender::activateOptions()`. Acquires `mObjectGuard`.

#### void close() override

Closes the stream via the internal close path, then chains to `WriterAppender::close()`. Idempotent.

## 7. Protected Methods

#### void closeStream()

Detaches the writer (`setWriter(nullptr)`, which writes the footer) and destroys the owned `QTextStream`.

#### void append(const LoggingEvent &event) [override]

Overrides `WriterAppender::append()`. On non-Windows builds it simply delegates to `WriterAppender::append(event)`. On Windows, when `OutputDebugString` routing is active (no console + debugger present), it formats the event via `layoutSnapshot()` and emits the message through `OutputDebugString` instead of the stream; otherwise it delegates to the base. Runs under `mObjectGuard` (Phase 5).

## 8. Ownership and Lifecycle

`ConsoleAppender` is a `QObject` (parent-owned if given a parent) and is held by `AppenderSharedPtr` in the framework. It **owns** the `QTextStream` it creates, stored in `std::unique_ptr<QTextStream> mtextStream`; the stream wraps the process-owned `stdout`/`stderr` FILE handles, which are never closed by the appender. The stream is destroyed in `closeStream()` (called from `activateOptions()`, `close()`, and the destructor). The base class's writer pointer is cleared before the unique_ptr is reset, so the dangling-pointer ordering is safe.

## 9. Thread Safety

**Thread-safe.** Configuration and stream creation occur under `mObjectGuard`; `append()` runs serialised in Phase 5. The target is an atomic. Concurrent logging from multiple threads is interleaved safely at the line level by the base-class locking.

## 10. External Communication

This is the appender's defining behaviour: it performs **OS-level console output**.

- **Direction:** outbound only.
- **Channel:** the process's `stdout` or `stderr` stream (selected by `target`), wrapped in a `QTextStream` with the configured encoding.
- **Data format:** the layout's formatted text for each event.
- **Windows fallback:** when no console window is attached and a debugger is present, output is sent to `OutputDebugString` (visible in the debugger output pane) rather than the standard stream. This is decided once in `activateOptions()`.

## 11. Inter-Class Interactions

- **`WriterAppender`** provides the write/flush/header/footer machinery; `ConsoleAppender` supplies and owns the stream.
- **`AbstractLayout`** formats each event.
- **`OptionConverter`** maps configuration strings to the `Target` enum.
- **Configurators** set the `target` and layout properties from configuration files.

## 12. Usage Example

```cpp
#include <log4qt/consoleappender.h>
#include <log4qt/ttcclayout.h>

using namespace Log4Qt;

auto layout = LayoutSharedPtr::create<TTCCLayout>();
layout->activateOptions();

auto appender = AppenderSharedPtr::create<ConsoleAppender>(layout, ConsoleAppender::StdErr);
appender->setName(QStringLiteral("console"));
appender->activateOptions();        // creates the QTextStream over stderr and activates

// Attach `appender` to a logger (e.g. the root logger) so events are written to stderr.
// On shutdown:
appender->close();
```
