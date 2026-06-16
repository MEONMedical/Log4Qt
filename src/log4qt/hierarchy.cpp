/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
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

#include "hierarchy.h"

#include "logger.h"
#include "helpers/optionconverter.h"

#include <algorithm>
#include <utility>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(static_logger, ::LoggerRepository)

Hierarchy::Hierarchy()
    : mObjectGuard(QReadWriteLock::Recursive)
    , mThreshold(Level::NULL_INT)
    , mRootLogger(createLogger(QString()))
{}

Hierarchy::~Hierarchy()
{
    static_logger()->warn(u"Unexpected destruction of Hierarchy"_s);

    QWriteLocker locker(&mObjectGuard);
    // Hierarchy is a friend of Logger, so we can reach the protected destructor.
    for (Logger *logger : std::as_const(mLoggers))
        delete logger;
    mLoggers.clear();
    mRootLogger = nullptr;
}

bool Hierarchy::exists(const QString &name) const
{
    QReadLocker locker(&mObjectGuard);

    return mLoggers.contains(name);
}

Logger *Hierarchy::logger(const QString &name)
{
    // A single write lock is used deliberately instead of a read-lock
    // fast-path followed by a write-lock on miss. mObjectGuard is a
    // *recursive* QReadWriteLock, and logger() is called re-entrantly while
    // the write lock is already held: resetConfiguration() holds the write
    // lock and calls Logger::setLevel(NULL_INT) on the root logger, which
    // logs a warning that resolves a class logger back through logger().
    // Qt's recursive QReadWriteLock only recognizes recursion within the same
    // lock mode — acquiring a read lock while the thread holds the write lock
    // deadlocks. A recursive write-acquire is safe, so we take the write lock
    // directly. Logger lookups are cached at every call site (see the
    // LOG4QT_DECLARE_*_LOGGER macros), so this is not a hot path.
    QWriteLocker writeLocker(&mObjectGuard);
    return createLogger(name);
}

QList<Logger *> Hierarchy::loggers() const
{
    QReadLocker locker(&mObjectGuard);

    return mLoggers.values();
}

void Hierarchy::setThreshold(const QString &threshold)
{
    setThreshold(Level::fromString(threshold));
}

void Hierarchy::resetConfiguration()
{
    QWriteLocker locker(&mObjectGuard);

    // Reset all loggers.
    // Leave log, qt and root logger to the last to allow debugging of shutdown.

    Logger *p_logging_logger = createLogger(u""_s);
    Logger *p_qt_logger = createLogger(u"Qt"_s);
    Logger *p_root_logger = mRootLogger;

    // Define predicate for regular (non-special) loggers
    auto isRegularLogger = [=](Logger* logger) {
        return logger != p_logging_logger && 
               logger != p_qt_logger && 
               logger != p_root_logger;
    };

    // Reset all regular loggers
    auto loggers = mLoggers.values();
    std::for_each(loggers.begin(), loggers.end(), 
                  [&](Logger* logger) {
                      if (isRegularLogger(logger)) {
                          resetLogger(logger, Level::NULL_INT);
                      }
                  });
    
    // Reset special loggers
    resetLogger(p_qt_logger, Level::NULL_INT);
    resetLogger(p_logging_logger, Level::NULL_INT);
    resetLogger(p_root_logger, Level::DEBUG_INT);
}

void Hierarchy::shutdown()
{
    static_logger()->debug(u"Shutting down Hierarchy"_s);
    resetConfiguration();
}

Logger *Hierarchy::createLogger(const QString &orgName)
{
    static const auto name_separator = u"::"_s;

    QString name(OptionConverter::classNameJavaToCpp(orgName));

    Logger *logger = mLoggers.value(name, nullptr);
    if (logger != nullptr)
        return logger;

    if (name.isEmpty())
    {
        logger = new Logger(this, Level::DEBUG_INT, u"root"_s, nullptr);
        mLoggers.insert(QString(), logger);
        return logger;
    }
    QString parent_name;
    int index = name.lastIndexOf(name_separator);
    if (index >= 0)
        parent_name = name.left(index);

    logger = new Logger(this, Level::NULL_INT, name, createLogger(parent_name));
    mLoggers.insert(name, logger);
    return logger;
}

void Hierarchy::resetLogger(Logger *logger, Level level) const
{
    logger->removeAllAppenders();
    logger->setAdditivity(true);
    logger->setLevel(level);
}

} // namespace Log4Qt
