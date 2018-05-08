#include "appender.h"

namespace Log4Qt
{

Appender::Appender(QObject *parent) :
    QObject(parent)
{
}

Logger *Appender::logger() const
{
    return mLog4QtClassLogger.logger(this);
}

} // namespace Log4Qt

#include "moc_appender.cpp"
