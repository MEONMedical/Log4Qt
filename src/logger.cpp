/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logger.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:		Sep 2008, Martin Heinrich:
 * 				- Fixed problem in Qt 4.4 where QReadWriteLock is by default
 *  			  non-recursive.
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

/******************************************************************************
 * Dependencies
 ******************************************************************************/

#include "logger.h"

#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>

#include "appenderskeleton.h"
#include "varia/listappender.h"
#include "loggingevent.h"
#include "log4qt.h"
#include "loggerrepository.h"
#include "logmanager.h"
#include "logstream.h"

namespace Log4Qt {

  /**************************************************************************
   * Declarations
   **************************************************************************/

  /**************************************************************************
   * C helper functions
   **************************************************************************/

  /**************************************************************************
   * Class implementation: Logger
   **************************************************************************/

  Logger::Logger(LoggerRepository* pLoggerRepository, Level level,
    const QString &rName, Logger *pParent) :
    QObject(0),
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
    // QWriteLocker locker(&mObjectGuard); // Read/Write int is safe

    if ((parentLogger() == 0) && (level == Level::NULL_INT)) {
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

      Appender *p_appender;
      Q_FOREACH(p_appender, mAppenders)
          p_appender->doAppend(rEvent);
      if (additivity() && (parentLogger() != 0))
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

#ifndef QT_NO_DEBUG_STREAM
  QDebug Logger::debug(QDebug &rDebug) const
  {
    QReadLocker locker(&mAppenderGuard);

    QString parent_logger;
    if (mpParent)
      parent_logger = mpParent->name();

    rDebug.nospace() << "Logger(" << "name:" << name() << " " << "appenders:"
        << mAppenders.count() << " " << "additivity:" << mAdditivity << " "
        << mLevel << "parentLogger:" << parent_logger << ")";
    return rDebug.space();
  }
#endif // QT_NO_DEBUG_STREAM

  void Logger::forcedLog(Level level, const QString &rMessage) const
  {
    QReadLocker locker(&mAppenderGuard);

    LoggingEvent event(this, level, rMessage);
    callAppenders(event);
  }


  /**************************************************************************
   * Implementation: Operators, Helper
   **************************************************************************/

#ifndef QT_NO_DEBUG_STREAM
  QDebug operator<<(QDebug debug, const Logger &rLogger)
  {
    return rLogger.debug(debug);
  }
#endif // QT_NO_DEBUG_STREAM

  /**************************************************************************
   * Inline
   **************************************************************************/

  bool Logger::additivity() const
  {   // QReadLocker locker(&mObjectGuard); // Read/Write of int is safe
                  return mAdditivity; }

  Level Logger::level() const
  {   // QReadLocker locker(&mObjectGuard); // Level is thread-safe
                  return mLevel;  }

  LoggerRepository *Logger::loggerRepository() const
  {   // QReadLocker locker(&mObjectGuard); // Constant for object lifetime
                  return mpLoggerRepository;    }

  QString Logger::name() const
  {   // QReadLocker locker(&mObjectGuard); // Constant for object lifetime
                  return mName;    }

   Logger *Logger::parentLogger() const
  {   // QReadLocker locker(&mObjectGuard); // Constant for object lifetime
                  return mpParent;    }

  void Logger::setAdditivity(bool additivity)
  {   // QWriteLocker locker(&mObjectGuard); // Read/Write of int is safe
                  mAdditivity = additivity;  }

  // Level operations

  bool Logger::isDebugEnabled() const
  {   return isEnabledFor(Level::DEBUG_INT);  }

  bool Logger::isErrorEnabled() const
  {   return isEnabledFor(Level::ERROR_INT);  }

  bool Logger::isFatalEnabled() const
  {   return isEnabledFor(Level::FATAL_INT);  }

  bool Logger::isInfoEnabled() const
  {   return isEnabledFor(Level::INFO_INT);  }

  bool Logger::isTraceEnabled() const
  {   return isEnabledFor(Level::TRACE_INT);  }

  bool Logger::isWarnEnabled() const
  {   return isEnabledFor(Level::WARN_INT);  }

  // Log operations: debug

  LogStream Logger::debug() const
  {   return LogStream(*this, Level::DEBUG_INT); }

  void Logger::debug(const LogError &rLogError) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, rLogError.toString());  }

  void Logger::debug(const QString &rMessage) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, rMessage);  }

  void Logger::debug(const char *pMessage) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage)); }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::debug(const char *pMessage,
                                                                                                          const QVariant &rArg1,
                                                                                                          const QVariant &rArg2,
                                                                                                          const QVariant &rArg3) const
  {   if (isEnabledFor(Level::DEBUG_INT))
                  forcedLog(Level::DEBUG_INT, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }

  // Log operations: error

  LogStream Logger::error() const
  {   return LogStream(*this, Level::ERROR_INT); }

  void Logger::error(const QString &rMessage) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, rMessage);  }

  void Logger::error(const LogError &rLogError) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, rLogError.toString());  }

  void Logger::error(const char *pMessage) const
  {   if (isEnabledFor(Level::ERROR_INT))
                                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage)); }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1) const
  {   if (isEnabledFor(Level::ERROR_INT))
                                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::error(const char *pMessage,
                                                                                                          const QVariant &rArg1,
                                                                                                          const QVariant &rArg2,
                                                                                                          const QVariant &rArg3) const
  {   if (isEnabledFor(Level::ERROR_INT))
                  forcedLog(Level::ERROR_INT, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }

  // Log operations: fatal

  LogStream Logger::fatal() const
  {   return LogStream(*this, Level::FATAL_INT); }

  void Logger::fatal(const QString &rMessage) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, rMessage);  }

  void Logger::fatal(const LogError &rLogError) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, rLogError.toString());  }

  void Logger::fatal(const char *pMessage) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage)); }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1, const QString &rArg2) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1, int arg2) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::fatal(const char *pMessage,
                                                                                                          const QVariant &rArg1,
                                                                                                          const QVariant &rArg2,
                                                                                                          const QVariant &rArg3) const
  {   if (isEnabledFor(Level::FATAL_INT))
                  forcedLog(Level::FATAL_INT, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }

  // Log operations: info

  LogStream Logger::info() const
  {   return LogStream(*this, Level::INFO_INT); }

  void Logger::info(const QString &rMessage) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, rMessage);  }

  void Logger::info(const LogError &rLogError) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, rLogError.toString());  }

  void Logger::info(const char *pMessage) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage)); }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   const QString &rArg2) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   int arg2) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   const QString &rArg2) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   int arg2) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   const QString &rArg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   const QString &rArg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   int arg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   int arg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   const QString &rArg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   const QString &rArg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   int arg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   int arg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::info(const char *pMessage,
                                                                                                   const QVariant &rArg1,
                                                                                                   const QVariant &rArg2,
                                                                                                   const QVariant &rArg3) const
  {   if (isEnabledFor(Level::INFO_INT))
                  forcedLog(Level::INFO_INT, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }

  // Log operations: log

  LogStream Logger::log(Level level) const
  {   return LogStream(*this, level); }

  void Logger::log(Level level,
                                                                                                  const QString &rMessage) const
  {   if (isEnabledFor(level))
                  forcedLog(level, rMessage);  }

  void Logger::log(Level level,
                                                                                                  const LogError &rLogError) const
  {   if (isEnabledFor(level))
                  forcedLog(level, rLogError.toString());  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage)); }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1,
                                                                                                  const QString &rArg2) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1, int arg2) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1,
                                                                                                  const QString &rArg2) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1,
                                                                                                  int arg2) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1,
                                                                                                  const QString &rArg2,
                                                                                                  const QString &rArg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1,
                                                                                                  const QString &rArg2,
                                                                                                  int arg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1,
                                                                                                  int arg2,
                                                                                                  const QString &rArg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QString &rArg1,
                                                                                                  int arg2,
                                                                                                  int arg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1,
                                                                                                  const QString &rArg2,
                                                                                                  const QString &rArg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1,
                                                                                                  const QString &rArg2,
                                                                                                  int arg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1,
                                                                                                  int arg2,
                                                                                                  const QString &rArg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  int arg1,
                                                                                                  int arg2,
                                                                                                  int arg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::log(Level level,
                                                                                                  const char *pMessage,
                                                                                                  const QVariant &rArg1,
                                                                                                  const QVariant &rArg2,
                                                                                                  const QVariant &rArg3) const
  {   if (isEnabledFor(level))
                  forcedLog(level, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }

  // Log operations: trace

  LogStream Logger::trace() const
  {   return LogStream(*this, Level::TRACE_INT); }

  void Logger::trace(const QString &rMessage) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, rMessage);  }

  void Logger::trace(const LogError &rLogError) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, rLogError.toString());  }

  void Logger::trace(const char *pMessage) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage)); }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QString &rArg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          const QString &rArg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          const QString &rArg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          int arg1,
                                                                                                          int arg2,
                                                                                                          int arg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::trace(const char *pMessage,
                                                                                                          const QVariant &rArg1,
                                                                                                          const QVariant &rArg2,
                                                                                                          const QVariant &rArg3) const
  {   if (isEnabledFor(Level::TRACE_INT))
                  forcedLog(Level::TRACE_INT, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }

  // Log operations: warn

  LogStream Logger::warn() const
  {   return LogStream(*this, Level::WARN_INT); }

  void Logger::warn(const QString &rMessage) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, rMessage);  }

  void Logger::warn(const LogError &rLogError) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, rLogError.toString());  }

  void Logger::warn(const char *pMessage) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage)); }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   const QString &rArg2) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   int arg2) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   const QString &rArg2) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   int arg2) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   const QString &rArg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2, rArg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   const QString &rArg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1, rArg2).arg(arg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   int arg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(rArg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QString &rArg1,
                                                                                                   int arg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1).arg(arg2).arg(arg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   const QString &rArg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(rArg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   const QString &rArg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1).arg(rArg2).arg(arg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   int arg2,
                                                                                                   const QString &rArg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(rArg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   int arg1,
                                                                                                   int arg2,
                                                                                                   int arg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(arg1).arg(arg2).arg(arg3));  }

  void Logger::warn(const char *pMessage,
                                                                                                   const QVariant &rArg1,
                                                                                                   const QVariant &rArg2,
                                                                                                   const QVariant &rArg3) const
  {   if (isEnabledFor(Level::WARN_INT))
                  forcedLog(Level::WARN_INT, QString::fromUtf8(pMessage).arg(rArg1.toString(), rArg2.toString(), rArg3.toString()));  }



} // namespace Log4Qt
