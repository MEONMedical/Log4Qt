# RandomAccessFileAppender

## 1. Class Overview

`RandomAccessFileAppender` is a high-throughput file appender that bypasses `QTextStream` and formats log events *outside* the appender lock. It is an alternative to `FileAppender` optimised for workloads with many concurrent threads or expensive layout patterns (e.g. `%d{ISO8601}`).

It improves on `FileAppender` in two complementary ways:

1. **Direct byte-buffer I/O.** Formatted messages are encoded to UTF-8 and accumulated in a pre-allocated `QByteArray` buffer (`bufferSize`, default 256 KB). The buffer is flushed to disk with a single `QFile::write()` when it fills or when the appender closes, eliminating the `QTextStream` codec layer and reducing write syscalls.
2. **Split-lock formatting.** The class overrides `preAppend()`, which `AppenderSkeleton::doAppend()` calls *outside* the appender mutex. Layout formatting and UTF-8 encoding happen there into a thread-local staging buffer; the mutex is held only for the subsequent `append()` that copies those bytes into the shared write buffer. Multiple threads can therefore format concurrently — only the buffer copy is serialised.

For maximum throughput it can be wrapped with `AsyncAppender`, which dispatches events on a background thread so calling threads never block on I/O.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/randomaccessfileappender.h`
- Source: `src/log4qt/randomaccessfileappender.cpp`

Direct dependencies:

- `AppenderSkeleton` (`appenderskeleton.h`) — base class; this appender inherits it **directly**, not via `FileAppender`/`WriterAppender`.
- `AbstractStringLayout` (`abstractstringlayout.h`) — provides `formatTo()` (zero-allocation UTF-8 formatting) and the shared thread-local staging buffer `threadLocalBuffer()`.
- `AbstractLayout` (`abstractlayout.h`) — supplies `header()`, `footer()`, and `endOfLine()`.
- `QFile`, `QDir`, `QFileInfo`, `QMutexLocker`; on Windows, `<windows.h>` for `ExpandEnvironmentStringsW`.

## 3. Class Hierarchy and Role

`QObject` → `Appender`/`AppenderSkeleton` → **`RandomAccessFileAppender`**

Unlike `FileAppender`, it does not derive from `WriterAppender` and does not use a `QTextStream`. It manages its own `QFile` and byte buffer. Its role is a drop-in, faster file sink for high-concurrency or high-volume logging.

## 4. Q_PROPERTY

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `appendFile` | `bool` | `appendFile()` | `setAppendFile()` | — | Whether output is appended to an existing file. Default `false` (truncate on open). |
| `file` | `QString` | `file()` | `setFile()` | — | Name (path) of the log file. |
| `bufferSize` | `int` | `bufferSize()` | `setBufferSize()` | — | Size in bytes of the in-memory write buffer. Default `262144` (256 KB). |
| `immediateFlush` | `bool` | `immediateFlush()` | `setImmediateFlush()` | — | Whether the buffer is flushed after every append. Default `false`. (Note: `FileAppender` defaults this to `true`.) |

## 5. Enumerations

None.

## 6. Public Member Variables

None. All state (`mAppendFile`, `mBufferSize`, `mImmediateFlush` as `std::atomic`; `mFileName`, `mByteBuffer`, `mFile` guarded by the mutex) is private.

## 7. Signals

None declared beyond those inherited.

## 8. Public Slots & Q_INVOKABLE

None declared. Configuration is via the property setters below.

## 9. Public Methods

#### bool appendFile() const
Returns the `appendFile` property (atomic, relaxed read).

#### QString file() const
Returns the configured file name. Thread-safe (mutex-guarded).

#### int bufferSize() const
Returns the write-buffer size in bytes (atomic, relaxed read).

#### bool immediateFlush() const
Returns the `immediateFlush` property (atomic, relaxed read).

#### void setAppendFile(bool append)
Sets append-vs-truncate behaviour (atomic store). Takes effect at the next `openFile()`.

#### void setFile(const QString &fileName)
Sets the log file path. Thread-safe.

#### void setBufferSize(int bufferSize)
Sets the buffer size. If a file is currently open, the underlying `QByteArray` capacity is re-reserved to the new size. Thread-safe.

#### void setImmediateFlush(bool immediateFlush)
Sets whether each append is flushed immediately (atomic store).

#### bool requiresLayout() const
Returns `true` — this appender always requires a layout. Overrides `AppenderSkeleton::requiresLayout()`.

#### void activateOptions()
Validates that a file name is set (logs `AppenderActivateMissingFileError` and returns if not), closes any open file, opens the new file, reserves the buffer capacity, and chains to `AppenderSkeleton::activateOptions()` only if the file opened successfully. Thread-safe. Overrides `AppenderSkeleton::activateOptions()`.

#### void close()
Flushes and closes the file (via `closeInternal()` → `closeFile()`), then chains to `AppenderSkeleton::close()`. Overrides `AppenderSkeleton::close()`.

#### ~RandomAccessFileAppender()
Destructor. Calls `closeInternal()`, guaranteeing that all buffered data is flushed to disk even if `close()` was never called explicitly.

## 10. Protected Virtual Methods

#### void preAppend(const LoggingEvent &event, const LayoutSharedPtr &layout)
Called by `AppenderSkeleton::doAppend()` **outside** `mObjectGuard`. Clears the thread-local staging buffer and formats `event` into it: via `AbstractStringLayout::formatTo()` when the layout is an `AbstractStringLayout` (no intermediate `QString` allocation), otherwise via `layout->format(event).toUtf8()`. This moves the expensive formatting work out of the locked region. Overrides `AppenderSkeleton::preAppend()`.

#### void append(const LoggingEvent &event)
Runs under `mObjectGuard`. Reads the bytes that `preAppend()` produced in the thread-local buffer; if empty (e.g. a `close()` raced), does nothing. If appending the bytes would exceed `bufferSize`, flushes first, then appends the bytes to the shared buffer and clears the staging buffer. If `immediateFlush` is set, flushes again. Overrides `AppenderSkeleton::append()`.

#### bool checkEntryConditions() const
Returns `false` (logging `AppenderNoOpenFileError`) if no file is open; otherwise delegates to `AppenderSkeleton::checkEntryConditions()`. Overrides the skeleton hook.

#### bool handleIoErrors() const
Inspects `QFile::error()`; if not `NoError`, logs an `AppenderWritingFileError` (with the underlying file error as cause) and returns `true`. Otherwise returns `false`.

#### void flushBuffer()
Writes the accumulated buffer to the file with a single `QFile::write()`, checks for I/O errors, and clears the buffer (preserving its reserved capacity for reuse). No-op when the buffer is empty.

#### virtual void openFile()
Opens the log file for writing. Creates the parent directory if missing (logging `AppenderOpeningFileError` on failure), and on Windows expands environment variables in the path via `ExpandEnvironmentStringsW`. Opens in `WriteOnly` mode with `Append` or `Truncate` depending on `appendFile` — **without** `QIODevice::Text` (raw UTF-8 is written; the layout's `endOfLine()` already supplies the platform line ending) and without `Unbuffered` (the class manages its own buffer). On open failure logs an error and resets the file. For a new/empty file, the layout header (if any) is staged into the buffer so it is part of the first flush. Declared `virtual` so rolling subclasses may override.

#### void closeFile()
If a file is open, stages the layout footer (if any) into the buffer, performs a final `flushBuffer()`, then resets the `QFile` and clears the buffer.

#### bool removeFile(QFile &file) const
Removes `file`; logs `AppenderRemoveFileError` and returns `false` on failure, otherwise `true`.

#### bool renameFile(QFile &file, const QString &fileName) const
Renames `file` to `fileName`; logs `AppenderRenamingFileError` and returns `false` on failure, otherwise `true`.

## 11. Ownership and Lifecycle

The `QFile` is owned through a `std::unique_ptr<QFile>` (`mFile`), created in `openFile()` and reset in `closeFile()`. The layout is held as a `LayoutSharedPtr` via the appender skeleton. The destructor guarantees a flush-and-close, so no data is lost even without an explicit `close()`. The appender follows the library's managed-ownership convention.

## 12. Thread Safety

All public functions are thread-safe. The mutable file/buffer state (`mFileName`, `mByteBuffer`, `mFile`) is guarded by the inherited recursive `mObjectGuard` mutex; the scalar configuration flags (`mAppendFile`, `mBufferSize`, `mImmediateFlush`) are `std::atomic` and read/written with relaxed ordering, so getters/setters for them are lock-free. The key concurrency feature is the split lock: `preAppend()` formats outside the mutex into a per-thread buffer, and only the short `append()` copy into the shared buffer is serialised.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- Pairs naturally with `AsyncAppender` (background-thread dispatch) for end-to-end non-blocking logging.
- Relies on `AbstractStringLayout::threadLocalBuffer()` / `formatTo()` for allocation-free formatting; falls back to any `AbstractLayout` via `format()`.
- Functionally analogous to `FileAppender` but does not participate in the `WriterAppender`/`QTextStream` chain, and is **not** the base of the rolling appenders (those extend `FileAppender`).

## 15. External Communication

A single owned `QFile`. Writes are batched into one `write()` call per flush; flushes occur when the buffer fills, on `immediateFlush`, and on close/destruction. Parent directories are auto-created and Windows environment variables in the path are expanded. No network or IPC.

## 16. Usage Example

```cpp
#include "log4qt/randomaccessfileappender.h"
#include "log4qt/asyncappender.h"
#include "log4qt/patternlayout.h"

using namespace Log4Qt;

auto layout = LayoutSharedPtr(new PatternLayout(u"%d{ISO8601} [%t] %-5p %c - %m%n"_s));
layout->activateOptions();

auto *file = new RandomAccessFileAppender(layout, u"fast.log"_s, /*append*/ true);
file->setBufferSize(1 * 1024 * 1024);   // 1 MB write buffer
file->setImmediateFlush(false);          // batch writes for throughput
file->activateOptions();

// Optional: wrap for non-blocking logging on caller threads.
auto *async = new AsyncAppender;
async->addAppender(AppenderSharedPtr(file));
async->activateOptions();
// ... attach `async` to a Logger.
```
