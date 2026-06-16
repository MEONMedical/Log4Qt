# AppenderSkeleton

## 1. Class Overview

`AppenderSkeleton` is the concrete base class of the Log4Qt appender hierarchy. It implements the general machinery shared by every appender so that subclasses only have to provide the actual output step (`append()`). Specifically it manages:

- the attached **layout**,
- the **filter chain** (head/tail linked list of `Filter` objects),
- the **threshold** `Level` below which events are discarded,
- the **active / closed** lifecycle state, and
- the complete `doAppend()` lifecycle, including a five-phase locking strategy and a thread-local recursion guard.

A developer writing a new appender almost always derives from `AppenderSkeleton` (directly, or via `WriterAppender`) and implements `append()`, optionally overriding `checkEntryConditions()`, `activateOptions()`, and `preAppend()`.

## 2. Project Structure and Dependencies

Declared in `appenderskeleton.h`; implemented in `appenderskeleton.cpp`. It is the direct base of `WriterAppender`, `AsyncAppender`, `SignalAppender`, the `varia` appenders (`ListAppender`, `NullAppender`, etc.), and others.

Build requirement: **Qt Core** (`QObject`, `QMutex`/`QRecursiveMutex`, `QEvent`).

Project-internal types:

- **`Appender`** (`appender.h`) — the abstract base whose pure virtuals this class implements.
- **`AbstractLayout` / `LayoutSharedPtr`** — the formatter.
- **`Filter` / `FilterSharedPtr`** (`spi/filter.h`) — filter-chain links.
- **`Level`** (`level.h`) — the severity type used by the threshold; the threshold is stored in a `std::atomic<Level>`.
- **`LoggingEvent`** (`loggingevent.h`) — the event being appended; also carries the custom `QEvent` id used by `customEvent()`.
- **`Logger`** (`logger.h`) — used for internal error reporting via the inherited `logger()`.

Standard library: `<atomic>` for the lock-free state flags and threshold.

## 3. Class Hierarchy and Role

`QObject` → `Appender` → **`AppenderSkeleton`**.

From `Appender` it inherits the abstract contract and the `layout`/`name`/`requiresLayout` properties; it implements `filter()`, `layout()`, `name()`, `setLayout()`, `setName()`, `addFilter()`, `clearFilters()`, `close()`, and `doAppend()`. It leaves `requiresLayout()` and the new pure virtual `append()` for subclasses. Copy/move are disabled via `Q_DISABLE_COPY_MOVE`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `isActive` | `bool` | `isActive` | — | — | Read-only. `true` once the appender has been activated (after `activateOptions()` succeeds) and not yet closed. Events are dropped while inactive. |
| `isClosed` | `bool` | `isClosed` | — | — | Read-only. `true` after `close()` has been called; the appender rejects all further events. |
| `threshold` | `Log4Qt::Level` | `threshold` | `setThreshold` | — | The minimum severity an event must have to be appended. Events less severe than the threshold are silently dropped. Defaults to `Level::NULL_INT` (no threshold filtering). |

## 5. Public Member Variables

None public. One protected data member is documented in the Protected section (`mObjectGuard`).

## 6. Public Methods

#### explicit AppenderSkeleton(QObject *parent = nullptr)

Constructs an appender that starts **active** and not closed, with threshold `Level::NULL_INT`. (Subclasses that must be configured before use typically prefer the protected `isActive = false` constructor.)

#### FilterSharedPtr filter() const [override]

Returns the head of the filter chain (or null). Acquires `mObjectGuard`.

#### LayoutSharedPtr layout() const [override]

Returns the attached layout (or null). Acquires `mObjectGuard`.

#### bool isActive() const

Lock-free atomic read of the active flag (relaxed ordering). Inline.

#### bool isClosed() const

Lock-free atomic read of the closed flag (relaxed ordering). Inline.

#### QString name() const [override]

Returns the appender's name (backed by `objectName()`). Acquires `mObjectGuard`.

#### Level threshold() const

Returns the current threshold level. Inline atomic read.

#### void setLayout(const LayoutSharedPtr &layout) [override]

