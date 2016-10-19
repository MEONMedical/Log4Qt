/******************************************************************************
 *
 * package:     Log4Qt
 * file:        mainthreadappender.h
 * created:     Febrary 2011
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

#ifndef LOG4QT_MAINTHREADAPPENDER_H
#define LOG4QT_MAINTHREADAPPENDER_H

#include "appenderskeleton.h"
#include "helpers/appenderattachable.h"

#include <QQueue>
#include <QMutex>

namespace Log4Qt
{

/*!
 * \brief The class MainThreadAppender uses the QEvent system to write
 *        log from not main threads within the main thread.
 *
 *
 * \note All the functions declared in this class are thread-safe.
 * &nbsp;
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT  MainThreadAppender : public AppenderSkeleton, public AppenderAttachable
{
    Q_OBJECT

public:
    MainThreadAppender(QObject *parent = Q_NULLPTR);
    virtual ~MainThreadAppender();

    virtual bool requiresLayout() const Q_DECL_OVERRIDE;

    virtual void activateOptions() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;

    /*!
     * Tests if all entry conditions for using append() in this class are
     * met.
     *
     * If a conditions is not met, an error is logged and the function
     * returns false. Otherwise the result of
     * AppenderSkeleton::checkEntryConditions() is returned.
     *
     * The checked conditions are:
     * - none
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by AppenderSkeleton::doAppend().
     *
     * \sa AppenderSkeleton::doAppend(),
     *     AppenderSkeleton::checkEntryConditions()
     */
    virtual bool checkEntryConditions() const Q_DECL_OVERRIDE;

protected:
    virtual void append(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(MainThreadAppender)

};


} // namespace Log4Qt

#endif
