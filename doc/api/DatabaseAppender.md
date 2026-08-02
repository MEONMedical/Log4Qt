# DatabaseAppender

## 1. Class Overview

Log4Qt is a Qt port of Apache log4j. An *appender* is the sink that writes a formatted log event somewhere. `DatabaseAppender` writes each log event as a row inserted into a table of a SQL database accessed through Qt SQL.

A developer uses it to persist logs into a relational database for querying, auditing, or centralised collection. The column-to-field mapping is supplied by a companion `DatabaseLayout`, which names the table columns for timestamp, logger name, thread name, level, and message. The appender precompiles a parameterised `INSERT` statement once and re-binds values for every event, so per-event cost is just value binding plus execution. Preparation is self-healing: if it could not succeed at activation time, or if the connection is later lost, the statement is re-prepared from `append()` and the insert retried once.

## 2. Project Structure and Dependencies

- **Header includes:** `appenderskeleton.h` (base class), `<QtSql/QSqlDatabase>`, `<QtSql/QSqlQuery>`, `<memory>`, `<vector>`.
- **Implementation includes:** `databaselayout.h`, `loggingevent.h`, `helpers/datetime.h`, `<QStringBuilder>`, and the Qt SQL diagnostics headers (`QSqlDriver`, `QSqlRecord`, `QSqlField`, `QSqlError`).
- **Qt module:** Qt SQL plus Qt Core. `Qt::Sql` is linked **`PUBLIC`** (only when `BUILD_WITH_DB_LOGGING` is enabled), because the installed `databaseappender.h` / `databaselayout.h` include QtSql headers — consumers of the installed library need it on their include path too.
- **Project-internal types:**
  - `DatabaseLayout` — a `Layout` subclass that supplies the target column names (`timeStampColumn()`, `loggerNameColumn()`, `threadNameColumn()`, `levelColumn()`, `messageColumn()`). `prepareInsert()` `qobject_cast`s the configured layout to this type; if the cast fails, no statement is prepared.
  - `LoggingEvent` — source of the bound values (`timeStamp()`, `loggername()`, `threadName()`, `level()`, `message()`).
  - `DateTime` (helper) — converts the event's epoch-milliseconds timestamp into a value bindable to a SQL datetime column.

## 3. Class Hierarchy and Role

`DatabaseAppender` inherits **`AppenderSkeleton`** (→ `Appender` → `QObject`), gaining the meta-object system, parent-based ownership, the `doAppend()` entry pipeline, threshold/filter handling, and `mObjectGuard`. It is constructed with the base's "not active until activated" flag set (`AppenderSkeleton(false, ...)`). It overrides `requiresLayout()`, `activateOptions()`, `append()`, and `checkEntryConditions()`.

Its role is a SQL persistence sink driven by a `DatabaseLayout`.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `connection` | `QString` | `connection` | `setConnection` | — | Name of the `QSqlDatabase` connection to use. Defaults to `QSqlDatabase::defaultConnection`. Changing it resets the prepared statement so it is rebuilt against the new connection. |
| `table` | `QString` | `table` | `setTable` | — | Name of the destination table the `INSERT` targets. Changing it resets the prepared statement. Must be non-empty for activation to succeed. |

## 5. Enumerations

The class defines a private `enum class ColumnSource { TimeStamp, Loggername, ThreadName, Level, Message }` used internally to remember, for each bound placeholder, which event field supplies its value. It is not part of the public API.

## 6. Public Member Variables

None. State (`connectionName`, `tableName`, prepared query, bindings) is non-public.

## 7. Signals

None declared.

## 8. Public Slots and Q_INVOKABLE Methods

None declared.

## 9. Public Methods

#### DatabaseAppender(QObject *parent = nullptr)

Constructs an inactive appender using `QSqlDatabase::defaultConnection`. No layout, no table.

#### DatabaseAppender(const LayoutSharedPtr &layout, QObject *parent = nullptr)

Constructs with a layout (expected to be a `DatabaseLayout`) and the default connection.

#### DatabaseAppender(const LayoutSharedPtr &layout, const QString &tableName, const QString &connection = QSqlDatabase::defaultConnection, QObject *parent = nullptr)

Constructs fully configured: layout, target table, and connection name.

#### ~DatabaseAppender() override

