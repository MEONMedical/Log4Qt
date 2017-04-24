/******************************************************************************
 *
 * package:
 * file:        hierarchy.h
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

#ifndef LOG4QT_HIERARCHY_H
#define LOG4QT_HIERARCHY_H

#include "loggerrepository.h"

#include <QHash>
#include <QReadWriteLock>

namespace Log4Qt
{

/*!
 * \brief The class Hierarchy implements a logger repository.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class  LOG4QT_EXPORT Hierarchy : public LoggerRepository
{
public:
    Hierarchy();
    virtual ~Hierarchy();


public:
    virtual bool exists(const QString &rName) const Q_DECL_OVERRIDE;
    virtual Logger *logger(const QString &rName) Q_DECL_OVERRIDE;
    virtual QList<Logger *> loggers() const Q_DECL_OVERRIDE;
    virtual Logger *rootLogger() const Q_DECL_OVERRIDE;
    virtual Level threshold() const Q_DECL_OVERRIDE;
    virtual void setThreshold(Level level) Q_DECL_OVERRIDE;
    virtual void setThreshold(const QString &rThreshold) Q_DECL_OVERRIDE;

    virtual bool isDisabled(Level level) Q_DECL_OVERRIDE;
    virtual void resetConfiguration() Q_DECL_OVERRIDE;
    virtual void shutdown() Q_DECL_OVERRIDE;

private:
    Logger *createLogger(const QString &rName);
    void resetLogger(Logger *pLogger, Level level) const;

private:
    mutable QReadWriteLock mObjectGuard;
    QHash<QString, Logger *> mLoggers;
    Level mThreshold;
    Logger *mpRootLogger;
};

inline Logger *Hierarchy::rootLogger() const
{
    // QReadLocker locker(&mObjectGuard); // Constant for object lifetime
    return mpRootLogger;
}

inline Level Hierarchy::threshold() const
{
    // QReadLocker locker(&mObjectGuard); // Level is threadsafe
    return mThreshold;
}

inline void Hierarchy::setThreshold(Level level)
{
    // QReadLocker locker(&mObjectGuard); // Level is threadsafe
    mThreshold = level;
}

inline bool Hierarchy::isDisabled(Level level)
{
    // QReadLocker locker(&mObjectGuard); // Level is threadsafe
    return level < mThreshold;
}


} // namespace Log4Qt

#endif // LOG4QT_HIERARCHY_H
