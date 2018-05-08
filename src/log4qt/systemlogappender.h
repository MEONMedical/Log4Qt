/******************************************************************************
 *
 * package:     Log4Qt
 * file:        databaseappender.h
 * created:     December 2010
 * author:      Michael Filonenko
 *
 *
 * Copyright 2010 Michael Filonenko
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
#ifndef SYSTEMLOGAPPENDER_H
#define SYSTEMLOGAPPENDER_H

#include "appenderskeleton.h"

namespace Log4Qt
{

/*!
 * \brief The class SystemLogAppender appends log events to a Event Log under win*
 * and to syslog under *nix.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT SystemLogAppender: public AppenderSkeleton
{
    Q_OBJECT

    /**
     * The property holds the port used by the telenet appender.
     *
     * The default is QCoreApplication::applicationName()
     *
     * \sa serviceName(), setServiceName()
     */
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName)

public:
    explicit SystemLogAppender(QObject *parent = nullptr);
    ~SystemLogAppender() override;

    bool requiresLayout() const override;
    QString serviceName() const;
    void setServiceName(const QString &serviceName);

protected:
    void append(const Log4Qt::LoggingEvent &event) override;

    QString mServiceName;
    char *ident;
};

inline bool SystemLogAppender::requiresLayout() const
{
    return true;
}

}

#endif //#ifndef SYSTEMLOGAPPENDER_H
