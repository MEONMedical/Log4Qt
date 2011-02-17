/******************************************************************************
 *
 * package:     Log4Qt
 * file:        mainthreadappender.cpp
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

/******************************************************************************
 * Dependencies
 ******************************************************************************/

#include "mainthreadappender.h"
#include "loggingevent.h"
#include "helpers/dispatcher.h"

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QReadLocker>

namespace Log4Qt
{

/**************************************************************************
 * Declarations
 **************************************************************************/


/**************************************************************************
 * C helper functions
 **************************************************************************/


/**************************************************************************
 * Class implementation: MainThreadAppender
 **************************************************************************/


MainThreadAppender::MainThreadAppender(QObject *parent) : AppenderSkeleton(parent)
{}

MainThreadAppender::~MainThreadAppender()
{
    close();
}

bool MainThreadAppender::requiresLayout() const
{
    return false;
}

void MainThreadAppender::activateOptions()
{
}

void MainThreadAppender::close()
{
}

void MainThreadAppender::append(const LoggingEvent &rEvent)
{
    QReadLocker locker(&mAppenderGuard);

    Appender *pAppender;
    Q_FOREACH(pAppender, mAppenders)
    {
        if (QThread::currentThread() != qApp->thread())
        {
            LoggingEvent *event = new LoggingEvent(rEvent);
            qApp->postEvent(pAppender, event);

        }
        else
            pAppender->doAppend(rEvent);
    }
}

bool MainThreadAppender::checkEntryConditions() const
{
    return AppenderSkeleton::checkEntryConditions();
}

#ifndef QT_NO_DEBUG_STREAM
/*!
 * Writes all object member variables to the given debug stream
 * \a rDebug and returns the stream.
 *
 * <tt>
 * %MainThreadAppender(name:"WA" )
 * </tt>
 * \sa QDebug, operator<<(QDebug debug, const LogObject &rLogObject	)
 */
QDebug MainThreadAppender::debug(QDebug &rDebug) const
{
    rDebug.nospace() << "MainThreadAppender("
        << "name:" << name() << " "
        << ")";

    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM


/**************************************************************************
 * Implementation: Operators, Helper
 **************************************************************************/

} // namespace Log4Qt