Replaces the attached layout under `mObjectGuard`.

#### void setName(const QString &name) [override]

Sets the appender's name (forwarded to `setObjectName`) under `mObjectGuard`.

#### void setThreshold(Level level)

Sets the threshold level. Inline atomic store.

#### virtual void activateOptions()

Validates configuration and marks the appender active. The base implementation checks that, if `requiresLayout()` is `true`, a layout has been set; if not, it logs an `AppenderActivateMissingLayoutError` and leaves the appender inactive. Subclasses override this to open their resources first and then call the base. Acquires `mObjectGuard`.

#### void addFilter(const FilterSharedPtr &filter) [override]

Appends `filter` to the tail of the filter chain. A null filter is ignored (a warning is logged). Acquires `mObjectGuard`.

#### void clearFilters() [override]

Removes all filters by resetting the head pointer. Acquires `mObjectGuard`.

#### void close() [override]

Marks the appender closed and inactive. Idempotent. Acquires `mObjectGuard` (via `closeInternal()`).

#### void doAppend(const LoggingEvent &event) [override]

The core append lifecycle. See Section 10 for the full five-phase description. This is the method loggers call for every event routed to this appender.

#### FilterSharedPtr firstFilter() const

Returns the head filter; identical to `filter()` but spelled out explicitly under the lock. Inline, acquires `mObjectGuard`.

#### bool isAsSevereAsThreshold(Level level) const

Returns `true` if `level` is at least as severe as the configured threshold (`threshold <= level`). Used internally by `doAppend()` to apply the threshold. Inline.

## 7. Protected Methods

These constructors and methods exist for subclassing.

#### explicit AppenderSkeleton(bool isActive, QObject *parent = nullptr)

Constructs the appender with an explicit initial active state. Subclasses that require activation (e.g. `WriterAppender`) pass `false` so the appender stays inactive until `activateOptions()` succeeds.

#### explicit AppenderSkeleton(bool isActive, const LayoutSharedPtr &layout, QObject *parent = nullptr)

As above, but also attaches an initial layout.

#### virtual void append(const LoggingEvent &event) = 0

**Pure virtual.** The actual output step, called from Phase 5 of `doAppend()` while `mObjectGuard` is held. Subclasses write the formatted event to their destination here. Because it runs under the lock, I/O is serialised across threads.

#### void customEvent(QEvent *event) [override]

Overrides `QObject::customEvent`. When the event's type matches `LoggingEvent::eventId`, it casts to `LoggingEvent` and forwards it to `doAppend()`; otherwise it defers to `QObject::customEvent`. This lets logging events be delivered asynchronously through Qt's event queue (used by the cross-thread/async appenders).

#### virtual bool checkEntryConditions() const

Tests the conditions required before `append()` may run: the appender is active (`AppenderNotActivatedError`), not closed (`AppenderClosedError`), and has a layout if it requires one (`AppenderUseMissingLayoutError`). On failure it logs the error and returns `false`. Subclasses override to add their own checks (e.g. "writer set", "file open") and then call the base via `AppenderSkeleton::checkEntryConditions()` to chain the checks. Called by `doAppend()` under the lock.

#### virtual void preAppend(const LoggingEvent &event, const LayoutSharedPtr &layout)

Optional hook called in Phase 4b of `doAppend()` — **outside** `mObjectGuard`, after entry checks and the filter chain have passed. It receives a `QSharedPointer` snapshot of the layout that stays valid for the call even if the layout is replaced concurrently. Subclasses (e.g. `RandomAccessFileAppender`) use it to perform expensive, read-only preparation (typically layout formatting) into thread-local storage while other threads run their own `preAppend()` in parallel. Contract: must be stateless with respect to shared appender data, store results in thread-local storage, and must not call `doAppend()` (the recursion guard would drop the nested call). The default implementation is a no-op.

#### static void forwardEvent(const AppenderSharedPtr &appender, const LoggingEvent &event)

Forwards `event` to `appender->doAppend()` while temporarily resetting the thread-local recursion depth, so the call is **not** dropped by the recursion guard. Used for intentional event redirection (e.g. routing an overflow event to an error appender), not for internally generated log messages. All normal `doAppend()` checks still run on the target appender.

