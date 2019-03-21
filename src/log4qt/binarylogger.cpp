#include "binarylogger.h"
#include "binaryloggingevent.h"
#include "helpers/classlogger.h"
#include "helpers/datetime.h"

namespace Log4Qt
{

BinaryLogger::BinaryLogger(LoggerRepository *loggerRepository, Level level, const QString &name, Logger *parent)
    : Logger(loggerRepository, level, name, parent)
{
}

BinaryLogger::~BinaryLogger() = default;

void BinaryLogger::forcedLog(Level level, const QByteArray &message) const
{
    BinaryLoggingEvent event(this, level, message);
    callAppenders(event);
}

BinaryLogStream BinaryLogger::log(Level level) const
{
    return BinaryLogStream(this, level);
}

void BinaryLogger::log(Level level, const QByteArray &message) const
{
    if (isEnabledFor(level))
        forcedLog(level, message);
}

void BinaryLogger::log(Level level, const QByteArray &message, const QDateTime &timeStamp) const
{
    if (isEnabledFor(level))
    {
        BinaryLoggingEvent event(this, level, message, timeStamp.toMSecsSinceEpoch());
        callAppenders(event);
    }
}

} // namespace Log4Qt

#include "moc_binarylogger.cpp"

