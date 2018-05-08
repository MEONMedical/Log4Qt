#ifndef LOG4QT_BINARYEVENTFILTER_H
#define LOG4QT_BINARYEVENTFILTER_H

#include <log4qt/spi/filter.h>

namespace Log4Qt
{

class LOG4QT_EXPORT BinaryEventFilter : public Filter
{
    Q_OBJECT
    Q_PROPERTY(bool acceptBinaryEvents READ acceptBinaryEvents WRITE setAcceptBinaryEvents)
public:
    explicit BinaryEventFilter(QObject *parent = nullptr);

    bool acceptBinaryEvents() const;
    void setAcceptBinaryEvents(bool accept);

    Decision decide(const LoggingEvent &event) const override;

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
