#include "binarylayout.h"
#include "binaryloggingevent.h"

namespace Log4Qt
{

BinaryLayout::BinaryLayout(QObject *parent)
    : Layout(parent)
{
}

QByteArray BinaryLayout::binaryFormat(const BinaryLoggingEvent &rEvent)
{
    return rEvent.binaryMessage();
}

QString BinaryLayout::format(const LoggingEvent &)
{
    return QString{};
}

QString BinaryLayout::contentType() const
{
    return "application/octet-stream";
}

} // namespace Log4Qt

#include "moc_binarylayout.cpp"

