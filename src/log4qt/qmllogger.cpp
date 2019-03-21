#include "qmllogger.h"

#include "logger.h"
#include "level.h"

#include <QStringBuilder>
#include <QTimer>

namespace Log4Qt
{

QmlLogger::QmlLogger(QObject *parent) :
    QObject(parent)
    , mContext(QStringLiteral("Qml"))
    , mLogger(nullptr)
{
}

void QmlLogger::trace(const QString &message) const
{
    logger()->trace(message);
}

void QmlLogger::debug(const QString &message) const
{
    logger()->debug(message);
}

void QmlLogger::info(const QString &message) const
{
    logger()->info(message);
}

void QmlLogger::error(const QString &message) const
{
    logger()->error(message);
}

void QmlLogger::fatal(const QString &message) const
{
    logger()->fatal(message);
}

void QmlLogger::log(Level level, const QString &message) const
{
    Log4Qt::Level loglevel(static_cast<Log4Qt::Level::Value>(level));
    logger()->log(loglevel, message);
}

QString QmlLogger::name() const
{
    return mName;
}

void QmlLogger::setName(const QString &name)
{
    if (mName != name)
    {
        mName = name;
        emit nameChanged(name);
    }
}

QString QmlLogger::context() const
{
    return mContext;
}

QmlLogger::Level QmlLogger::level() const
{
    return static_cast<QmlLogger::Level>(mLogger->level().toInt());
}

void QmlLogger::setContext(const QString &context)
{
    if (mContext != context)
    {
        mContext = context;
        emit contextChanged(context);
    }
}

void QmlLogger::setLevel(QmlLogger::Level level)
{
    if (this->level() != level)
    {
        mLogger->setLevel(Log4Qt::Level(static_cast<Log4Qt::Level::Value>(level)));
        emit levelChanged(level);
    }
}

QString QmlLogger::loggename() const
{
    if (mName.isEmpty() && (parent() != nullptr))
        mName = parent()->objectName();

    if (!mContext.isEmpty())
        return mContext % "." % mName;
    return mName;
}

Logger *QmlLogger::logger() const
{
    if (mLogger == nullptr)
        mLogger = Log4Qt::Logger::logger(loggename());

    return  mLogger;
}

}

#include "moc_qmllogger.cpp"
