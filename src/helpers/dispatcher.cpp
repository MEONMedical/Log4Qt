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

#include "dispatcher.h"
#include "loggingevent.h"
#include "asyncappender.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

namespace Log4Qt
{

/**************************************************************************
 * Class implementation: Dispatcher
 **************************************************************************/
Dispatcher::Dispatcher(QObject *parent) : QObject(parent)
            , mpAsyncAppender(0)
{}

void Dispatcher::customEvent(QEvent* event)
{
    if (event->type() == LoggingEvent::eventId)
    {
        LoggingEvent *logEvent = static_cast<LoggingEvent*>(event);
        qDebug() << "Dispatcher::customEvent()" << QString("0x%1").arg((quintptr)(QThread::currentThread()), QT_POINTER_SIZE * 2, 16, QChar('0'));
        if (mpAsyncAppender)
            mpAsyncAppender->callAppenders(*logEvent);
    }
    QObject::customEvent(event);
}

void Dispatcher::setAsyncAppender(AsyncAppender *pAsyncAppender)
{
    mpAsyncAppender = pAsyncAppender;
}


/**************************************************************************
 * Class implementation: DispatcherThread
 **************************************************************************/
DispatcherThread::DispatcherThread(QObject *parent) :
    QThread(parent)
    , mpDispatcher(0)
{
}

DispatcherThread::~DispatcherThread()
{
    delete mpDispatcher;
}

void DispatcherThread::postLoggingEvent(const LoggingEvent &rEvent) const
{
    // Post to the event loop of the dispatcher thread
    LoggingEvent *event = new LoggingEvent(rEvent);
    qApp->postEvent(mpDispatcher, event);
}

void DispatcherThread::setAsyncAppender(AsyncAppender *pAsyncAppender)
{
    mpDispatcher->setAsyncAppender(pAsyncAppender);
}

void DispatcherThread::run()
{
    mpDispatcher = new Dispatcher();
    exec();
}

} // namespace Log4Qt
