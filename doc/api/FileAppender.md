# FileAppender

## 1. Class Overview

`FileAppender` writes formatted log events to a file on disk. It opens a `QFile`, wraps it in a `QTextStream`, and hands that stream to its `WriterAppender` base for the actual writing. It adds file-specific configuration: the **file name**, whether to **append** to an existing file or truncate it, and whether I/O is **buffered**. It also creates any missing parent directories, expands environment variables in the path on Windows, and reports file I/O errors back through the logging system.

`FileAppender` is the base for the rolling and time-based file appenders (`RollingFileAppender`, `DailyRollingFileAppender`, `RandomAccessFileAppender`), which reuse its `openFile()` / `closeFile()` / `renameFile()` / `removeFile()` primitives.

## 2. Project Structure and Dependencies

Declared in `fileappender.h`; implemented in `fileappender.cpp`. It is the base of `RollingFileAppender`, `DailyRollingFileAppender`, and `RandomAccessFileAppender`.

Build requirement: **Qt Core** — `QFile`, `QTextStream`, `QDir`, `QFileInfo`. On Windows it includes `<windows.h>` for `ExpandEnvironmentStringsW`.

Project-internal types:

- **`WriterAppender`** (`writerappender.h`) — base providing `append()`, flushing, header/footer, and writer plumbing.
- **`AbstractLayout` / `LayoutSharedPtr`** — formats events.
- **`LoggingEvent`** — the event being written.

Standard library: `<memory>` (`std::unique_ptr<QFile>`, `std::unique_ptr<QTextStream>`), `<atomic>` for the boolean flags.

## 3. Class Hierarchy and Role

`QObject` → `Appender` → `AppenderSkeleton` → `WriterAppender` → **`FileAppender`**.

It adds the `appendFile`, `bufferedIo`, and `file` properties, owns the `QFile` and `QTextStream`, overrides `activateOptions()` and `close()`, overrides the protected `checkEntryConditions()`, `handleIoErrors()`, and `writeHeader()` hooks, and introduces the protected file primitives `openFile()`, `closeFile()`, `renameFile()`, `removeFile()`. Copy/move are disabled via `Q_DISABLE_COPY_MOVE`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `appendFile` | `bool` | `appendFile` | `setAppendFile` | — | If `true`, new output is appended to an existing file; if `false` (default), the file is truncated on open. Stored atomically. |
| `bufferedIo` | `bool` | `bufferedIo` | `setBufferedIo` | — | If `true` (default), file I/O is buffered; if `false`, the file is opened `Unbuffered`. Stored atomically. |
| `file` | `QString` | `file` | `setFile` | — | The path of the log file. Read/written under the object lock. Takes effect on the next `activateOptions()`. |

## 5. Public Methods

#### explicit FileAppender(QObject *parent = nullptr)

Constructs an appender with no file name, append disabled, buffering enabled.

#### FileAppender(const LayoutSharedPtr &layout, const QString &fileName, QObject *parent = nullptr)

Constructs with a layout and target file name; append disabled, buffering enabled.

#### FileAppender(const LayoutSharedPtr &layout, const QString &fileName, bool append, QObject *parent = nullptr)

As above, with explicit append mode; buffering enabled.

#### FileAppender(const LayoutSharedPtr &layout, const QString &fileName, bool append, bool buffered, QObject *parent = nullptr)

Full constructor with explicit append and buffered flags.

#### ~FileAppender() override

Closes the file via the internal close path (flushing and writing the footer through the base, then releasing the stream and file).

#### bool appendFile() const

Returns the append-mode flag. Inline.

#### QString file() const

Returns the configured file path. Acquires `mObjectGuard`.

#### bool bufferedIo() const

Returns the buffering flag. Inline.

#### void setAppendFile(bool append)

Sets append mode. Inline atomic store. Takes effect on the next `activateOptions()`.

#### void setBufferedIo(bool buffered)

Sets buffering. Inline atomic store. Takes effect on the next `activateOptions()`.

#### void setFile(const QString &fileName)

Sets the file path under `mObjectGuard`. Takes effect on the next `activateOptions()`.

#### void activateOptions() override

Validates that a file name is set; if empty, logs `AppenderActivateMissingFileError` and stays inactive. Otherwise it closes any currently open file, opens the new file (`openFile()`), and chains to `WriterAppender::activateOptions()` (which validates the writer/layout and activates). Acquires `mObjectGuard`.

#### void close() override

Closes the file via the internal close path, then chains to `WriterAppender::close()`. Idempotent.

## 6. Protected Virtual Methods

#### bool checkEntryConditions() const [override]

Defined up the chain by `AppenderSkeleton`. Adds the check that both the `QFile` and the `QTextStream` exist (file is open); if not, logs `AppenderNoOpenFileError` and returns `false`. Otherwise chains to `WriterAppender::checkEntryConditions()` (which checks the writer) and then `AppenderSkeleton::checkEntryConditions()`. Runs in `doAppend()` Phase 3 under the lock.

