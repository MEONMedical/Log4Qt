#ifndef LOG4QT_XMLLAYOUT_H
#define LOG4QT_XMLLAYOUT_H

#include "layout.h"

namespace Log4Qt
{

class  LOG4QT_EXPORT XMLLayout : public Layout
{
    Q_OBJECT
public:
    explicit XMLLayout(QObject *pParent = nullptr);
    virtual ~XMLLayout();

    virtual QString format(const LoggingEvent &rEvent);

protected:
#ifndef QT_NO_DEBUG_STREAM
    virtual QDebug debug(QDebug &rDebug) const;
#endif // QT_NO_DEBUG_STREAM

private:
    Q_DISABLE_COPY(XMLLayout)
};

}

#endif // LOG4QT_XMLLAYOUT_H

