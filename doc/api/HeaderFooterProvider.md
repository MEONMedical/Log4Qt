# HeaderFooterProvider

## 1. Class Overview

`HeaderFooterProvider` is the abstract base class for objects that supply dynamic header and footer text to layouts. A layout writes its header when a log file is opened and its footer when it is closed; a provider lets the application compute those strings at that moment rather than fixing them statically.

A provider can be attached to an individual layout (`AbstractLayout::setHeaderFooterProvider()`) or registered globally as a fallback for all layouts (`AbstractLayout::setGlobalHeaderFooterProvider()`). The default `header()` / `footer()` implementations return an empty string, which carries "no content provided — fall through to the next source in the priority chain" semantics; an override that returns an empty string behaves the same way.

This file also defines the concrete `PatternHeaderFooterProvider`, which formats its header and footer using Log4Qt conversion patterns (the same specifiers as `PatternLayout`), evaluated at file-open / file-close time.

## 2. Project Structure and Dependencies

- Header: `src/log4qt/spi/headerfooterprovider.h`
- Source: `src/log4qt/spi/headerfooterprovider.cpp`

Direct dependencies:

- `QObject` — base class.
- `Log4QtSharedPtr<HeaderFooterProvider>` (`log4qtsharedptr.h`) — managed shared pointer; aliased as `HeaderFooterProviderSharedPtr`.
- `PatternFormatter` (`helpers/patternformatter.h`) — used by `PatternHeaderFooterProvider` to evaluate patterns; held via `std::unique_ptr`.
- `LoggingEvent` (`loggingevent.h`) — a default-constructed event is passed to the formatter for header/footer evaluation.

## 3. Class Hierarchy and Role

`QObject` → **`HeaderFooterProvider`** (abstract base) → `PatternHeaderFooterProvider`

`HeaderFooterProvider` is the abstract interface layouts query. `PatternHeaderFooterProvider` is the bundled concrete implementation that renders patterns. Both are non-copyable and non-movable (`Q_DISABLE_COPY_MOVE`).

## 4. Q_PROPERTY

`HeaderFooterProvider` declares no properties.

`PatternHeaderFooterProvider` properties:

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `headerPattern` | `QString` | `headerPattern()` | `setHeaderPattern()` | — | Conversion pattern for the header, evaluated at file-open time. Supports `%d` (date/time), `%r` (ms since program start), `%P{key}` (object property), and literal text. Setting an empty pattern clears the header formatter. |
| `footerPattern` | `QString` | `footerPattern()` | `setFooterPattern()` | — | Conversion pattern for the footer, symmetric to `headerPattern`; evaluated at file-close time. |

## 5. Enumerations

None.

## 6. Public Member Variables

None exposed. `PatternHeaderFooterProvider` keeps its patterns and `PatternFormatter` instances private.

## 7. Signals

None.

## 8. Public Slots & Q_INVOKABLE

None.

## 9. Public Methods

### HeaderFooterProvider

#### explicit HeaderFooterProvider(QObject *parent = nullptr)
Constructs the provider with an optional QObject parent.

#### ~HeaderFooterProvider() override
Destroys the provider. Defaulted; declared `override` (the QObject destructor is virtual).

### PatternHeaderFooterProvider

#### explicit PatternHeaderFooterProvider(QObject *parent = nullptr)
Constructs the pattern provider with empty patterns (no formatters until a pattern is set).

#### ~PatternHeaderFooterProvider() override
Destroys the provider, releasing the owned `PatternFormatter` instances.

#### QString headerPattern() const
Returns the configured header pattern. `[[nodiscard]]`.

#### void setHeaderPattern(const QString &pattern)
Stores the header pattern. If `pattern` is empty the header formatter is cleared; otherwise a new `PatternFormatter` is created with the provider itself set as the property source (so `%P{key}` resolves against this object's QObject properties).

#### QString footerPattern() const
Returns the configured footer pattern. `[[nodiscard]]`.

#### void setFooterPattern(const QString &pattern)
Stores the footer pattern with the same formatter-lifecycle and property-source behaviour as `setHeaderPattern()`.

## 10. Protected Virtual Methods

There are no `protected` virtuals; the overridable hooks are public virtuals defined on `HeaderFooterProvider`:

#### virtual QString header() const
Returns the header string. The base implementation returns an empty string ("no header provided"). `PatternHeaderFooterProvider` overrides it to format the header pattern against a default-constructed `LoggingEvent` at call time, returning empty when no header formatter is set.

#### virtual QString footer() const
Returns the footer string. The base implementation returns an empty string ("no footer provided"). `PatternHeaderFooterProvider` overrides it symmetrically for the footer pattern.

#### virtual void activateOptions()
Called from the layout's `activateOptions()` to let the provider initialise. The base implementation is a no-op; `PatternHeaderFooterProvider` does not override it (its formatters are built lazily in the pattern setters).

## 11. Ownership and Lifecycle

Providers are held through `HeaderFooterProviderSharedPtr` (reference-counted `Log4QtSharedPtr`), allowing the same provider instance to be shared between a layout-local slot and the global fallback. The classes are non-copyable and non-movable. `PatternHeaderFooterProvider` owns its two `PatternFormatter` objects via `std::unique_ptr`, recreated whenever a pattern is (re)assigned and destroyed with the provider.

## 12. Thread Safety

The base class is stateless and inherently safe. `PatternHeaderFooterProvider::header()` / `footer()` are `const` but read the formatter pointers without locking, so reconfiguring patterns concurrently with header/footer evaluation is not synchronized by the class itself; configure providers before they are activated/used. Header and footer are normally evaluated by the layout at file-open / file-close boundaries.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- `AbstractLayout` queries a provider through `header()` / `footer()` and supports both a per-layout provider (`setHeaderFooterProvider()`) and a global fallback (`setGlobalHeaderFooterProvider()`); empty returns fall through the layout's priority chain.
- `PatternHeaderFooterProvider` delegates formatting to `PatternFormatter` and sets itself as that formatter's property source, enabling `%P{key}` to read QObject properties (either declared via `Q_PROPERTY` in a subclass or set dynamically with `QObject::setProperty`).

## 15. External Communication

None directly. The provider only returns strings; any file I/O (writing the header/footer) is performed by the layout and its appender.

## 16. Usage Example

```cpp
#include "log4qt/spi/headerfooterprovider.h"
#include "log4qt/patternlayout.h"

using namespace Log4Qt;

// Dynamic property variant — no subclass required.
auto *provider = new PatternHeaderFooterProvider;
provider->setHeaderPattern(u"=== run started %d{yyyy-MM-dd HH:mm:ss} S/N: %P{serialNumber} ==="_s);
provider->setFooterPattern(u"=== run ended %d{HH:mm:ss} ==="_s);
provider->setProperty("serialNumber", u"SN-001"_s); // resolves %P{serialNumber}

// Install globally so every layout uses it as a fallback.
AbstractLayout::setGlobalHeaderFooterProvider(
    HeaderFooterProviderSharedPtr(provider));
```