#### void closeFile()

Detaches the writer (`setWriter(nullptr)`, writing the footer), then destroys the owned `QTextStream` and `QFile`. Logs a debug message naming the closed file.

#### bool handleIoErrors() const [override]

Defined by `WriterAppender` (which returns `false`). This override inspects the underlying `QFile::error()`; on any error other than `NoError` it logs `AppenderWritingFileError` (with the file's `errorString()` attached as a causing error) and returns `true`, otherwise returns `false`. Called by the base `append()` after each write/flush.

#### virtual void openFile()

Opens the configured file for the appender. It asserts no file is already open, creates the parent directory if it does not exist (logging `AppenderOpeningFileError` on failure), and on Windows expands environment variables in the path via `ExpandEnvironmentStringsW`. It opens the `QFile` with `WriteOnly | Text`, plus `Append` or `Truncate` according to `appendFile`, plus `Unbuffered` when buffering is disabled; on failure it logs `AppenderOpeningFileError`. When appending to a non-empty existing file it sets a one-shot flag to suppress the next header (the header is already present from the prior run). It then creates the `QTextStream` over the file and installs it as the writer via `setWriter()`. Subclasses override to customise file opening for rollover.

#### bool removeFile(QFile &file) const

Removes `file`. Returns `true` on success; on failure logs `AppenderRemoveFileError` (with the file error attached) and returns `false`. Used by rolling subclasses.

#### bool renameFile(QFile &file, const QString &fileName) const

Renames `file` to `fileName`. Returns `true` on success; on failure logs `AppenderRenamingFileError` and returns `false`. Used by rolling subclasses to rotate log files.

#### void writeHeader() const [override]

Defined by `WriterAppender`. This override honours the one-shot header-suppression flag set by `openFile()` when appending to a non-empty file: if set, it consumes the flag and writes nothing; otherwise it delegates to `WriterAppender::writeHeader()`.

## 7. Ownership and Lifecycle

A `QObject` (parent-owned if given a parent) held by `AppenderSharedPtr` in the framework. It **owns** both the `QFile` (`std::unique_ptr<QFile> mFile`) and the `QTextStream` over it (`std::unique_ptr<QTextStream> mTextStream`). `closeFile()` clears the base writer pointer first, then resets the stream, then the file — the correct teardown order. The destructor calls the internal close path so the footer is flushed and the file handle released even if `close()` was not called explicitly. The base `WriterAppender` does not own the stream; `FileAppender` does.

## 8. Thread Safety

**Thread-safe.** File path and flags are read/written under `mObjectGuard` (path) or via atomics (append/buffered). `activateOptions()`, `close()`, and the open/close primitives lock the guard, and `append()` (inherited from `WriterAppender`) writes under the same lock in Phase 5, so writes to the file are serialised across threads.

## 9. External Communication

Performs **file-system I/O**.

- **Direction:** outbound only (writes log lines to a file).
- **Channel:** a `QFile` opened `WriteOnly | Text`, optionally `Append`/`Truncate` and `Unbuffered`, wrapped in a `QTextStream` with the configured encoding.
- **Data format:** the layout's formatted text per event, plus optional header/footer.
- **Path handling:** missing parent directories are created; on Windows, environment variables embedded in the path are expanded.
- **Error handling:** open, write, rename, and remove failures are logged through the framework as structured `LogError`s (`AppenderOpeningFileError`, `AppenderWritingFileError`, `AppenderRenamingFileError`, `AppenderRemoveFileError`) rather than thrown.

## 10. Inter-Class Interactions

- **`WriterAppender`** provides the write/flush/header/footer machinery; `FileAppender` supplies and owns the file-backed stream and the I/O-error inspection.
- **`AbstractLayout`** formats each event and supplies header/footer text.
- **Rolling subclasses** (`RollingFileAppender`, `DailyRollingFileAppender`) call `closeFile()`, `openFile()`, `renameFile()`, and `removeFile()` to rotate files.
- **Internal `Logger`** reports all file-related errors.
- **Configurators** set the `file`, `appendFile`, and `bufferedIo` properties and the layout.

## 11. Usage Example

```cpp
#include <log4qt/fileappender.h>
#include <log4qt/ttcclayout.h>

using namespace Log4Qt;

auto layout = LayoutSharedPtr::create<TTCCLayout>();
layout->activateOptions();

// Append to an existing file, buffered I/O.
auto appender = AppenderSharedPtr::create<FileAppender>(
    layout, QStringLiteral("logs/app.log"), /*append*/ true, /*buffered*/ true);
appender->setName(QStringLiteral("file"));
appender->activateOptions();   // creates dirs as needed, opens the file, activates

// Attach `appender` to a logger; events are now written to logs/app.log.
appender->close();             // flushes, writes the footer, closes the file
```
