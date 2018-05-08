#include "wdcappender.h"

#include "layout.h"
#include "loggingevent.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
static void OutputDebugString(const wchar_t *lpOutputString)
{
    Q_UNUSED(lpOutputString)
}
#endif

namespace Log4Qt
{

WDCAppender::WDCAppender(QObject *parent)
    : AppenderSkeleton(false, parent)
{
}

WDCAppender::WDCAppender(const LayoutSharedPtr &layout, QObject *parent)
    : AppenderSkeleton(false, layout, parent)
{
}

bool WDCAppender::requiresLayout() const
{
    return true;
}

void WDCAppender::append(const LoggingEvent &event)
{
    Q_ASSERT_X(layout(), "WDCAppender::append()", "Layout must not be null");

    QString message(layout()->format(event));

    OutputDebugString(message.toStdWString().c_str());
}

}

#include "moc_wdcappender.cpp"
