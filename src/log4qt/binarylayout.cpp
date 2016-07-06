#include "binarylayout.h"
#include "binaryloggingevent.h"

#include <QDebug>

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

#ifndef QT_NO_DEBUG_STREAM
QDebug BinaryLayout::debug(QDebug &rDebug) const
{
    rDebug.nospace() << "BinaryLayout("
                     << "name:" << name() << " "
                     << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM

} // namespace Log4Qt

#include "moc_binarylayout.cpp"

