# LogError

## 1. Class Overview

`LogError` is a structured error value type used throughout Log4Qt's internal error-reporting path. It captures an error in a way that keeps the *message template* separate from the *substitution arguments*, so that all information remains accessible after the error has been raised. This separation makes it possible to translate the message at a later point in time, or to obtain both a translated text (for a UI) and an untranslated text (for a log) from the same error object.

An error carries:

- a message template (`message()`),
- a list of arguments to be substituted into the template (`args()`),
- an integer error `code()` and/or a symbolic `symbol()`,
- a translation `context()` used with `QCoreApplication::translate()`, and
- a list of *causing errors* (`causingErrors()`) that form a chain of related failures.

The class also maintains a per-thread "last error" slot accessible via the static `lastError()` / `setLastError()` pair.

Two convenience macros simplify construction:

- `LOG4QT_ERROR(message, code, context)` — for classes **not** derived from `QObject`. Expands to `LogError(message, code, #code, context)`, so the symbol is set to the stringized error-code identifier.
- `LOG4QT_QCLASS_ERROR(message, code)` — for classes derived from `QObject`. Expands to `LogError(message, code, #code, this->metaObject()->className())`, taking the context from the runtime class name.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/helpers/logerror.h`
- Source: `src/log4qt/helpers/logerror.cpp`

Header dependencies: `log4qt/log4qtshared.h` (for the `LOG4QT_EXPORT` macro), `QString`, `QVariant`.

Source dependencies: `QBuffer`, `QByteArray`, `QDataStream`, `QCoreApplication` (for `translate()`), `QThreadStorage` (for the per-thread last-error slot), and a `Qt::StringLiterals` using-directive (for the `u"..."_s` string literal helper).

The integer error codes used with `LogError` (for example `LayoutExpectedDigitError`, `ConfiguratorInvalidOptionError`) are defined in the library-wide `ErrorCode` enum in `src/log4qt/log4qt.h`.

## 3. Class Hierarchy and Role

`LogError` is a standalone value/error type. It does **not** derive from `QObject` and has no base class. It is copyable and is registered as a movable, metatype-enabled type:

- `Q_DECLARE_METATYPE(Log4Qt::LogError)` — usable inside `QVariant`.
- `Q_DECLARE_TYPEINFO(Log4Qt::LogError, Q_MOVABLE_TYPE)` — relocatable for efficient container storage.

Errors compose recursively: each `LogError` can hold a list of other `LogError` instances as causing errors, forming a tree that `toString()` renders depth-first.

## 4. Q_PROPERTY Table

None. `LogError` is not a `QObject` and declares no `Q_PROPERTY` members.

## 5. Enumerations

| Enum | Underlying type | Value | Description |
|------|-----------------|-------|-------------|
| `Encoding` | `int` | `Latin1` | The `const char *` message argument is Latin-1 encoded. |
| `Encoding` | `int` | `CodecForTr` | The encoding specified by `QTextCodec::codecForTr()` (Latin-1 if none has been set). |
| `Encoding` | `int` | `UnicodeUtf8` | The `const char *` message argument is UTF-8 encoded. |

`Encoding` selects how the `const char *` message passed to the `const char *` constructor overload is decoded into a `QString`.

## 6. Public Member Variables

None. All data members are private (`mCode`, `mContext`, `mMessage`, `mSymbol`, `mArgs`, `mCausingErrors`) and accessed through the methods below.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

### Construction

#### LogError()

Creates an empty error. The code is set to `0` and all other members are empty. See `isEmpty()`.

#### LogError(const QString &message, int code = 0, const QString &symbol = QString(), const QString &context = QString())

Creates an error from a `QString` message with the given `code`, `symbol`, and `context`. The message is passed through an internal cleaning step that strips a single trailing `.` (period). `context` must be convertible to Latin-1; its Latin-1 form is used as the translation context for `translatedMessage()`.

#### LogError(const char *message, int code = 0, const char *symbol = nullptr, const char *context = nullptr, Encoding encoding = Latin1)

Creates an error from a C-string message decoded according to `encoding` (`symbol` and `context` are always treated as Latin-1). To support the `LOG4QT_ERROR` / `LOG4QT_QCLASS_ERROR` macros, the constructor checks whether `symbol` equals the decimal string form of `code`; if so, the symbol is cleared. The message is also cleaned of a single trailing period.

### Accessors and mutators

#### int code() const

Returns the integer error code.

#### const QString &context() const

Returns the translation context.

#### const QString &message() const

Returns the (cleaned) message template.

#### const QString &symbol() const

Returns the symbolic name for the error code.

#### void setCode(int code)

Sets the integer error code.

#### void setContext(const QString &context)

Sets the translation context. Must be Latin-1 convertible.

#### void setMessage(const QString &message)

Sets the message template; the supplied string is cleaned of a single trailing period before storage.

#### void setSymbol(const QString &symbol)

Sets the symbolic name.

### Message rendering and translation

#### QString translatedMessage() const

Returns the translated message template via `QCoreApplication::translate(context().toLatin1(), message().toUtf8(), nullptr)`. No arguments are substituted.

#### QString messageWithArgs() const

Returns the (untranslated) message with all arguments substituted in order using `QString::arg()`.

#### QString translatedMessageWithArgs() const

Returns the translated message with all arguments substituted using `QString::arg()`.

#### QString toString() const

Returns a single-line string representation. The format is `message (context::symbol, code): causing_error, causing_error`, where empty members are omitted: the context is dropped if empty, the symbol if empty, the `::` separator if either is empty, the code if `0`, the surrounding parentheses if context/symbol/code are all empty, and the trailing `: ...` if there are no causing errors. Causing errors are rendered recursively, comma-separated.

### Arguments

#### LogError &addArg(const QVariant &arg)

Appends an argument and returns `*this` (chainable). Overloads exist for `int` and `const QString &`.

#### LogError &operator<<(const QVariant &arg)

Equivalent to `addArg(arg)`; returns `*this`. Overloads exist for `int` and `const QString &`, enabling the `e << a << b` streaming idiom.

#### const QList<QVariant> &args() const

Returns the list of arguments added so far.

#### void clearArgs()

Removes all arguments.

### Causing errors

#### LogError &addCausingError(const LogError &logError)

Appends a causing error and returns `*this` (chainable).

#### const QList<LogError> &causingErrors() const

Returns the list of causing errors.

#### void clearCausingErrors()

Removes all causing errors.

### State

#### bool isEmpty() const

Returns `true` when the code is `0` **and** the message is empty.

### Per-thread last error (static)

#### static LogError lastError()

Returns the last error stored for the current thread via `setLastError()`, or a default-constructed (empty) `LogError` if none has been set. Thread-safe — each thread has its own slot.

#### static void setLastError(const LogError &logError)

Stores `logError` as the current thread's last error. Thread-safe.

### Related (non-member) stream operators

#### QDataStream &operator<<(QDataStream &stream, const LogError &logError)

Serializes the error (with a leading `quint16` version tag of `0`, followed by code, context, message, symbol, args, and causing errors). Available unless `QT_NO_DATASTREAM` is defined.

#### QDataStream &operator>>(QDataStream &stream, LogError &logError)

Deserializes an error written by the matching `operator<<`. Available unless `QT_NO_DATASTREAM` is defined.

## 10. Protected Virtual Methods

None. `LogError` has no virtual methods and is not designed for inheritance.

## 11. Ownership and Lifecycle

`LogError` is a plain value type with value semantics: copying an error deep-copies its arguments and the entire chain of causing errors (each held by value in a `QList`). There is no manual memory management and no parent/child ownership. The per-thread last-error slot is backed by `QThreadStorage<LogError *>`; the stored pointer's lifetime is managed by `QThreadStorage`, which deletes it on thread exit.

## 12. Thread Safety

Individual `LogError` instances are not synchronized — sharing one mutable instance across threads requires external locking, as with most Qt value types. The static `lastError()` / `setLastError()` pair **is** thread-safe: each thread reads and writes its own independent slot through `QThreadStorage`, so concurrent calls from different threads do not interfere.

## 13. QML Exposure

Not registered with QML.

## 14. Inter-Class Interactions

`LogError` is the common currency of Log4Qt's error path. Helpers such as `OptionConverter` and `PatternFormatter` build a `LogError` via the `LOG4QT_ERROR` macro, stream arguments into it with `operator<<`, and pass it to `Logger::error()` / `Logger::warn()` / `Logger::trace()` (which accept a `LogError` directly). Causing errors let lower-level failures (for example a `QFile` error) be nested inside a higher-level application error so the full chain is preserved in a single log entry.

## 15. External Communication

None directly. `LogError` does not perform I/O. The `QDataStream` operators allow an error to be marshaled to or from any stream the caller provides (file, socket, byte array), but the class itself opens no resources.

## 16. Usage Example

```cpp
using namespace Log4Qt;

// Inside a non-QObject class:
if (!c.isDigit())
{
    LogError e = LOG4QT_ERROR(
        QT_TR_NOOP("Found character '%1' where digit was expected."),
        LayoutExpectedDigitError,
        "Log4Qt::PatternFormatter");
    e << QString(c);
    logger()->error(e);
}

// Inside a QObject-derived class, nesting a causing error:
if (!file.open(mode))
{
    LogError e = LOG4QT_QCLASS_ERROR(
        QT_TR_NOOP("Unable to open file '%1' for appender '%2'"),
        AppenderOpeningFileError);
    e << file.fileName() << name();
    e.addCausingError(LogError(file.errorString(), file.error()));
    logger()->error(e);
    LogError::setLastError(e);   // remember it for this thread
}

// Render for a log vs. a UI:
const QString forLog = e.messageWithArgs();
const QString forUi  = e.translatedMessageWithArgs();
```
