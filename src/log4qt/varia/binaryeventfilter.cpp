#include "varia/binaryeventfilter.h"

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
    bool isBinaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&rEvent) != Q_NULLPTR;

    if (!isBinaryEvent)
        return Filter::NEUTRAL;

    if (mAcceptBinaryEvents)
        return Filter::ACCEPT;
    else
        return Filter::DENY;
}


} // namespace Log4Qt
