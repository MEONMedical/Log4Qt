# DatabaseLayout

## 1. Class Overview

Log4Qt is a Qt/C++ port of Apache log4j; *layouts* turn a `LoggingEvent` into the representation an appender writes. `DatabaseLayout` is unusual among the layouts: instead of producing a string for a text destination, its primary product is a `QSqlRecord` whose fields map event data onto named database columns. It is used together with the database appender to insert each log event as a row in an SQL table.

Reach for `DatabaseLayout` when log records should be stored in a relational database. You configure the column name for each event field (timestamp, logger, thread, level, message); fields whose column name is left empty are omitted from the record. This class is compiled only when database logging support is enabled (`BUILD_WITH_DB_LOGGING` / `LOG4QT_DB_LOGGING_SUPPORT`).

## 2. Project Structure and Dependencies

- **Instantiated by**: The database appender (compiled alongside it under `BUILD_WITH_DB_LOGGING`), and application code/configurators that map fields to columns.
- **Qt modules**: Qt Core and **Qt Sql** (`QSqlRecord`, `QSqlField`, `QMetaType`). The build links `Qt::Sql` `PUBLIC` (the installed headers include QtSql headers), and only when database logging is enabled.
- **Internal types**: `AbstractLayout` (base — note: *not* `AbstractStringLayout`), `LoggingEvent`, `Level`, and `DateTime` (`helpers/datetime.h`) for epoch-to-`QDateTime` conversion.

## 3. Class Hierarchy and Role

`DatabaseLayout` → `AbstractLayout` → `QObject`. It derives directly from `AbstractLayout` (not the text-oriented `AbstractStringLayout`) because its main output is a `QSqlRecord` rather than encoded bytes. It inherits the meta-object system, the layout contract, and the header/footer provider chain. It implements the required `format()` (returning a debug-style string) and adds the SQL-specific `formatRecord()`. Copy and move are disabled.

## 4. Q_PROPERTY Declarations

| Property | Type | READ | WRITE | NOTIFY | Description |
|----------|------|------|-------|--------|-------------|
| `timeStampColumn` | `QString` | `timeStampColumn` | `setTimeStampColumn` | — | Column name for the event timestamp (stored as `QDateTime`). Empty means the field is omitted. |
| `loggerNameColumn` | `QString` | `loggerNameColumn` | `setLoggernameColumn` | — | Column name for the logger name (`QString`). Empty means omitted. Note the spelling asymmetry: the READ accessor is `loggerNameColumn` (capital N) while the WRITE accessor is `setLoggernameColumn` (lower-case n). |
| `threadNameColumn` | `QString` | `threadNameColumn` | `setThreadNameColumn` | — | Column name for the thread name (`QString`). Empty means omitted. |
| `levelColumn` | `QString` | `levelColumn` | `setLevelColumn` | — | Column name for the level name (`QString`). Empty means omitted. |
| `messageColumn` | `QString` | `messageColumn` | `setMessageColumn` | — | Column name for the message text (`QString`). Empty means omitted. |

## 5. Enumerations

None.

## 6. Public Member Variables

None (all column-name members private behind properties).

## 7. Signals

None.

## 8. Public Slots and Q_INVOKABLE Methods

None.

## 9. Public Methods

#### DatabaseLayout(QObject *parent = nullptr)
Constructs the layout with all column names empty (so no fields are emitted until configured). Defined inline.

#### QSqlRecord formatRecord(const LoggingEvent &event) [virtual]
The primary output method. Builds and returns a `QSqlRecord` containing one `QSqlField` per configured (non-empty) column: timestamp as `QMetaType::QDateTime` (converted via `DateTime::fromMSecsSinceEpoch`), and logger name, thread name, level string, and message as `QMetaType::QString`. Each field is marked generated. Virtual, so subclasses can customise the column-to-value mapping.

#### QString format(const LoggingEvent &event) [override]
Satisfies the `AbstractLayout` contract by returning a flat `name:value; …` text rendering of the configured fields (timestamp formatted as `dd.MM.yyyy hh:mm`, then thread, level, logger, message). Primarily diagnostic; the database appender uses `formatRecord()` for actual insertion.

#### QString timeStampColumn() const
Returns the configured timestamp column name.

#### QString loggerNameColumn() const
Returns the configured logger-name column name.

#### QString threadNameColumn() const
Returns the configured thread-name column name.

#### QString levelColumn() const
Returns the configured level column name.

#### QString messageColumn() const
Returns the configured message column name.

#### void setTimeStampColumn(const QString &columnName)
Sets the timestamp column name (empty omits the field).

#### void setLoggernameColumn(const QString &columnName)
Sets the logger-name column name (empty omits the field).

#### void setThreadNameColumn(const QString &columnName)
Sets the thread-name column name (empty omits the field).

#### void setLevelColumn(const QString &columnName)
Sets the level column name (empty omits the field).

#### void setMessageColumn(const QString &columnName)
Sets the message column name (empty omits the field).

## 10. Protected Virtual Methods / Event Handlers

No `protected` members. The overridable surface is the public `virtual formatRecord()` (the intended customisation point for subclasses) and the inherited `format()` override.

## 11. Ownership and Lifecycle

A `QObject` accepting an optional `QObject *parent`; parent-owned when given, otherwise managed through `LayoutSharedPtr`. No owned heap resources; `formatRecord()` and `format()` build values on the stack. The destructor uses the compiler default. Copy/move disabled.

## 12. Thread Safety

Single-threaded by convention. Both `formatRecord()` and `format()` read only the immutable event and the column-name members, with no shared mutable state, so they are effectively reentrant; concurrent use is mediated by the owning appender's lock.

## 13. QML Exposure

Not registered for QML.

## 14. Inter-Class Interactions

- The database appender calls `formatRecord()` to obtain the `QSqlRecord` it inserts.
- Reads timestamp, logger name, thread name, level, and message from the `LoggingEvent`; converts timestamps via `DateTime`.
- Configurators set the five column-name properties through the meta-object system.
- Inherits the header/footer provider chain from `AbstractLayout`.

## 15. External Communication

`DatabaseLayout` does not open a connection itself; it produces a `QSqlRecord`. The paired database appender uses that record to execute an INSERT against a `QSqlDatabase`, so the effective external channel (a relational database via Qt Sql) is owned by the appender, not by this layout.

## 16. Usage Example

```cpp
#include "log4qt/databaselayout.h"

using namespace Log4Qt;

auto *layout = new DatabaseLayout();
layout->setTimeStampColumn(u"ts"_s);
layout->setLevelColumn(u"level"_s);
layout->setLoggernameColumn(u"logger"_s);
layout->setMessageColumn(u"msg"_s);
layout->activateOptions();

// The paired database appender inserts formatRecord(event) into its table.
QSqlRecord record = layout->formatRecord(event);
```
