/******************************************************************************
 *
 * package:     Log4Qt
 * file:        appenderattachable.cpp
 * created:     Dezember 2010
 * author:      Andreas Bacher
 *
 *
 * Copyright 2007 Andreas Bacher
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


#include "helpers/appenderattachable.h"
#include "varia/listappender.h"
#include "appender.h"

#include <QtCore/QDebug>

namespace Log4Qt
{


/**************************************************************************
 * Declarations
 **************************************************************************/



/**************************************************************************
 * C helper functions
 **************************************************************************/



/**************************************************************************
 * Class implementation: Filter
 **************************************************************************/
AppenderAttachable::AppenderAttachable() :
#if QT_VERSION < QT_VERSION_CHECK(4, 4, 0)
        mAppenderGuard()
#else
        mAppenderGuard(QReadWriteLock::Recursive)
#endif
{
}

QList<Appender *> AppenderAttachable::appenders() const
{
    QReadLocker locker(&mAppenderGuard);

    QList<Appender *> result;
    Appender *p_appender;
    Q_FOREACH(p_appender, mAppenders)
        result << p_appender;

    return result;
}

void AppenderAttachable::addAppender(Appender *pAppender)
{
    // Avoid deadlock:
    // - Handle warnings, before write lock is aquired

    // Keep objects with a 0 reference count safe
    LogObjectPtr < Appender > p_appender = pAppender;

    {
        QReadLocker locker(&mAppenderGuard);

        if (!p_appender) {
            //logger()->warn("Adding null Appender to Logger '%1'", name());
            return;
        }

        if (mAppenders.contains(p_appender)) {
            //logger()->warn("Adding of duplicate appender '%2' to logger '%1'",
                //name(), p_appender->name());
            return;
        }
    }
    {
        QWriteLocker locker(&mAppenderGuard);

        if (mAppenders.contains(p_appender))
            return;
        mAppenders.append(p_appender);
    }
}

Appender *AppenderAttachable::appender(const QString &rName) const
{
    QReadLocker locker(&mAppenderGuard);

    Appender *p_appender;
    Q_FOREACH(p_appender, mAppenders)
        if (p_appender->name() == rName)
            return p_appender;
    return 0;
}

bool AppenderAttachable::isAttached(Appender *pAppender) const
{
    QReadLocker locker(&mAppenderGuard);

    // Keep objects with a 0 reference count safe
    LogObjectPtr < Appender > p_appender = pAppender;

    return mAppenders.contains(p_appender);
}

void AppenderAttachable::removeAllAppenders()
{
// Avoid deadlock:
// - Only log warnings without having the write log aquired
// - Hold a reference to all appenders so that the remove does not
//   destruct the appender over the reference count while the write
//   log is held. The appender may log messages.

    //logger()->trace("Removing all appenders from logger '%1'", name());

    QList < LogObjectPtr<Appender> > appenders;
    {
        QWriteLocker locker(&mAppenderGuard);
        QMutableListIterator < LogObjectPtr<Appender> > i(mAppenders);
        while (i.hasNext()) {
            Appender *p_appender = i.next();
            ListAppender *p_listappender = qobject_cast<ListAppender*> (p_appender);
            if (p_listappender && p_listappender->configuratorList())
                continue;
            else {
                appenders << p_appender;
                i.remove();
            }
        }
    }
    appenders.clear();
}

    void AppenderAttachable::removeAppender(Appender *pAppender)
    {
    // Avoid deadlock:
    // - Only log warnings without having the write log aquired
    // - Hold a reference to the appender so that the remove does not
    //   destruct the appender over the reference count while the write
    //   log is held. The appender may log messages.

    LogObjectPtr < Appender > p_appender = pAppender;

    if (!p_appender) {
        //logger()->warn("Request to remove null Appender from Logger '%1'", name());
        return;
    }
    int n;
    {
        QWriteLocker locker(&mAppenderGuard);

        n = mAppenders.removeAll(p_appender);
    }
    if (n == 0) {
        //logger()->warn(
        //      "Request to remove Appender '%2', which is not part of Logger '%1' appenders",
        //      name(), p_appender->name());
    return;
    }
}

void AppenderAttachable::removeAppender(const QString &rName)
{
    Appender *p_appender = appender(rName);
    if (p_appender)
        removeAppender(p_appender);
}


/**************************************************************************
 * Implementation: Operators, Helper
 **************************************************************************/


} // namespace Log4Qt
