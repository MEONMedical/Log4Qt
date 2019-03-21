/******************************************************************************
 *
 * package:
 * file:        hierarchy.cpp
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

#include "hierarchy.h"

#include "logger.h"
#include "binarylogger.h"
#include "helpers/optionconverter.h"

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(static_logger, ::LoggerRepository)

Hierarchy::Hierarchy() :
    mObjectGuard(QReadWriteLock::Recursive),
    mThreshold(Level::NULL_INT),
    mRootLogger(logger(QString()))
{
}

Hierarchy::~Hierarchy()
{
    static_logger()->warn(QStringLiteral("Unexpected destruction of Hierarchy"));
}

bool Hierarchy::exists(const QString &name) const
{
    QReadLocker locker(&mObjectGuard);

    return mLoggers.contains(name);
}

Logger *Hierarchy::logger(const QString &name)
{
    QWriteLocker locker(&mObjectGuard);

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

    Logger *p_logging_logger = logger(QLatin1String(""));
    Logger *p_qt_logger = logger(QStringLiteral("Qt"));
    Logger *p_root_logger = rootLogger();

    for (auto &&p_logger : qAsConst(mLoggers))
    {
        if ((p_logger == p_logging_logger) || (p_logger == p_qt_logger) || (p_logger == p_root_logger))
            continue;
        resetLogger(p_logger, Level::NULL_INT);
    }
    resetLogger(p_qt_logger, Level::NULL_INT);
    resetLogger(p_logging_logger, Level::NULL_INT);
    resetLogger(p_root_logger, Level::DEBUG_INT);
}

void Hierarchy::shutdown()
{
    static_logger()->debug(QStringLiteral("Shutting down Hierarchy"));
    resetConfiguration();
}

Logger *Hierarchy::createLogger(const QString &orgName)
{
    static const QLatin1String binaryIndicator = QLatin1String("@@binary@@");
    static const QLatin1String name_separator = QLatin1String("::");

    QString name(OptionConverter::classNameJavaToCpp(orgName));
    bool needBinaryLogger = orgName.contains(binaryIndicator);

    if (needBinaryLogger)
        name.remove(binaryIndicator);

    Logger *logger = mLoggers.value(name, nullptr);
    if (logger != nullptr)
        return logger;

    if (name.isEmpty())
    {
        logger = new Logger(this, Level::DEBUG_INT, QStringLiteral("root"), nullptr);
        mLoggers.insert(QString(), logger);
        return logger;
    }
    QString parent_name;
    int index = name.lastIndexOf(name_separator);
    if (index >= 0)
        parent_name = name.left(index);

    if (needBinaryLogger)
        logger = new BinaryLogger(this, Level::NULL_INT, name, createLogger(parent_name));
    else
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
