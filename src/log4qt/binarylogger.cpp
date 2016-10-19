#include "binarylogger.h"
#include "binaryloggingevent.h"
#include "helpers/classlogger.h"
#include "helpers/datetime.h"

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

} // namespace Log4Qt

#include "moc_binarylogger.cpp"