Releases the prepared statement through `resetPreparedQuery()`, which routes the teardown through the activation-thread check described under Thread Safety. An explicit destructor is needed because destroying the `QSqlQuery` implicitly would tear the statement down through driver code on whatever thread happens to destroy the appender.

#### bool requiresLayout() const override

Returns `true` — a (database) layout is mandatory.

#### QString connection() const

Returns the connection name. Thread-safe (guarded by `mObjectGuard`).

#### QString table() const

Returns the destination table name. Thread-safe (guarded by `mObjectGuard`).

#### void setConnection(const QString &connection)

Sets the connection name. If it changes, the prepared statement and binding plan are discarded so they are rebuilt on the next `activateOptions()`. Guarded by `mObjectGuard`.

#### void setTable(const QString &table)

Sets the destination table. If it changes, the prepared statement and binding plan are discarded. Guarded by `mObjectGuard`.

#### void activateOptions() override

Validates that the named connection exists (`QSqlDatabase::contains`) and a non-empty table is set; logs an error (`AppenderMissingDatabaseOrTableError`) and returns if not. Otherwise calls `prepareInsert()` to build and prepare the parameterised `INSERT`, then chains to `AppenderSkeleton::activateOptions()`. Guarded by `mObjectGuard`.

## 10. Protected Virtual Methods

#### void append(const LoggingEvent &event) override

Invoked from `doAppend()` under `mObjectGuard`. In order:

1. **Late prepare.** If no prepared statement exists, `prepareInsert()` is retried here. The activation-time prepare legitimately fails in transient situations — the table did not exist yet, the database was briefly unreachable — and without this retry every subsequent event would be rejected as "unprepared query" for the rest of the process. Only if the retry also fails does it log `AppenderInvalidDatabaseLayoutError` and return.
2. **Thread check.** It verifies the calling thread is the one that prepared the statement (recorded by `prepareInsert()`): if a log call reaches `append()` from a different thread it logs once (`AppenderExecSqlQueryError`) and **drops the event** rather than touch the `QSqlQuery` cross-thread, which is undefined and can crash the SQL driver.
3. **Bind and execute.** `bindEventValues()` walks the binding plan, binding each positional placeholder to the corresponding event field (timestamp via `DateTime::fromMSecsSinceEpoch`, logger name, thread name, level string, message), then `exec()` runs the insert.
4. **Retry once on failure.** A failed `exec()` is usually a connection that dropped since preparation (server restart, network outage). Because `QSqlDatabase::database()` re-opens a closed connection, the statement is re-prepared, re-bound and executed once more. Only if that retry also fails is the error reported — with the *original* failure's query and `QSqlError` text (`AppenderExecSqlQueryError`), since that is the diagnostically useful one.

Re-preparing happens on the logging thread under the appender lock, which keeps the activation-thread guard consistent: the new query belongs to the thread that will use it.

#### bool checkEntryConditions() const override

Returns `false` (logging `AppenderMissingDatabaseOrTableError`) if the connection no longer exists or the table name is empty; otherwise chains to `AppenderSkeleton::checkEntryConditions()`.

#### void closeWriter()

Declared protected for the lifecycle of the underlying writer/statement (releases the prepared query). Subclasses overriding teardown should account for it.

## 11. Ownership and Lifecycle

- The appender is a `QObject`; a `parent` deletes it. In normal use it is held via `AppenderSharedPtr` and managed by the logger repository.
- The prepared `QSqlQuery` is owned via `std::unique_ptr` (`mPreparedQuery`) and rebuilt by `prepareInsert()` / cleared by `resetPreparedQuery()` whenever the connection or table changes, and by the destructor.
- `resetPreparedQuery()` is reachable from `setConnection()` / `setTable()` on any thread. When it runs on a thread other than the one that prepared the statement it **intentionally leaks the query handle** (with a logged warning) instead of destroying it, because `~QSqlQuery` tears the statement down through driver code — precisely the cross-thread driver use the activation-thread guard exists to prevent. Reconfiguring the connection or table from a foreign thread is a rare path, and a leaked handle is preferable to a driver crash.
- The `QSqlDatabase` connection itself is **not owned** by the appender — it is looked up by name from Qt's global connection registry. The application is responsible for opening and (eventually) removing that connection.
- The binding plan (`mBindings`) is a `std::vector<ColumnSource>` rebuilt alongside the prepared statement.

## 12. Thread Safety

