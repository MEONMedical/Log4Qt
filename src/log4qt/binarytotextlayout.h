#ifndef LOG4QT_BINARYTOTEXTLAYOUT_H
#define LOG4QT_BINARYTOTEXTLAYOUT_H

#include "layout.h"

namespace Log4Qt
{

//! Used to format binary messages for a textual appenders
class LOG4QT_EXPORT BinaryToTextLayout : public Layout
{
    Q_OBJECT
    Q_PROPERTY(LayoutSharedPtr subLayout READ subLayout WRITE setSubLayout)
public:
    explicit BinaryToTextLayout(LayoutSharedPtr subLayout = LayoutSharedPtr(), QObject *parent = Q_NULLPTR);

    virtual QString format(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;

    LayoutSharedPtr subLayout() const {return mSubLayout;}
    void setSubLayout(LayoutSharedPtr layout) {mSubLayout = layout;}

protected:
#ifndef QT_NO_DEBUG_STREAM
    virtual QDebug debug(QDebug &rDebug) const Q_DECL_OVERRIDE;
#endif // QT_NO_DEBUG_STREAM

private:
    Q_DISABLE_COPY(BinaryToTextLayout)
    LayoutSharedPtr mSubLayout;
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYTOTEXTLAYOUT_H
