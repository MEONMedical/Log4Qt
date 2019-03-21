#include "binaryloggingevent.h"
#include "logger.h"
#include "helpers/datetime.h"

#ifndef QT_NO_DATASTREAM
#include <QDataStream>
#endif

namespace Log4Qt
{

static const char binMarker[] = "@@@ binary message @@@";

BinaryLoggingEvent::BinaryLoggingEvent() = default;

BinaryLoggingEvent::BinaryLoggingEvent(const Logger *logger, Level level, const QByteArray &message)
    : LoggingEvent(logger, level, QString(binMarker))
    , mBinaryMessage(message)
{
}

BinaryLoggingEvent::BinaryLoggingEvent(const Logger *logger, Level level, const QByteArray &message, qint64 timeStamp)
    : LoggingEvent(logger, level, QString(binMarker), timeStamp)
    , mBinaryMessage(message)
{
}

BinaryLoggingEvent::BinaryLoggingEvent(const Logger *logger, Level level, const QByteArray &message, const QString &ndc, const QHash<QString, QString> &properties, const QString &threadName, qint64 timeStamp)
    : LoggingEvent(logger, level, QString(binMarker), ndc, properties, threadName, timeStamp)
    , mBinaryMessage(message)
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
QDataStream &operator<<(QDataStream &out, const BinaryLoggingEvent &loggingEvent)
{
    out << static_cast<const LoggingEvent &>(loggingEvent);
    out << loggingEvent.mBinaryMessage;
    return out;
}

QDataStream &operator>>(QDataStream &in, BinaryLoggingEvent &loggingEvent)
{
    in >> static_cast<LoggingEvent &>(loggingEvent);
    in >> loggingEvent.mBinaryMessage;
    return in;
}
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt
