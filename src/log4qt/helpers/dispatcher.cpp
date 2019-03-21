/******************************************************************************
 *
 * package:     Log4Qt
 * file:        dispatcher.cpp
 * created:     February 2011
 * author:      Andreas Bacher
 *
 *
 * Copyright 2011 Andreas Bacher
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

#include "helpers/dispatcher.h"
#include "loggingevent.h"
#include "asyncappender.h"

#include <QCoreApplication>

namespace Log4Qt
{

Dispatcher::Dispatcher(QObject *parent) : QObject(parent)
    , mAsyncAppender(nullptr)
{}

void Dispatcher::customEvent(QEvent *event)
{
    if (event->type() == LoggingEvent::eventId)
    {
        auto *logEvent = static_cast<LoggingEvent *>(event);
        if (mAsyncAppender != nullptr)
            mAsyncAppender->callAppenders(*logEvent);
    }
    QObject::customEvent(event);
}

void Dispatcher::setAsyncAppender(AsyncAppender *asyncAppender)
{
    mAsyncAppender = asyncAppender;
}

} // namespace Log4Qt

#include "moc_dispatcher.cpp"