All public functions are thread-safe. Configuration accessors (`connection()`, `table()`, the setters), `activateOptions()`, and `append()` all take `mObjectGuard` (a recursive mutex), so the prepared statement is only ever bound and executed by one thread at a time — important because a single `QSqlQuery`/`QSqlDatabase` connection is not safe for concurrent use. Beyond serialisation, Qt requires a database connection (and queries prepared from it) to be used only on the thread that created them. `append()` enforces this at runtime: `prepareInsert()` records the thread that prepared the statement and, if a later log call arrives on a different thread, `append()` logs once and drops the event instead of corrupting the driver. `resetPreparedQuery()` and the destructor apply the same check to statement *teardown* (leaking the handle rather than destroying it off-thread — see Ownership and Lifecycle). To log to the database from multiple threads, front this appender with a single-threaded dispatcher (for example a `MainThreadAppender` or an `AsyncAppender` bound to the database thread) so all SQL work happens on one thread — and ensure the connection was opened on that same thread (the library cannot detect a connection opened elsewhere).

## 14. Inter-Class Interactions

- Reads its column mapping from a `DatabaseLayout` obtained via `layout()` and `qobject_cast`.
- Reads the global Qt SQL connection registry by name (`QSqlDatabase::contains`, `QSqlDatabase::database`).
- Reports prepare/exec failures through the Log4Qt internal `logger()` as `LogError`s.

## 15. External Communication

`DatabaseAppender` communicates with an external relational database via Qt SQL.

- **Channel / driver:** a `QSqlDatabase` connection identified by `connection`, using whatever Qt SQL driver that connection was opened with (e.g. QSQLITE, QPSQL, QMYSQL, QODBC). The appender does not open or configure the connection — it only looks it up by name.
- **Direction:** outbound only. The appender issues `INSERT` statements; it never reads rows back.
- **Protocol / format:** a single parameterised `INSERT INTO <table> (<columns>) VALUES (?, ?, …)` statement, prepared once in `prepareInsert()`. Columns are exactly those non-empty column names returned by the `DatabaseLayout`, in fixed order (timestamp, logger name, thread name, level, message). Each event re-binds the positional parameters and executes.
- **Identifier escaping:** the table and column names are run through `QSqlDriver::escapeIdentifier()` (with `TableName` / `FieldName` respectively) before being concatenated into the statement, so quoted, mixed-case or space-containing identifiers work and configured identifiers cannot be injected into the statement text. The escaping is applied to locals only — `tableName` and the layout's column names keep their configured, unescaped form, so a re-prepare does not double-escape. If the driver cannot be obtained the raw names are used as a fallback.
- **Error handling:** preparation failures and execution failures are logged via the internal logger (with the SQL error text) and otherwise swallowed — a failing `INSERT` does not throw or propagate. Both are retried once (see `append()`), so a transient outage or a table created after startup recovers on its own. If the connection disappears or the table name is cleared, `checkEntryConditions()` blocks the append and logs an error.
- **Threading implications:** the connection must be used on a single thread (Qt SQL constraint); serialise access (e.g. behind `AsyncAppender`) when logging from multiple threads.

## 16. Usage Example

```cpp
#include "log4qt/databaseappender.h"
#include "log4qt/databaselayout.h"
#include "log4qt/logger.h"

#include <QtSql/QSqlDatabase>

using namespace Log4Qt;

// 1. Open a named connection and ensure the target table exists.
auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("logdb"));
db.setDatabaseName(QStringLiteral("logs.sqlite"));
db.open();

// 2. A DatabaseLayout maps event fields to column names.
auto layout = LayoutSharedPtr(new DatabaseLayout);
auto *dbLayout = static_cast<DatabaseLayout *>(layout.data());
dbLayout->setTimeStampColumn(QStringLiteral("ts"));
dbLayout->setLevelColumn(QStringLiteral("level"));
dbLayout->setMessageColumn(QStringLiteral("message"));
dbLayout->activateOptions();

// 3. Wire up the appender.
auto *appender = new DatabaseAppender(layout, QStringLiteral("log_events"),
                                      QStringLiteral("logdb"));
appender->setName(QStringLiteral("db"));
appender->activateOptions();           // prepares the INSERT statement

Logger::rootLogger()->addAppender(AppenderSharedPtr(appender));
Logger::rootLogger()->error(QStringLiteral("persisted to the database"));
```
