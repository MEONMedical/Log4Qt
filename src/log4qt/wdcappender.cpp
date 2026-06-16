/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#include "wdcappender.h"

#include "abstractlayout.h"
#include "loggingevent.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
static void OutputDebugString([[maybe_unused]] const wchar_t *lpOutputString)
{
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
    const LayoutSharedPtr &layoutSnap = layoutSnapshot();
    if (!layoutSnap)
        return;

    QString message(layoutSnap->format(event));

    OutputDebugString(message.toStdWString().c_str());
}

}

#include "moc_wdcappender.cpp"
