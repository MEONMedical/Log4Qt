# log4qtshared.h

## A. Overview

`log4qtshared.h` defines the single symbol-visibility macro, `LOG4QT_EXPORT`, that the entire public API of Log4Qt is annotated with. Its job is to resolve, at compile time, whether each public class or function should be **exported** from the shared library, **imported** by a consumer, or left **unadorned** for a static build.

A developer reaches for this header (usually transitively, through `log4qt.h` or any public class header) whenever they declare a class or function that must be part of the library's public ABI. Annotating a type with `LOG4QT_EXPORT` ensures it is visible across the shared-library boundary on every supported platform (notably providing `__declspec(dllexport/dllimport)` on Windows via Qt's wrappers).

The behaviour is driven by two preprocessor flags set by the build system:

- `LOG4QT_STATIC` — defined when consuming or building the static library; disables all decoration.
- `LOG4QT_LIBRARY` — defined only while building the shared library itself; selects export rather than import.

The project's CMake configures these automatically: `LOG4QT_STATIC` is added as a `PUBLIC` definition when `BUILD_SHARED_LIBS` is off, and `LOG4QT_LIBRARY` is added as a `PRIVATE` definition while building the shared target.

## B. Namespaces

None. This header operates entirely at the preprocessor level and introduces no namespace.

## C. Types and Type Aliases

None.

## D. Constants

None. The header defines only the function-like-context macro below; its expansion depends on build flags rather than being a fixed value.

## E. Functions / Macros

#### LOG4QT_EXPORT

A symbol-visibility decorator placed before public class and function declarations. It resolves to one of three forms depending on the active build flags:

| Active flag | Expansion | When |
|-------------|-----------|------|
| `LOG4QT_STATIC` defined | (empty) | Building or linking against the static library; no decoration needed. |
| `LOG4QT_LIBRARY` defined (and not static) | `Q_DECL_EXPORT` | Compiling the Log4Qt shared library itself — symbols are exported. |
| neither defined | `Q_DECL_IMPORT` | Compiling a consumer of the shared library — symbols are imported. |

When to use: annotate every type or free function that forms part of the library's public ABI. Omitting it on a shared build leaves the symbol non-exported and unusable by consumers; placing it on an internal-only type needlessly enlarges the export table.

Preconditions: the build system must define `LOG4QT_STATIC` and/or `LOG4QT_LIBRARY` consistently between building and linking. A static consumer that forgets `LOG4QT_STATIC` will attempt to import symbols that were never exported.

Thread-safety: not applicable (preprocessor only).

## F. Dependencies

| Include | Provides |
|---------|----------|
| `<QtGlobal>` | `Q_DECL_EXPORT` and `Q_DECL_IMPORT`, Qt's portable wrappers over the platform's symbol-visibility attributes. |

## G. Usage Example

```cpp
#include <log4qt/log4qtshared.h>

// Public class participating in the library ABI.
class LOG4QT_EXPORT MyAppender
{
public:
    void append();
};

// Public free function.
LOG4QT_EXPORT void configureDefaults();
```
