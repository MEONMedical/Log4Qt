/******************************************************************************
 *
 * package:     Log4Qt
 * file:        signalappender.h
 * created:     March 2010
 * author:      Filonenko Michael
 *
 *
 * Copyright 2010 Filonenko Michael
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

#ifndef SIGNALAPPENDER_H
#define SIGNALAPPENDER_H

#include "appenderskeleton.h"
#include "loggingevent.h"

namespace Log4Qt
{

/*!
 * \ingroup log4qt
 * @class SignalAppender signalappender.h "src/kernel/components/signalappender.h"
 */
class  LOG4QT_EXPORT SignalAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    explicit SignalAppender(QObject *parent = Q_NULLPTR);

    bool requiresLayout() const Q_DECL_OVERRIDE;

protected:
    virtual void append(const Log4Qt::LoggingEvent &rEvent) Q_DECL_OVERRIDE;

signals:
    /*!
    * @param message
    */
    void appended(const QString &message);

};

inline bool SignalAppender::requiresLayout() const
{
    return true;
}

} // namespace Log4Qt

#endif // SIGNALAPPENDER_H
