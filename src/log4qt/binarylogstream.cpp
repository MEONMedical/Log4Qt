#include "binarylogstream.h"
#include "binarylogger.h"
#include "binaryloggingevent.h"

#include <QByteArray>
#include <QPointer>

namespace Log4Qt
{

struct BinaryLogStream::Stream
{
    Stream(const Logger *logger, Level level)
        : logger(logger)
        , level(level)
    {
    }
    ~Stream();

    QByteArray buffer;
    QPointer<const Logger> logger;
    Level level;
};

BinaryLogStream::Stream::~Stream()
{
    if (auto blogger = qobject_cast<const BinaryLogger *>(logger))
        blogger->log(level, buffer);
    else if (logger->isEnabledFor(level))
    {
        BinaryLoggingEvent event(logger, level, buffer);
        logger->callAppenders(event);
    }
}

BinaryLogStream::BinaryLogStream(const Logger *logger, Level level)
    : mStream(new Stream(logger, level))
{
}

BinaryLogStream &BinaryLogStream::operator<<(const QByteArray &data)
{
    mStream->buffer.append(data);
    return *this;
}

} // namespace Log4Qt
