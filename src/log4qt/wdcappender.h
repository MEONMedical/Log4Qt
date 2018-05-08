#ifndef LOG4QT_WDCAPPENDER_H
#define LOG4QT_WDCAPPENDER_H

#include "appenderskeleton.h"

namespace Log4Qt
{

class LOG4QT_EXPORT WDCAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    WDCAppender(QObject *parent = nullptr);
    WDCAppender(const LayoutSharedPtr &layout,
                QObject *parent = nullptr);

    virtual bool requiresLayout() const override;

protected:
    virtual void append(const LoggingEvent &event) override;

private:
    Q_DISABLE_COPY(WDCAppender)
};

} // namespace Log4Qt

#endif //  LOG4QT_WDCAPPENDER_H
