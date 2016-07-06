#include "wdcappender.h"

#include <QDebug>
#include "layout.h"
#include "loggingevent.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
static void OutputDebugString(const wchar_t *)
{
}
#endif

namespace Log4Qt
{

WDCAppender::WDCAppender(QObject *pParent)
    : AppenderSkeleton(false, pParent)
{
}

WDCAppender::WDCAppender(LayoutSharedPtr pLayout, QObject *pParent)
    : AppenderSkeleton(false, pParent)
{
    setLayout(pLayout);
}

bool WDCAppender::requiresLayout() const
{
    return true;
}

void WDCAppender::append(const LoggingEvent &rEvent)
{
    Q_ASSERT_X(layout(), "WDCAppender::append()", "Layout must not be null");

    QString message(layout()->format(rEvent));

    OutputDebugString(message.toStdWString().c_str());
}

#ifndef QT_NO_DEBUG_STREAM
QDebug WDCAppender::debug(QDebug &rDebug) const
{
    QString layout_name;
    if (layout())
        layout_name = layout()->name();

    rDebug.nospace() << "WDCAppender("
                     << "name:" << name() << " "
                     << "isactive:" << isActive()
                     << "isclosed:" << isClosed()
                     << "layout:" << layout_name
                     << "threshold:" << threshold().toString()
                     << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM

}
