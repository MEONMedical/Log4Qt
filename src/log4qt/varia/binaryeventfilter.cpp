#include "varia/binaryeventfilter.h"

#include "binaryloggingevent.h"

namespace Log4Qt
{

BinaryEventFilter::BinaryEventFilter(QObject *parent)
    : Filter(parent)
    , mAcceptBinaryEvents{true}
{
}

Filter::Decision BinaryEventFilter::decide(const LoggingEvent &event) const
{
    bool isBinaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&event) != nullptr;

    if (!isBinaryEvent)
        return Filter::NEUTRAL;

    if (mAcceptBinaryEvents)
        return Filter::ACCEPT;
    return Filter::DENY;
}

} // namespace Log4Qt

#include "moc_binaryeventfilter.cpp"
