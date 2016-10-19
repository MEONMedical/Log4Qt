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

} // namespace Log4Qt

