#ifndef LOG4QT_BINARYEVENTFILTER_H
#define LOG4QT_BINARYEVENTFILTER_H

#include "spi/filter.h"

namespace Log4Qt
{

class LOG4QT_EXPORT BinaryEventFilter : public Filter
{
    Q_OBJECT
    Q_PROPERTY(bool acceptBinaryEvents READ acceptBinaryEvents WRITE setAcceptBinaryEvents)
public:
    explicit BinaryEventFilter(QObject *parent = 0);

    bool acceptBinaryEvents() const;
    void setAcceptBinaryEvents(bool accept);

    virtual Decision decide(const LoggingEvent &rEvent) const Q_DECL_OVERRIDE;

protected:
#ifndef QT_NO_DEBUG_STREAM
    virtual QDebug debug(QDebug &rDebug) const Q_DECL_OVERRIDE;
#endif // QT_NO_DEBUG_STREAM

private:
    bool mAcceptBinaryEvents;
};


inline bool BinaryEventFilter::acceptBinaryEvents() const
{
    return mAcceptBinaryEvents;
}

inline void BinaryEventFilter::setAcceptBinaryEvents(bool accept)
{
    mAcceptBinaryEvents = accept;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYEVENTFILTER_H
