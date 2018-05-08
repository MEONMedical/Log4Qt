#ifndef LOG4QT_BINARYLOGGINGEVENT_H
#define LOG4QT_BINARYLOGGINGEVENT_H

#include "loggingevent.h"

namespace Log4Qt
{

class LOG4QT_EXPORT BinaryLoggingEvent : public LoggingEvent
{
public:
    BinaryLoggingEvent();
    BinaryLoggingEvent(const Logger *logger,
                       Level level,
                       const QByteArray &message);
    BinaryLoggingEvent(const Logger *logger,
                       Level level,
                       const QByteArray &message,
                       qint64 timeStamp);
    BinaryLoggingEvent(const Logger *logger,
                       Level level,
                       const QByteArray &message,
                       const QString &ndc,
                       const QHash<QString, QString> &properties,
                       const QString &threadName,
                       qint64 timeStamp);
    QByteArray binaryMessage() const;

    QString toString() const;

    static QString binaryMarker();

private:
    QByteArray mBinaryMessage;

#ifndef QT_NO_DATASTREAM
    // Needs to be friend to stream objects
    friend LOG4QT_EXPORT QDataStream &operator<<(QDataStream &out, const BinaryLoggingEvent &loggingEvent);
    friend LOG4QT_EXPORT QDataStream &operator>>(QDataStream &in, BinaryLoggingEvent &loggingEvent);
#endif // QT_NO_DATASTREAM
};

#ifndef QT_NO_DATASTREAM
LOG4QT_EXPORT QDataStream &operator<<(QDataStream &out, const BinaryLoggingEvent &loggingEvent);
LOG4QT_EXPORT QDataStream &operator>>(QDataStream &in, BinaryLoggingEvent &loggingEvent);
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt

Q_DECLARE_METATYPE(Log4Qt::BinaryLoggingEvent)
Q_DECLARE_TYPEINFO(Log4Qt::BinaryLoggingEvent, Q_MOVABLE_TYPE);

#endif // LOG4QT_BINARYLOGGINGEVENT_H
