/******************************************************************************
 *
 * package:     Log4Qt
 * file:        debugappender.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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

#include "varia/debugappender.h"

#include <QtCore/QDebug>
#include <iostream>

#include "layout.h"
#include "loggingevent.h"

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif


namespace Log4Qt
{

DebugAppender::DebugAppender(Layout *pLayout,
                             QObject *pParent) :
    AppenderSkeleton(pParent)
{
    setLayout(pLayout);
}


bool DebugAppender::requiresLayout() const
{
    return true;
}


void DebugAppender::append(const LoggingEvent &rEvent)
{
    // Q_ASSERT_X(, "DebugAppender::append()", "Lock must be held by caller");
    Q_ASSERT_X(layout(), "DebugAppender::append()", "Layout must not be null");

    QString message(layout()->format(rEvent));
#if defined(Q_OS_WIN32)
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QT_WA(
    {
        OutputDebugStringW(reinterpret_cast<const WCHAR*>(message.utf16()));
    },
    {
        OutputDebugStringA(message.toLocal8Bit().data());
    });
#else
    OutputDebugStringW(reinterpret_cast<const WCHAR*>(message.utf16()));
#endif
#else
    std::cerr << message.toLocal8Bit().constData() << std::endl;
    std::cerr << std::flush;
#endif
}

#ifndef QT_NO_DEBUG_STREAM
QDebug DebugAppender::debug(QDebug &rDebug) const
{
    QString layout_name;
    if (layout())
        layout_name = layout()->name();

    rDebug.nospace() << "DebugAppender("
                     << "name:" << name() << " "
                     << "filter:" << firstFilter() << " "
                     << "isactive:" << isActive() << " "
                     << "isclosed:" << isClosed() << " "
                     << "layout:" << layout_name << " "
                     << "referencecount:" << referenceCount() << " "
                     << "threshold:" << threshold().toString()
                     << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM


} // namspace Log4Qt
