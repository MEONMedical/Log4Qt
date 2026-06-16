# log4qtdefs.h

## A. Overview

`log4qtdefs.h` is a small, foundational definitions header. It establishes the project's convention for writing `QString` literals and pulls in the Qt string-literal facilities so that the rest of the library can construct `QString` objects cheaply and consistently.

A developer reaches for this header (usually transitively, since most internal source files include it) when they need the `LOG4QT_LITERAL` macro to build a compile-time `QString` from a string constant without an allocating runtime conversion from `const char *`.

The header brings `Qt::StringLiterals` into scope at file level, which makes the `_s` (and related) user-defined literal suffixes available wherever the header is included.

> Note: the convenience *logging* macros that declare and cache a `Logger` — `LOG4QT_DECLARE_STATIC_LOGGER` and `LOG4QT_DECLARE_QCLASS_LOGGER` — are **not** defined in this header. They live in `logger.h`. This header is concerned only with the string-literal helper.

## B. Namespaces

| Namespace | Usage |
|-----------|-------|
| `Qt::StringLiterals` | Brought into scope with a `using namespace` directive so the string-literal suffix operators (e.g. `_s`) are available to any translation unit that includes this header. |

## C. Types and Type Aliases

None.

## D. Constants

None.

## E. Functions / Macros

#### LOG4QT_LITERAL(str)

Converts a plain string literal into a `QStringLiteral`-style Qt string literal. It expands to `u##str##_s` — that is, it prefixes the argument with `u` to form a UTF-16 (`char16_t`) literal and appends the `_s` user-defined-literal suffix from `Qt::StringLiterals`, yielding a `QString` constructed from statically stored UTF-16 data.

What it does: produces a `QString` from a compile-time string with no heap allocation or runtime transcoding from Latin-1/UTF-8, equivalent in spirit to `QStringLiteral` but written through the modern `_s` literal operator.

When to use: anywhere a `QString` is needed from a fixed string constant — keys, format fragments, fixed messages — to avoid the cost of constructing a `QString` from a `const char *` at runtime.

Preconditions: the argument must be a bare string literal (e.g. `LOG4QT_LITERAL("name")`), because token pasting requires the `u`, the literal, and `_s` to be adjacent tokens. It cannot be applied to a `const char *` variable. The `Qt::StringLiterals` namespace must be in scope, which this header guarantees.

Thread-safety: not applicable (preprocessor only); the resulting `QString` references immutable static data and is safe to read concurrently.

## F. Dependencies

| Include | Provides |
|---------|----------|
| `<QtGlobal>` | Core Qt typedefs and configuration macros. |
| `<QtClassHelperMacros>` | Qt class-helper macros (e.g. disabling copy/move), available to includers of this header. |
| `<QString>` | The `QString` type and the `Qt::StringLiterals` namespace whose `_s` operator backs `LOG4QT_LITERAL`. |

## G. Usage Example

```cpp
#include <log4qt/log4qtdefs.h>

void configure()
{
    // Allocation-free QString from a compile-time literal.
    const QString key = LOG4QT_LITERAL("rootLogger.level");

    // The _s suffix is also directly available via Qt::StringLiterals.
    const QString value = u"DEBUG"_s;
}
```
