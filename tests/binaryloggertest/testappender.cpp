#include "testappender.h"
#include <QMutexLocker>

#include "layout.h"

#include <QDebug>

TestAppender::TestAppender(QObject *pParent)
    : Log4Qt::AppenderSkeleton(pParent)
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

void TestAppender::append(const Log4Qt::LoggingEvent &rEvent)
{
    Q_ASSERT_X(layout(), "TestAppender::append()", "Layout must not be null");
    mMessages << layout()->format(rEvent);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug TestAppender::debug(QDebug &rDebug) const
{
    rDebug.nospace() << "TestAppender("
                     << "name:" << name() << " "
                     << "count:" <<  list().count() << " "
                     << "filter:" << firstFilter() << " "
                     << "isactive:" << isActive() << " "
                     << "isclosed:" << isClosed() << " "
                     << "referencecount:" << referenceCount() << " "
                     << "threshold:" << threshold().toString()
                     << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM

#include "moc_testappender.cpp"
