#include "binaryloggingevent.h"
#include "logger.h"
#include "helpers/datetime.h"

#include <QDebug>

#ifndef QT_NO_DATASTREAM
#include <QDataStream>
#endif

namespace Log4Qt
{

static const char binMarker[] = "@@@ binary message @@@";

BinaryLoggingEvent::BinaryLoggingEvent()
    : LoggingEvent()
{
}

BinaryLoggingEvent::BinaryLoggingEvent(const Logger *pLogger, Level level, const QByteArray &bMessage)
    : LoggingEvent(pLogger, level, QString(binMarker))
    , mBinaryMessage(bMessage)
{
}

BinaryLoggingEvent::BinaryLoggingEvent(const Logger *pLogger, Level level, const QByteArray &bMessage, qint64 timeStamp)
    : LoggingEvent(pLogger, level, QString(binMarker), timeStamp)
    , mBinaryMessage(bMessage)
{
}

BinaryLoggingEvent::BinaryLoggingEvent(const Logger *pLogger, Level level, const QByteArray &bMessage, const QString &rNdc, const QHash<QString, QString> &rProperties, const QString &rThreadName, qint64 timeStamp)
    : LoggingEvent(pLogger, level, QString(binMarker), rNdc, rProperties, rThreadName, timeStamp)
    , mBinaryMessage(bMessage)
{
}

QByteArray BinaryLoggingEvent::binaryMessage() const
{
    return mBinaryMessage;
}

QString BinaryLoggingEvent::toString() const
{
    return level().toString() + QLatin1Char(':') + mBinaryMessage.toHex();
}

QString BinaryLoggingEvent::binaryMarker()
{
    return binMarker;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &rStream, const BinaryLoggingEvent &rLoggingEvent)
{
    rStream << static_cast<const LoggingEvent &>(rLoggingEvent);
    rStream << rLoggingEvent.mBinaryMessage;
    return rStream;
}

QDataStream &operator>>(QDataStream &rStream, BinaryLoggingEvent &rLoggingEvent)
{
    rStream >> static_cast<LoggingEvent &>(rLoggingEvent);
    rStream >> rLoggingEvent.mBinaryMessage;
    return rStream;
}
#endif // QT_NO_DATASTREAM


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const BinaryLoggingEvent &rLoggingEvent)
{
    QString logger;
    if (rLoggingEvent.logger() != Q_NULLPTR)
        logger = rLoggingEvent.logger()->name();

    debug.nospace() << "BinaryLoggingEvent("
                    << "level:" << rLoggingEvent.level().toString() << " "
                    << "logger:" << logger << " "
                    << "message:" << rLoggingEvent.binaryMessage().toHex() << " "
                    << "sequencenumber:" << rLoggingEvent.sequenceNumber() << " "
                    << "threadname:" << rLoggingEvent.threadName() << " "
                    << "timestamp:" << rLoggingEvent.timeStamp()
                    << "(" << DateTime::fromMSecsSinceEpoch(rLoggingEvent.timeStamp()) << ")"
                    << "sequenceCount:" << rLoggingEvent.sequenceCount()
                    << ")";
    return debug.space();
}
#endif // QT_NO_DEBUG_STREAM

} // namespace Log4Qt
