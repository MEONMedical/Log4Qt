#ifndef TESTAPPENDER_H
#define TESTAPPENDER_H

#include "appenderskeleton.h"

#include <QStringList>
class QDebug;

class  TestAppender : public Log4Qt::AppenderSkeleton
{
    Q_OBJECT
public:
    explicit TestAppender(QObject *pParent = nullptr);
    TestAppender(const TestAppender &) = delete;
    TestAppender &operator=(const TestAppender &) = delete;

    QStringList list() const;
    QStringList clearList();
    virtual bool requiresLayout() const;

protected:
    virtual void append(const Log4Qt::LoggingEvent &rEvent);

#ifndef QT_NO_DEBUG_STREAM
    virtual QDebug debug(QDebug &rDebug) const;
#endif // QT_NO_DEBUG_STREAM

private:
    QStringList mMessages;
};

#endif // TESTAPPENDER_H
