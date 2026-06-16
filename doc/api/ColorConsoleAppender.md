# ColorConsoleAppender

## 1. Class Overview

`ColorConsoleAppender` is a `ConsoleAppender` that renders ANSI colour escape sequences as real console colours. Layouts can embed ANSI SGR escape codes (e.g. `\033[31m` for red) in their output; on a terminal that understands ANSI those codes colourise the text directly. On Windows, where the legacy console does not natively interpret ANSI sequences, this appender parses the escape codes and translates them into Win32 console attribute calls so the colours still appear.

A developer uses it to get colourised, severity-highlighted console output. It is built **only on Windows** (see `CMakeLists.txt`, which appends `colorconsoleappender.cpp` inside the `if(WIN32)` block); on other platforms the plain `ConsoleAppender` already passes ANSI codes through unchanged, so the class is unnecessary.

## 2. Project Structure and Dependencies

Declared in `colorconsoleappender.h`; implemented in `colorconsoleappender.cpp`. Compiled only for Windows targets.

Build requirement: **Qt Core** — `QTextStream`, `QString`/`QStringList`. On Windows it includes `<windows.h>` (with `WIN32_LEAN_AND_MEAN` / `NOGDI`) for the console API (`GetStdHandle`, `GetConsoleScreenBufferInfo`, `SetConsoleTextAttribute`, `WriteConsoleW`, `OutputDebugString`).

Project-internal types:

- **`ConsoleAppender`** (`consoleappender.h`) — base providing the target, stream ownership, and `activateOptions()`/`close()` flow.
- **`AbstractLayout` / `LayoutSharedPtr`** — formats events (and is expected to embed ANSI escapes for colour).
- **`LoggingEvent`** — the event being written.

Standard library: `<vector>` (wide-character conversion buffer in the translation helper).

## 3. Class Hierarchy and Role

`QObject` → `Appender` → `AppenderSkeleton` → `WriterAppender` → `ConsoleAppender` → **`ColorConsoleAppender`**.

It inherits all of `ConsoleAppender`'s configuration (the `target` property, the `Target` enum, stream ownership). On Windows it overrides `activateOptions()`, `close()`, and `append()` to obtain a console `HANDLE` and perform colour-aware output. On non-Windows builds the class body adds nothing over `ConsoleAppender` (the overrides are guarded by `#ifdef Q_OS_WIN`).

## 4. Q_PROPERTY Declarations

Inherits the `target` property (and `Target` enum) from `ConsoleAppender`; declares no new properties of its own.

## 5. Public Methods

#### ColorConsoleAppender(QObject *parent = nullptr)

Constructs a colour console appender targeting `StdOut`, with no console handle yet.

#### ColorConsoleAppender(const LayoutSharedPtr &layout, QObject *parent = nullptr)

As above, with the given layout.

#### ColorConsoleAppender(const LayoutSharedPtr &layout, const QString &target, QObject *parent = nullptr)

Constructs with a layout and a string target (`"STDOUT_TARGET"` / `"STDERR_TARGET"`).

#### ColorConsoleAppender(const LayoutSharedPtr &layout, Target target, QObject *parent = nullptr)

Constructs with a layout and an explicit `ConsoleAppender::Target` enum value.

#### ~ColorConsoleAppender() override

Releases the console handle reference via the internal close path (the handle itself is process-owned and never closed).

#### void activateOptions() override *(Windows only)*

Calls `ConsoleAppender::activateOptions()` to create the stream, then acquires the standard console handle with `GetStdHandle` — `STD_OUTPUT_HANDLE` when the target is `"STDOUT_TARGET"`, otherwise `STD_ERROR_HANDLE`. Stores it in the private `hConsole` member used by `append()`.

#### void close() override *(Windows only)*

Releases the console handle reference via the internal close path, then chains to `ConsoleAppender::close()`. Idempotent.

## 6. Protected Methods

#### void append(const LoggingEvent &event) [override] *(Windows only)*

Overrides `ConsoleAppender::append()`. Formats the event via the layout, then passes the resulting string to the internal `colorOutputString()` helper, which:

- reads the current console attributes with `GetConsoleScreenBufferInfo` (falling back to `OutputDebugString` if the console is blocked by a debugger),
- splits the message on the ANSI escape character `\033`,
- parses each `[...m` SGR sequence, mapping the Unix colour codes (foreground, background, bold, default) to Win32 console attribute words,
- writes each text segment with `WriteConsoleW` after applying the translated attributes with `SetConsoleTextAttribute`, and
- restores the original console attributes when done.

After output it calls `handleIoErrors()` and, if `immediateFlush()` is set, flushes the underlying writer. Runs under `mObjectGuard` (Phase 5).

On non-Windows builds this override does not exist; the inherited `ConsoleAppender::append()` (which passes ANSI codes straight through) is used.

## 7. Ownership and Lifecycle

A `QObject` (parent-owned if given a parent) held by `AppenderSharedPtr` in the framework. It owns the same `QTextStream` as its `ConsoleAppender` base. The Windows console **`HANDLE` (`hConsole`) is not owned**: it is obtained from `GetStdHandle`, which returns a process-owned handle. As the close path comments note, it must **not** be passed to `CloseHandle`; the appender merely sets the member back to null on close.

## 8. Thread Safety

**Thread-safe**, via the base-class locking. `activateOptions()` and `close()` acquire `mObjectGuard`; `append()` runs serialised in Phase 5, so the parse-and-write sequence (which temporarily changes and then restores the console attributes) is not interleaved with other threads' output.

## 9. External Communication

Performs **OS-level coloured console output** on Windows.

- **Direction:** outbound only.
- **Channel:** the standard output or error console, addressed through a Win32 `HANDLE` and written with `WriteConsoleW`; colour is applied with `SetConsoleTextAttribute`.
- **Data format:** the layout's formatted text, in which ANSI SGR escape sequences are interpreted and translated to native console colours. On platforms other than Windows the same ANSI sequences are emitted verbatim to the terminal.
- **Debugger fallback:** if the console is blocked by a debugger (`GetConsoleScreenBufferInfo` fails), the message is sent to `OutputDebugString` instead.

## 10. Inter-Class Interactions

- **`ConsoleAppender`** provides stream creation, ownership, and the target selection that determines which standard handle is acquired.
- **`AbstractLayout`** formats events and is expected to embed ANSI colour escapes.
- **Configurators** set the `target` and layout from configuration.

## 11. Usage Example

```cpp
// Windows build only — the colour translation overrides are compiled there.
#include <log4qt/colorconsoleappender.h>
#include <log4qt/ttcclayout.h>   // or any layout that emits ANSI colour escapes

using namespace Log4Qt;

auto layout = LayoutSharedPtr::create<TTCCLayout>();
layout->activateOptions();

auto appender = AppenderSharedPtr::create<ColorConsoleAppender>(layout, ConsoleAppender::StdOut);
appender->setName(QStringLiteral("colorconsole"));
appender->activateOptions();   // creates the stream and acquires the console handle

// ANSI escapes produced by the layout are rendered as real colours in the Windows console.
appender->close();
```
