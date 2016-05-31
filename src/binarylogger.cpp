#include "binarylogger.h"
#include "binaryloggingevent.h"
#include "helpers/classlogger.h"
#include "helpers/datetime.h"

#include <QDebug>

namespace Log4Qt
{

BinaryLogger::BinaryLogger(LoggerRepository *pLoggerRepository, Level level, const QString &rName, Logger *pParent)
    : Logger(pLoggerRepository, level, rName, pParent)
{
}

BinaryLogger::~BinaryLogger()
{
}

void BinaryLogger::forcedLog(Level level, const QByteArray &rMessage) const
{
    BinaryLoggingEvent event(this, level, rMessage);
    callAppenders(event);
}

BinaryLogStream BinaryLogger::log(Level level) const
{
    return BinaryLogStream(this, level);
}

void BinaryLogger::log(Level level, const QByteArray &rMessage) const
{
    if (isEnabledFor(level))
        forcedLog(level, rMessage);
}

void BinaryLogger::log(Level level, const QByteArray &rMessage, const QDateTime &timeStamp) const
{
    if (isEnabledFor(level))
    {
        BinaryLoggingEvent event(this, level, rMessage, timeStamp.toMSecsSinceEpoch());
        callAppenders(event);
    }
}

#ifndef QT_NO_DEBUG_STREAM
QDebug BinaryLogger::debug(QDebug &rDebug) const
{
    QReadLocker locker(&mAppenderGuard);

    QString parent_logger;
    if (Logger *parent = parentLogger())
        parent_logger = parent->name();

    rDebug.nospace() << "BinaryLogger("
                     << "name:" << name() << " "
                     << "appenders:" << mAppenders.count() << " "
                     << "additivity:" << additivity() << " "
                     << level() << "parentLogger:" << parent_logger
                     << ")";
    return rDebug.space();
}

QDebug operator<<(QDebug debug, const BinaryLogger &rLogger)
{
    return rLogger.debug(debug);
}

#endif // QT_NO_DEBUG_STREAM

} // namespace Log4Qt

#include "moc_binarylogger.cpp"

