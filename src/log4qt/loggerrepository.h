/******************************************************************************
 *
 * package:     Log4Qt
 * file:        loggerrepository.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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

#ifndef LOG4QT_LOGGERREPOSITORY_H
#define LOG4QT_LOGGERREPOSITORY_H

#include "level.h"

#include <QList>

namespace Log4Qt
{

class Logger;

/*!
 * \brief The class LoggerRepository is abstract base class for a logger
 *        repository.
 */
class LOG4QT_EXPORT LoggerRepository
{
public:
    virtual ~LoggerRepository() = default;

    virtual bool exists(const QString &name) const = 0;
    virtual Logger *logger(const QString &name) = 0;
    virtual QList<Logger *> loggers() const = 0;
    virtual Logger *rootLogger() const = 0;
    virtual Level threshold() const = 0;
    virtual void setThreshold(Level level) = 0;
    virtual void setThreshold(const QString &threshold) = 0;

    virtual bool isDisabled(Level level) = 0;
    virtual void resetConfiguration() = 0;
    virtual void shutdown() = 0;
};

} // namespace Log4Qt

#endif // LOG4QT_LOGGERREPOSITORY_H
