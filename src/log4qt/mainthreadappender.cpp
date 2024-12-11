/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#include "mainthreadappender.h"
#include "loggingevent.h"
#include "helpers/dispatcher.h"

#include <QCoreApplication>
#include <QReadLocker>
#include <QThread>

#if (__cplusplus >= 201703L) // C++17 or later
#include <utility>
#endif

namespace Log4Qt
{

MainThreadAppender::MainThreadAppender(QObject *parent) : AppenderSkeleton(parent)
{}

bool MainThreadAppender::requiresLayout() const
{
    return false;
}

void MainThreadAppender::activateOptions()
{
}

void MainThreadAppender::append(const LoggingEvent &event)
{
    QReadLocker locker(&mAppenderGuard);

#if (__cplusplus >= 201703L)
    for (auto &&pAppender : std::as_const(mAppenders))
#else
    for (auto &&pAppender : qAsConst(mAppenders))
#endif
    {
        if (QThread::currentThread() != qApp->thread())
            qApp->postEvent(pAppender.data(), new LoggingEvent(event));
        else
            pAppender->doAppend(event);
    }
}

bool MainThreadAppender::checkEntryConditions() const
{
    return AppenderSkeleton::checkEntryConditions();
}

} // namespace Log4Qt

#include "moc_mainthreadappender.cpp"

