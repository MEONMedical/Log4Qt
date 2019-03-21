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

Logger::Logger(LoggerRepository *loggerRepository, Level level,
               const QString &name, Logger *parent) :
    QObject(nullptr),
    mName(name), mLoggerRepository(loggerRepository), mAdditivity(true),
    mLevel(level), mParentLogger(parent)
{
    Q_ASSERT_X(loggerRepository, "Logger::Logger()",
               "Construction of Logger with null LoggerRepository");

    setObjectName( mName);
}

Logger::~Logger()
{
    logger()->warn(QStringLiteral("Unexpected destruction of Logger"));
}

void Logger::setLevel(Level level)
{
    if ((parentLogger() == nullptr) && (level == Level::NULL_INT))
    {
        logger()->warn(
            QStringLiteral("Invalid root logger level NULL_INT. Using DEBUG_INT instead"));
        level = Level::DEBUG_INT;
    }
    mLevel = level;
}

// Note: use MainThreadAppender if you want write the log from non-main threads
// within the main trhead
void Logger::callAppenders(const LoggingEvent &event) const
{
    QReadLocker locker(&mAppenderGuard);

    for (auto &&appender : qAsConst(mAppenders))
        appender->doAppend(event);
    if (additivity() && (parentLogger() != nullptr))
        parentLogger()->callAppenders(event);
}

Level Logger::effectiveLevel() const
{
    Q_ASSERT_X(LogManager::rootLogger()->level() != Level::NULL_INT,
               "Logger::effectiveLevel()", "Root logger level must not be NULL_INT");

    QReadLocker locker(&mAppenderGuard);

    const Logger *logger = this;
    while (logger->level() == Level::NULL_INT)
        logger = logger->parentLogger();
    return logger->level();
}

bool Logger::isEnabledFor(Level level) const
{
    if (mLoggerRepository->isDisabled(level))
        return false;
    return (effectiveLevel() <= level);
}

Logger *Logger::logger(const QString &name)
{
    return LogManager::logger(name);
}

Logger *Logger::logger(const char *name)
{
    return LogManager::logger(QLatin1String(name));
}

Logger *Logger::rootLogger()
{
    return LogManager::rootLogger();
}

void Logger::forcedLog(Level level, const QString &message) const
{
    LoggingEvent event(this, level, message);
    forcedLog(event);
}

void Logger::forcedLog(const LoggingEvent &logEvent) const
{
    callAppenders(logEvent);
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
    return mLoggerRepository;
}

QString Logger::name() const
{
    return mName;
}

Logger *Logger::parentLogger() const
{
    return mParentLogger;
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

void Logger::debug(const LogError &logError) const
{
    if (isEnabledFor(Level::DEBUG_INT))
        forcedLog(Level::DEBUG_INT, logError.toString());
}

void Logger::debug(const QString &message) const
{
    if (isEnabledFor(Level::DEBUG_INT))
        forcedLog(Level::DEBUG_INT, message);
}

// Log operations: error

LogStream Logger::error() const
{
    return LogStream(*this, Level::ERROR_INT);
}

void Logger::error(const QString &message) const
{
    if (isEnabledFor(Level::ERROR_INT))
        forcedLog(Level::ERROR_INT, message);
}

void Logger::error(const LogError &logError) const
{
    if (isEnabledFor(Level::ERROR_INT))
        forcedLog(Level::ERROR_INT, logError.toString());
}

// Log operations: fatal

LogStream Logger::fatal() const
{
    return LogStream(*this, Level::FATAL_INT);
}

void Logger::fatal(const QString &message) const
{
    if (isEnabledFor(Level::FATAL_INT))
        forcedLog(Level::FATAL_INT, message);
}

void Logger::fatal(const LogError &logError) const
{
    if (isEnabledFor(Level::FATAL_INT))
        forcedLog(Level::FATAL_INT, logError.toString());
}

// Log operations: info

LogStream Logger::info() const
{
    return LogStream(*this, Level::INFO_INT);
}

void Logger::info(const QString &message) const
{
    if (isEnabledFor(Level::INFO_INT))
        forcedLog(Level::INFO_INT, message);
}

void Logger::info(const LogError &logError) const
{
    if (isEnabledFor(Level::INFO_INT))
        forcedLog(Level::INFO_INT, logError.toString());
}

// Log operations: log

LogStream Logger::log(Level level) const
{
    return LogStream(*this, level);
}

void Logger::log(Level level,
                 const QString &message) const
{
    if (isEnabledFor(level))
        forcedLog(level, message);
}

void Logger::log(const LoggingEvent &logEvent) const
{
    if (isEnabledFor(logEvent.level()))
        forcedLog(logEvent);
}

void Logger::logWithLocation(Level level, const char *file, int line, const char *function, const QString &message) const
{
    LoggingEvent loggingEvent = LoggingEvent(this,
                                             level,
                                             message,
                                             MessageContext(file, line, function),
                                             QString());
    forcedLog(loggingEvent);
}

void Logger::log(Level level,
                 const LogError &logError) const
{
    if (isEnabledFor(level))
        forcedLog(level, logError.toString());
}

// Log operations: trace

LogStream Logger::trace() const
{
    return LogStream(*this, Level::TRACE_INT);
}

void Logger::trace(const QString &message) const
{
    if (isEnabledFor(Level::TRACE_INT))
        forcedLog(Level::TRACE_INT, message);
}

void Logger::trace(const LogError &logError) const
{
    if (isEnabledFor(Level::TRACE_INT))
        forcedLog(Level::TRACE_INT, logError.toString());
}

// Log operations: warn

LogStream Logger::warn() const
{
    return LogStream(*this, Level::WARN_INT);
}

void Logger::warn(const QString &message) const
{
    if (isEnabledFor(Level::WARN_INT))
        forcedLog(Level::WARN_INT, message);
}

void Logger::warn(const LogError &logError) const
{
    if (isEnabledFor(Level::WARN_INT))
        forcedLog(Level::WARN_INT, logError.toString());
}

void MessageLogger::log(const QString &message) const
{
    mLogger->logWithLocation(mLevel, mContext.file, mContext.line, mContext.function, message);
}

LogStream MessageLogger::log() const
{
    return LogStream(*mLogger.data(), mLevel);
}

} // namespace Log4Qt

#include "moc_logger.cpp"

