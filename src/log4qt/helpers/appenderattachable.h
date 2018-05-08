/******************************************************************************
 *
 * package:     Log4Qt
 * file:        appenderattachable.h
 * created:     December 2010
 * author:      Andreas Bacher
 *
 *
 * Copyright 2010 Andreas Bacher
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

#ifndef LOG4QT_APPENDERATTACHABLE_H
#define LOG4QT_APPENDERATTACHABLE_H

#include <log4qt/log4qt.h>
#include <log4qt/appender.h>

#include <QList>
#include <QReadWriteLock>

namespace Log4Qt
{

/*!
 * \brief Implementation for attaching appenders to objects
 */
class LOG4QT_EXPORT AppenderAttachable
{

public:
    AppenderAttachable();
    virtual ~AppenderAttachable() = default;

    /*!
     * Add an appender.
     */
    virtual void addAppender(const AppenderSharedPtr &appender);

    /*!
     * Get all previously added appenders as an Enumeration.
     */
    virtual QList<AppenderSharedPtr> appenders() const;

    /*!
     * Get an appender by name.
     */
    virtual AppenderSharedPtr appender(const QString &name) const;

    /*!
     Returns <code>true</code> if the specified appender is in the
     list of attached appenders, <code>false</code> otherwise.
    */
    virtual bool isAttached(const AppenderSharedPtr &appender) const;

    /*!
     * Removes all appenders that have been previously added from this
     * Logger.
     *
     * To allow configurators to collect events during the configuration
     * process ListAppenders with the configuratorList property set, will
     * not be removed.
     *
     * \sa LisAppender::setConfiguratorList()
     */
    virtual void removeAllAppenders();

    /*!
     * Remove the appender passed as parameter from the list of appenders.
     */
    virtual void removeAppender(const AppenderSharedPtr &appender);

    /*!
     * Remove the appender with the name passed as parameter from the
     * list of appenders.
     */
    virtual void removeAppender(const QString &name);

protected:
    QList<AppenderSharedPtr> mAppenders;
    mutable QReadWriteLock mAppenderGuard;
};

} // namespace Log4Qt

#endif // LOG4QT_APPENDERATTACHABLE_H
