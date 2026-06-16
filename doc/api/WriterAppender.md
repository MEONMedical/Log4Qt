# WriterAppender

## 1. Class Overview

`WriterAppender` is the appender that writes formatted log events to a **`QTextStream`**. It is the common base for every text-stream-backed appender in Log4Qt: `ConsoleAppender` (and its `ColorConsoleAppender` subclass) and `FileAppender` (and its rolling/daily descendants). It adds three things on top of `AppenderSkeleton`:

- a target `QTextStream` (the "writer"),
- a character **encoding** applied to that stream, and
- an **immediate flush** policy controlling whether the stream is flushed after every event.

It also defines the header/footer write points and an I/O-error hook that subclasses specialise. A developer rarely instantiates `WriterAppender` directly with a bare stream, but it is the reusable building block behind the concrete console and file appenders.

## 2. Project Structure and Dependencies

Declared in `writerappender.h`; implemented in `writerappender.cpp`. Direct subclasses: `ConsoleAppender`, `FileAppender` (and, transitively, `ColorConsoleAppender`, `RollingFileAppender`, `DailyRollingFileAppender`, `RandomAccessFileAppender`).

Build requirement: **Qt Core** — `QTextStream` (forward-declared in the header, included in the `.cpp`) and `QStringConverter`.

Project-internal types:

- **`AppenderSkeleton`** (`appenderskeleton.h`) — the base providing the `doAppend()` lifecycle, locking, layout, filters, and threshold.
- **`AbstractLayout` / `LayoutSharedPtr`** — formats the event and supplies `header()`, `footer()`, and `endOfLine()`.
- **`LoggingEvent`** — the event being formatted and written.

Standard library: `<atomic>` (via the base) for the `mImmediateFlush` flag; `QStringConverter::Encoding` for the codec.

## 3. Class Hierarchy and Role

`QObject` → `Appender` → `AppenderSkeleton` → **`WriterAppender`**.

It implements `requiresLayout()` to return `true` (a writer appender always needs a layout to produce text), implements the pure virtual `append()`, and overrides `checkEntryConditions()`, `activateOptions()`, and `close()`. It introduces the `writeHeader()` / `writeFooter()` / `handleIoErrors()` virtual hooks for subclasses. Copy/move are disabled via `Q_DISABLE_COPY_MOVE`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `encoding` | `QStringConverter::Encoding` | `encoding` | `setEncoding` | — | The character encoding applied to the writer's `QTextStream`. Setting it overrides the encoding the stream already had. Defaults to `Utf8` (default-constructed appender) or `System` (layout-based constructors). |
| `writer` | `QTextStream *` | `writer` | `setWriter` | — | The target text stream. The appender does **not** own the stream — see Ownership. Setting a new writer first closes the old one (writing its footer), then writes the new header. |
| `immediateFlush` | `bool` | `immediateFlush` | `setImmediateFlush` | — | If `true` (default), the stream is flushed after every event so output appears immediately; if `false`, output is buffered. Stored atomically. |

## 5. Public Methods

#### WriterAppender(QObject *parent = nullptr)

Constructs an inactive appender with no writer, `Utf8` encoding, and immediate flush enabled.

#### WriterAppender(const LayoutSharedPtr &layout, QObject *parent = nullptr)

Constructs an inactive appender with the given layout, `System` encoding, no writer, immediate flush enabled.

#### WriterAppender(const LayoutSharedPtr &layout, QTextStream *textStream, QObject *parent = nullptr)

Constructs an inactive appender with the given layout and target stream, `System` encoding, immediate flush enabled. The stream is **not** owned.

#### ~WriterAppender() override

Closes the writer (writing the footer if applicable) via the internal close path. Does not delete the stream.

#### bool requiresLayout() const [override]

Returns `true` — a writer appender always needs a layout.

#### QStringConverter::Encoding encoding() const

Returns the configured encoding. Inline, acquires `mObjectGuard`.

#### bool immediateFlush() const

Returns the immediate-flush flag. Inline atomic read.

#### QTextStream *writer() const

Returns the current target stream (or null). Inline.

#### void setEncoding(QStringConverter::Encoding encoding)

Sets the encoding and, if a writer is attached, applies it to the stream immediately via `QTextStream::setEncoding`. No-op if the encoding is unchanged. Acquires `mObjectGuard`.

#### void setImmediateFlush(bool immediateFlush)

Enables or disables flushing after each write. Inline atomic store.

#### void setWriter(QTextStream *textStream)

Replaces the target stream. Closes the previous writer first (writing its footer), assigns the new stream, applies the current encoding, then writes the new header. Passing `nullptr` detaches the writer. Acquires `mObjectGuard`.

#### void activateOptions() override

Validates that a writer has been set; if not, logs `AppenderActivateMissingWriterError` and stays inactive. Otherwise calls `AppenderSkeleton::activateOptions()` (which checks the layout requirement and marks the appender active). Acquires `mObjectGuard`.

#### void close() override

Closes the writer (writing the footer through the internal close path), then calls `AppenderSkeleton::close()` to mark the appender closed and inactive. Idempotent.

