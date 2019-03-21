#include "binarytotextlayout.h"
#include "binaryloggingevent.h"
#include "simplelayout.h"

#include <QStringBuilder>

namespace Log4Qt
{

BinaryToTextLayout::BinaryToTextLayout(const LayoutSharedPtr &subLayout, QObject *parent)
    : Layout(parent)
    , mSubLayout(subLayout)
{
}

QString BinaryToTextLayout::format(const LoggingEvent &event)
{
    if (!mSubLayout.isNull())
    {
        if (const auto *binaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&event))
        {
            QString hexData = binaryEvent->binaryMessage().toHex();
            QString spacedHexData;

            for (int i = 0; i < hexData.length(); i += 2)
                spacedHexData.append(hexData.mid(i, 2) % " ");

            // replace binary marker in output with hexdump
            return mSubLayout->format(event).replace(Log4Qt::BinaryLoggingEvent::binaryMarker(), QStringLiteral("%1 bytes: %2").arg(binaryEvent->binaryMessage().size()).arg(spacedHexData));
        }
    }
    return QString();
}

} // namespace Log4Qt

#include "moc_binarytotextlayout.cpp"
