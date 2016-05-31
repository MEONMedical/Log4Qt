#include "appender.h"

namespace Log4Qt
{

Appender::Appender(QObject *pParent) :
    QObject(pParent)
{
}

Appender::~Appender()
{
}

Logger *Appender::logger() const
{
    return mLog4QtClassLogger.logger(this);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const Appender &rAppender)
{
    return rAppender.debug(debug);
}
#endif // QT_NO_DEBUG_STREAM


} // namespace Log4Qt

