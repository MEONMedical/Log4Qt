#ifndef TESTAPPENDER_H
#define TESTAPPENDER_H

#include "log4qt/appenderskeleton.h"

#include <QStringList>

class  TestAppender : public Log4Qt::AppenderSkeleton
{
    Q_OBJECT
public:
    explicit TestAppender(QObject *pParent = nullptr);
    TestAppender(const TestAppender &) = delete;
    TestAppender &operator=(const TestAppender &) = delete;

    QStringList list() const;
    QStringList clearList();
    virtual bool requiresLayout() const override;

protected:
    virtual void append(const Log4Qt::LoggingEvent &rEvent) override;

private:
    QStringList mMessages;
};

#endif // TESTAPPENDER_H
