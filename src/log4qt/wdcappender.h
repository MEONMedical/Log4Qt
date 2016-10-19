#ifndef LOG4QT_WDCAPPENDER_H
#define LOG4QT_WDCAPPENDER_H

#include "appenderskeleton.h"

namespace Log4Qt
{

class LOG4QT_EXPORT WDCAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    WDCAppender(QObject *pParent = Q_NULLPTR);
    WDCAppender(LayoutSharedPtr pLayout,
                QObject *pParent = Q_NULLPTR);
private:
    Q_DISABLE_COPY(WDCAppender)
public:
    virtual bool requiresLayout() const Q_DECL_OVERRIDE;
protected:
    virtual void append(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;

};

} // namespace Log4Qt

#endif //  LOG4QT_WDCAPPENDER_H
