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

} // namespace Log4Qt
