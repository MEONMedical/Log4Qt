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

#include "layout.h"
#include "loggingevent.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include <iostream>

namespace Log4Qt
{

DebugAppender::DebugAppender(const LayoutSharedPtr &layout,
                             QObject *parent) :
    AppenderSkeleton(true, layout, parent)
{
}

bool DebugAppender::requiresLayout() const
{
    return true;
}

void DebugAppender::append(const LoggingEvent &event)
{
    Q_ASSERT_X(layout(), "DebugAppender::append()", "Layout must not be null");

    QString message(layout()->format(event));
#if defined(Q_OS_WIN)
    OutputDebugStringW(message.toStdWString().c_str());
#else
    std::cerr << message.toLocal8Bit().constData() << std::endl;
    std::cerr << std::flush;
#endif
}

} // namspace Log4Qt

#include "moc_debugappender.cpp"
