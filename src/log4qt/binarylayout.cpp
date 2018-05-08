#include "binarylayout.h"
#include "binaryloggingevent.h"

namespace Log4Qt
{

BinaryLayout::BinaryLayout(QObject *parent)
    : Layout(parent)
{
}

QByteArray BinaryLayout::binaryFormat(const BinaryLoggingEvent &event) const
{
    return event.binaryMessage();
}

QString BinaryLayout::format(const LoggingEvent &event)
{
    Q_UNUSED(event)
    return QString{};
}

QString BinaryLayout::contentType() const
{
    return QStringLiteral("application/octet-stream");
}

} // namespace Log4Qt

#include "moc_binarylayout.cpp"

