#ifndef LOG4QT_QMLLOGGER_H
#define LOG4QT_QMLLOGGER_H

#include "log4qtshared.h"
#include "logger.h"
#include "level.h"

#include <QObject>

namespace Log4Qt
{

/*!
 * \brief A  qml wapper on top of the log4qt logger for passing logger calls from qml to Log4Qt's logger.
 *
 * Resister qml wrapper:
 * \code qmlRegisterType<Log4Qt::QmlLogger>("org.log4qt", 1, 0, "Logger"); \endcode
 *
 * Use in QML:
 * \code
 * property Logger logger: Logger {}
 *
 * Component.onCompleted:  {
 *  logger.trace("Component completed.")
 * }
 * \endcode
 * \note The default logger name is the parents object name. The logger name can also be set:
 *       property Logger logger: Logger { name: "MyLogger" }
 *
 * \note The default logger context is Qml. The Logger name is put together from context and name (context.name)
 */
class LOG4QT_EXPORT QmlLogger : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QmlLogger)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString context READ context WRITE setContext NOTIFY contextChanged)
    Q_PROPERTY(Level level READ level WRITE setLevel NOTIFY levelChanged)

public:
    enum Level
    {
        Null = Log4Qt::Level::NULL_INT,
        All = Log4Qt::Level::ALL_INT,
        Trace = Log4Qt::Level::TRACE_INT,
        Debug = Log4Qt::Level::DEBUG_INT,
        Info  = Log4Qt::Level::INFO_INT,
        Warn  = Log4Qt::Level::WARN_INT,
        Error = Log4Qt::Level::ERROR_INT,
        Fatal = Log4Qt::Level::FATAL_INT,
        Off = Log4Qt::Level::OFF_INT
    };
    Q_ENUM(Level)

    explicit QmlLogger(QObject *parent = nullptr);

    // String might be enough as in JavaScript logs concatenation is natural anyway
    Q_INVOKABLE void trace(const QString &message) const;
    Q_INVOKABLE void debug(const QString &message) const;
    Q_INVOKABLE void info(const QString &message) const;
    Q_INVOKABLE void error(const QString &message) const;
    Q_INVOKABLE void fatal(const QString &message) const;
    Q_INVOKABLE void log(Level level, const QString &message) const;

    QString name() const;
    QString context() const;
    Level level() const;

public slots:
    void setName(const QString &name);
    void setContext(const QString &context);
    void setLevel(Level level);

signals:
    void nameChanged(const QString &name);
    void contextChanged(const QString &context);
    void levelChanged(Level level);

private:
    QString mContext;
    mutable QString mName;
    mutable QPointer<Logger> mLogger;

    QString loggename() const;
    Logger *logger() const;
};

}


#endif // LOG4QT_QMLLOGGER_H
