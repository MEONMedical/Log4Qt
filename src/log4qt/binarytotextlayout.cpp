#include "binarytotextlayout.h"
#include "binaryloggingevent.h"
#include "simplelayout.h"

#include <QDebug>
#include <QStringBuilder>

namespace Log4Qt
{

BinaryToTextLayout::BinaryToTextLayout(LayoutSharedPtr subLayout, QObject *pParent)
    : Layout(pParent)
    , mSubLayout(subLayout)
{
}

QString BinaryToTextLayout::format(const LoggingEvent &rEvent)
{
    if (mSubLayout)
    {
        if (const BinaryLoggingEvent *binaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&rEvent))
        {
            QString hexData = binaryEvent->binaryMessage().toHex();
            QString spacedHexData;

            for (int i = 0; i < hexData.length(); i += 2)
                spacedHexData.append(hexData.mid(i, 2) % " ");

            // replace binary marker in output with hexdump
            return mSubLayout->format(rEvent).replace(binaryEvent->binaryMarker(), QString("%1 bytes: %2").arg(binaryEvent->binaryMessage().size()).arg(spacedHexData));
        }
    }
    return QString();
}

#ifndef QT_NO_DEBUG_STREAM
QDebug BinaryToTextLayout::debug(QDebug &rDebug) const
{
    rDebug.nospace() << "BinaryToTextLayout("
                     << "name:" << name() << " ";

    if (mSubLayout)
        rDebug.nospace() << "sublayout: " << *mSubLayout.data() << " ";
    else
        rDebug.nospace() << "sublayout: <null> ";

    rDebug.nospace() << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM

} // namespace Log4Qt