#### const LayoutSharedPtr &layoutSnapshot() const

Lock-free read of the live layout pointer. **Callable only while `mObjectGuard` is held by the caller** — i.e. from within `append()` (Phase 5). Returns a reference to avoid the shared-pointer refcount round-trip of `layout()`. Subclass `append()` implementations use this to access the layout cheaply.

## 8. Protected Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `mObjectGuard` | `mutable QRecursiveMutex` | The recursive mutex that serialises all configuration changes and the actual `append()` I/O. Protected so subclasses can lock it in their own `activateOptions()` / `close()` overrides. |

## 9. Append Lifecycle and Protected Virtual Methods

`doAppend()` (defined here, overriding `Appender::doAppend`) executes in five phases. Understanding them is essential for subclassing:

- **Phase 1 — Recursion guard.** A `thread_local int s_appendDepth` is checked: if it is already greater than zero the call returns immediately. This prevents infinite loops when an appender logs an internal error through a logger that routes back to an appender on the same thread. The depth is incremented on entry and decremented on scope exit via `qScopeGuard`. (`forwardEvent()` is the sanctioned way to bypass this for intentional redirects.)
- **Phase 2 — Fast atomic pre-checks.** `isActive()` and `isClosed()` are read without the lock; the call returns if the appender is inactive or closed.
- **Phase 3 — Entry conditions and snapshot (under `mObjectGuard`).** `checkEntryConditions()` runs, the threshold is applied via `isAsSevereAsThreshold(event.level())`, and the head filter plus the layout are snapshotted into shared pointers (so they stay alive after the lock is released). The lock is then released.
- **Phase 4 — Filter chain (no lock).** The chain is walked: `Filter::Accept` breaks out and proceeds, `Filter::Deny` returns (event dropped), `Filter::Neutral` advances to the next filter. Because `decide()` is `const`, multiple threads may evaluate concurrently.
- **Phase 4b — `preAppend()` (no lock).** The pre-format hook runs outside the lock so heavy formatting parallelises.
- **Phase 5 — `append()` (under `mObjectGuard`).** The lock is re-acquired, `isActive()` is re-checked (close() may have run during Phases 4–4b), and the subclass `append()` performs the serialised output.

`checkEntryConditions()`, `preAppend()`, and `append()` are the three override points; subclass `checkEntryConditions()` overrides should chain to the base implementation.

## 10. Ownership and Lifecycle

An `AppenderSkeleton` is a `QObject`: with a non-null parent it is destroyed by the parent. In the framework it is held by `AppenderSharedPtr` reference counting. The layout and filters are held by `LayoutSharedPtr` / `FilterSharedPtr`, so they outlive any concurrent reconfiguration via the Phase 3 snapshot. The destructor calls `closeInternal()` to mark the appender closed; subclasses release their own resources in their destructors. There are no raw owning pointers.

## 11. Thread Safety

Fully **thread-safe**. State is split between lock-free atomics (`mIsActive`, `mIsClosed`, `mThreshold`) for cheap reads, and `mObjectGuard` (a `QRecursiveMutex`) for layout, filter chain, and the serialised `append()` step. The recursive mutex permits a subclass method already holding the lock (e.g. `activateOptions()`) to call another locking method without deadlock. The thread-local recursion guard prevents re-entrant `doAppend()` loops. The carefully staged lock acquire/release in `doAppend()` lets filter evaluation and `preAppend()` run concurrently while keeping I/O serialised.

## 12. Inter-Class Interactions

- **Loggers** call `doAppend()` for each event.
- **`Layout`** formats the event in `append()` / `preAppend()`.
- **`Filter`** chain decides accept/deny/neutral in Phase 4.
- **`Level`** drives threshold comparison.
- **`LoggingEvent`** can arrive synchronously (direct `doAppend()`) or asynchronously through `customEvent()` via Qt's event loop.
- **Internal `Logger`** (from `logger()`) reports activation/entry-condition errors.
- Configurators set `threshold`, `layout`, filters, and `name`.