## 6. Protected Virtual Methods

#### void append(const LoggingEvent &event) [override]

Defined by `AppenderSkeleton` as pure virtual; implemented here. Runs in Phase 5 under `mObjectGuard`. It reads the layout via `layoutSnapshot()` (avoiding an extra mutex acquisition and shared-pointer copy), formats the event, and writes it to the stream with `operator<<`. After writing it calls `handleIoErrors()`; if that reports an error it returns. If `immediateFlush()` is set it flushes the stream and checks `handleIoErrors()` again. Subclasses such as `ConsoleAppender` and `ColorConsoleAppender` override `append()` and may delegate back here with `WriterAppender::append(event)`.

#### bool checkEntryConditions() const [override]

Defined by `AppenderSkeleton`. Adds the check that a writer has been set; if not, logs `AppenderUseMissingWriterError` and returns `false`. Otherwise chains to `AppenderSkeleton::checkEntryConditions()`. Called from `doAppend()` Phase 3 under the lock.

#### void closeWriter()

Detaches the current stream: writes the footer (via `writeFooter()`) and sets the writer pointer to null. Does not delete the stream. Called by `setWriter()`, `close()`, and subclass close paths (which themselves own and destroy the underlying device).

#### virtual bool handleIoErrors() const

Hook returning whether an I/O error occurred on the last operation. The base implementation always returns `false`. `FileAppender` overrides it to inspect the underlying `QFile` and log `AppenderWritingFileError`.

#### virtual void writeFooter() const

Writes the layout's `footer()` followed by `AbstractLayout::endOfLine()` to the stream, if a layout and writer are present and the footer is non-empty. If a pending footer suppression is set (see `suppressNextFooter()`), it consumes the flag and writes nothing. Subclasses may override.

#### virtual void writeHeader() const

Writes the layout's `header()` followed by `AbstractLayout::endOfLine()` to the stream, if a layout and writer are present and the header is non-empty. `FileAppender` overrides it to skip the header when appending to a non-empty existing file.

#### void suppressNextFooter()

Sets a one-shot flag so the next `writeFooter()` call writes nothing. Call before closing a file when the footer should be omitted — for example during a startup rollover.

## 7. Ownership and Lifecycle

`WriterAppender` is a `QObject` (parent-owned if a parent is given) and is held by `AppenderSharedPtr` in the framework. The crucial ownership detail is the **writer (`QTextStream *`) is not owned** by this base class — `setWriter()` only stores the pointer and `closeWriter()` only nulls it. Concrete subclasses own the stream and the underlying device: `ConsoleAppender` holds the stream in a `std::unique_ptr<QTextStream>`, and `FileAppender` holds both a `std::unique_ptr<QTextStream>` and a `std::unique_ptr<QFile>`, resetting them in their close paths. Header/footer text is written automatically on attach/detach of a writer.

## 8. Thread Safety

**Thread-safe**, inheriting the locking design of `AppenderSkeleton`. Configuration methods (`setWriter`, `setEncoding`, `activateOptions`, `close`) acquire `mObjectGuard`. `append()` runs under the same lock in Phase 5, so writes and flushes to the stream are serialised. `immediateFlush` is an atomic for lock-free reads.

## 9. External Communication

`WriterAppender` performs OS-level text output through its `QTextStream`. The base class itself is destination-agnostic — the stream may be wrapping stdout/stderr (via `ConsoleAppender`) or a file (via `FileAppender`). All actual byte output happens in `append()`, `writeHeader()`, and `writeFooter()`. The data format is the layout's formatted string, encoded per the `encoding` property, with line endings from `AbstractLayout::endOfLine()`.

## 10. Inter-Class Interactions

- **`AbstractLayout`** supplies the formatted event text plus header/footer/end-of-line.
- **`QTextStream`** (externally owned, or owned by a subclass) is the write target.
- **Subclasses** (`ConsoleAppender`, `FileAppender`) supply and own the concrete stream, override `append()`, `handleIoErrors()`, and `writeHeader()`, and call back into `WriterAppender::activateOptions()` / `close()` / `append()`.
- **Internal `Logger`** reports missing-writer and I/O errors.

## 11. Usage Example

```cpp
#include <log4qt/writerappender.h>
#include <log4qt/ttcclayout.h>
#include <log4qt/loggingevent.h>
#include <QTextStream>

using namespace Log4Qt;

QString sink;
QTextStream stream(&sink);

auto layout = LayoutSharedPtr::create<TTCCLayout>();   // any AbstractLayout subclass
layout->activateOptions();

auto appender = AppenderSharedPtr::create<WriterAppender>(layout, &stream);
appender->setName(QStringLiteral("memory"));
appender->setImmediateFlush(true);
appender->activateOptions();        // fails (and stays inactive) if no writer/layout

// Events routed to this appender are now formatted by `layout` and written to `sink`.
// The QTextStream and its backing QString must outlive the appender — it does not own them.
appender->close();                  // writes the layout footer, detaches the stream
```
