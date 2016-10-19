/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logger.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:     Sep 2008, Martin Heinrich:
 *              - Fixed problem in Qt 4.4 where QReadWriteLock is by default
 *                non-recursive.
 *
 *
 * Copyright 2007 - 2008 Martin Heinrich
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

#include "logger.h"

#include "appenderskeleton.h"
#include "varia/listappender.h"
#include "loggingevent.h"
#include "log4qt.h"
#include "loggerrepository.h"
#include "logmanager.h"
#include "logstream.h"

#include <QThread>
#include <QCoreApplication>

namespace Log4Qt
{

Logger::Logger(LoggerRepository *pLoggerRepository, Level level,
               const QString &rName, Logger *pParent) :
    QObject(Q_NULLPTR),
    mName(rName), mpLoggerRepository(pLoggerRepository), mAdditivity(true),
    mLevel(level), mpParent(pParent)
{
    Q_ASSERT_X(pLoggerRepository, "Logger::Logger()",
               "Construction of Logger with null LoggerRepository");

    setObjectName( mName);
}

Logger::~Logger()
{
    logger()->warn("Unexpected destruction of Logger");

    // QWriteLocker locker(&mObjectGuard);
    //
    // QMutableListIterator< LogObjectPtr<Appender> > i(mAppenders);
    // while (i.hasNext())
    // {
    //     i.next();
    //     i.remove();
    // }
}

void Logger::setLevel(Level level)
{
    if ((parentLogger() == Q_NULLPTR) && (level == Level::NULL_INT))
    {
        logger()->warn(
            "Invalid root logger level NULL_INT. Using DEBUG_INT instead");
        level = Level::DEBUG_INT;
    }
    mLevel = level;
}

// Note: use MainThreadAppender if you want write the log from non-main threads
// within the main trhead
void Logger::callAppenders(const LoggingEvent &rEvent) const
{
    QReadLocker locker(&mAppenderGuard);

    for (auto pAppender : mAppenders)
        pAppender->doAppend(rEvent);
    if (additivity() && (parentLogger() != Q_NULLPTR))
        parentLogger()->callAppenders(rEvent);
}

Level Logger::effectiveLevel() const
{
    Q_ASSERT_X(LogManager::rootLogger()->level() != Level::NULL_INT,
               "Logger::effectiveLevel()", "Root logger level must not be NULL_INT");

    QReadLocker locker(&mAppenderGuard);

    const Logger *p_logger = this;
    while (p_logger->level() == Level::NULL_INT)
        p_logger = p_logger->parentLogger();
    return p_logger->level();
}

bool Logger::isEnabledFor(Level level) const
{
    if (mpLoggerRepository->isDisabled(level))
        return false;
    return (effectiveLevel() <= level);
}

Logger *Logger::logger(const QString &rName)
{
    return LogManager::logger(rName);
}

Logger *Logger::logger(const char *pName)
{
    return LogManager::logger(QLatin1String(pName));
}

Logger *Logger::rootLogger()
{
    return LogManager::rootLogger();
}

void Logger::forcedLog(Level level, const QString &rMessage) const
{
    QReadLocker locker(&mAppenderGuard);

    LoggingEvent event(this, level, rMessage);
    callAppenders(event);
}

bool Logger::additivity() const
{
    return mAdditivity;
}

Level Logger::level() const
{
    return mLevel;
}

LoggerRepository *Logger::loggerRepository() const
{
    return mpLoggerRepository;
}

QString Logger::name() const
{
    // QReadLocker locker(&mObjectGuard); // Constant for object lifetime
    return mName;
}

Logger *Logger::parentLogger() const
{
    return mpParent;
}

void Logger::setAdditivity(bool additivity)
{
    mAdditivity = additivity;
}

// Level operations

bool Logger::isDebugEnabled() const
{
    return isEnabledFor(Level::DEBUG_INT);
}

bool Logger::isErrorEnabled() const
{
    return isEnabledFor(Level::ERROR_INT);
}

bool Logger::isFatalEnabled() const
{
    return isEnabledFor(Level::FATAL_INT);
}

bool Logger::isInfoEnabled() const
{
    return isEnabledFor(Level::INFO_INT);
}

bool Logger::isTraceEnabled() const
{
    return isEnabledFor(Level::TRACE_INT);
}

bool Logger::isWarnEnabled() const
{
    return isEnabledFor(Level::WARN_INT);
}

// Log operations: debug

LogStream Logger::debug() const
{
    return LogStream(*this, Level::DEBUG_INT);
}

void Logger::debug(const LogError &rLogError) const
{
    if (isEnabledFor(Level::DEBUG_INT))
        forcedLog(Level::DEBUG_INT, rLogError.toString());
}

void Logger::debug(const QString &rMessage) const
{
    if (isEnabledFor(Level::DEBUG_INT))
        forcedLog(Level::DEBUG_INT, rMessage);
}

// Log operations: error

LogStream Logger::error() const
{
    return LogStream(*this, Level::ERROR_INT);
}

void Logger::error(const QString &rMessage) const
{
    if (isEnabledFor(Level::ERROR_INT))
        forcedLog(Level::ERROR_INT, rMessage);
}

void Logger::error(const LogError &rLogError) const
{
    if (isEnabledFor(Level::ERROR_INT))
        forcedLog(Level::ERROR_INT, rLogError.toString());
}

// Log operations: fatal

LogStream Logger::fatal() const
{
    return LogStream(*this, Level::FATAL_INT);
}

void Logger::fatal(const QString &rMessage) const
{
    if (isEnabledFor(Level::FATAL_INT))
        forcedLog(Level::FATAL_INT, rMessage);
}

void Logger::fatal(const LogError &rLogError) const
{
    if (isEnabledFor(Level::FATAL_INT))
        forcedLog(Level::FATAL_INT, rLogError.toString());
}

// Log operations: info

LogStream Logger::info() const
{
    return LogStream(*this, Level::INFO_INT);
}

void Logger::info(const QString &rMessage) const
{
    if (isEnabledFor(Level::INFO_INT))
        forcedLog(Level::INFO_INT, rMessage);
}

void Logger::info(const LogError &rLogError) const
{
    if (isEnabledFor(Level::INFO_INT))
        forcedLog(Level::INFO_INT, rLogError.toString());
}

// Log operations: log

LogStream Logger::log(Level level) const
{
    return LogStream(*this, level);
}

void Logger::log(Level level,
                 const QString &rMessage) const
{
    if (isEnabledFor(level))
        forcedLog(level, rMessage);
}

void Logger::log(Level level,
                 const LogError &rLogError) const
{
    if (isEnabledFor(level))
        forcedLog(level, rLogError.toString());
}

// Log operations: trace

LogStream Logger::trace() const
{
    return LogStream(*this, Level::TRACE_INT);
}

void Logger::trace(const QString &rMessage) const
{
    if (isEnabledFor(Level::TRACE_INT))
        forcedLog(Level::TRACE_INT, rMessage);
}

void Logger::trace(const LogError &rLogError) const
{
    if (isEnabledFor(Level::TRACE_INT))
        forcedLog(Level::TRACE_INT, rLogError.toString());
}

// Log operations: warn

LogStream Logger::warn() const
{
    return LogStream(*this, Level::WARN_INT);
}

void Logger::warn(const QString &rMessage) const
{
    if (isEnabledFor(Level::WARN_INT))
        forcedLog(Level::WARN_INT, rMessage);
}

void Logger::warn(const LogError &rLogError) const
{
    if (isEnabledFor(Level::WARN_INT))
        forcedLog(Level::WARN_INT, rLogError.toString());
}

} // namespace Log4Qt
