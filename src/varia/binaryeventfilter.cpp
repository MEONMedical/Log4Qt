#include "binaryeventfilter.h"

#include <QtCore/QDebug>
#include "binaryloggingevent.h"

namespace Log4Qt
{

BinaryEventFilter::BinaryEventFilter(QObject *parent)
    : Filter(parent)
    , mAcceptBinaryEvents{true}
{
}

Filter::Decision BinaryEventFilter::decide(const LoggingEvent &rEvent) const
{
    bool isBinaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&rEvent) != nullptr;

    if (!isBinaryEvent)
        return Filter::NEUTRAL;

    if (mAcceptBinaryEvents)
        return Filter::ACCEPT;
    else
        return Filter::DENY;
}


#ifndef QT_NO_DEBUG_STREAM
QDebug BinaryEventFilter::debug(QDebug &rDebug) const
{
    rDebug.nospace() << "BinaryEventFilter("
        << "referencecount:" << referenceCount() << " "
        << "acceptbinaryevents:" << mAcceptBinaryEvents << " "
        << "next:" << next()
        << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM

} // namespace Log4Qt
