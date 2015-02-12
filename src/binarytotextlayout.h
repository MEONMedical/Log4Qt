#ifndef LOG4QT_BINARYTOTEXTLAYOUT_H
#define LOG4QT_BINARYTOTEXTLAYOUT_H

#include "layout.h"

namespace Log4Qt
{

//! Used to format binary messages for a textual appenders
class LOG4QT_EXPORT BinaryToTextLayout : public Layout
{
    Q_OBJECT
    Q_PROPERTY(Layout * subLayout READ subLayout WRITE setSubLayout)
public:
    explicit BinaryToTextLayout(Layout *subLayout = 0, QObject *parent = 0);

    virtual QString format(const LoggingEvent &rEvent);

    Layout *subLayout() const {return mSubLayout;}
    void setSubLayout(Layout *layout) {mSubLayout = layout;}

protected:

#ifndef QT_NO_DEBUG_STREAM
        virtual QDebug debug(QDebug &rDebug) const;
#endif // QT_NO_DEBUG_STREAM

private:
    Q_DISABLE_COPY(BinaryToTextLayout)
    Layout *mSubLayout;
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYTOTEXTLAYOUT_H
