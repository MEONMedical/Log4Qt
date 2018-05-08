#include "testappender.h"
#include <QMutexLocker>

#include "log4qt/layout.h"

TestAppender::TestAppender(QObject *parent)
    : Log4Qt::AppenderSkeleton(parent)
{
}

QStringList TestAppender::list() const
{
    QMutexLocker locker(&mObjectGuard);
    return mMessages;
}

QStringList TestAppender::clearList()
{
    QMutexLocker locker(&mObjectGuard);
    QStringList copy(mMessages);
    mMessages.clear();
    return copy;
}

bool TestAppender::requiresLayout() const
{
    return true;
}

void TestAppender::append(const Log4Qt::LoggingEvent &event)
{
    Q_ASSERT_X(layout(), "TestAppender::append()", "Layout must not be null");
    mMessages << layout()->format(event);
}

#include "moc_testappender.cpp"
