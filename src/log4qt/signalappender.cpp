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

#include "signalappender.h"

#include "abstractlayout.h"

namespace Log4Qt
{

SignalAppender::SignalAppender(QObject *parent) :
    AppenderSkeleton(parent)
{
}

void SignalAppender::preAppend(const LoggingEvent &event, const LayoutSharedPtr &layout)
{
    // Runs outside mObjectGuard. Use the layout snapshot passed by doAppend
    // rather than layout() so we neither re-acquire the lock nor race a
    // concurrent setLayout(). Emitting here means a DirectConnection slot
    // executes without the appender lock held.
    if (!layout)
        return;
    Q_EMIT appended(layout->format(event));
}

void SignalAppender::append(const LoggingEvent & /*event*/)
{
    // Intentionally empty: SignalAppender emits from preAppend() (outside the
    // lock). See the header for the rationale.
}

}

#include "moc_signalappender.cpp"
