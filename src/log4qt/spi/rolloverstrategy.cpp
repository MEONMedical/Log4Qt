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

#include "spi/rolloverstrategy.h"

#include "helpers/logerror.h"
#include "logger.h"

#include <QFile>

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(static_logger, Log4Qt::RolloverStrategy)

RolloverStrategy::RolloverStrategy(QObject *parent) :
    QObject(parent)
{}

RolloverStrategy::~RolloverStrategy() = default;

void RolloverStrategy::activateOptions()
{}

QString RolloverStrategy::initialFileName(const QString &fileName) const
{
    return fileName;
}

bool RolloverStrategy::removeFile(const QString &fileName)
{
    QFile f(fileName);
    if (!f.exists())
        return true;
    if (f.remove())
        return true;

    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to remove file '%1' during rollover"),
                              AppenderRemoveFileError,
                              "Log4Qt::RolloverStrategy");
    e << fileName;
    e.addCausingError(LogError(f.errorString(), f.error()));
    static_logger()->error(e);
    return false;
}

bool RolloverStrategy::renameFile(const QString &source, const QString &target)
{
    QFile f(source);
    if (f.rename(target))
        return true;

    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Unable to rename file '%1' to '%2' during rollover"),
                              AppenderRenamingFileError,
                              "Log4Qt::RolloverStrategy");
    e << source << target;
    e.addCausingError(LogError(f.errorString(), f.error()));
    static_logger()->error(e);
    return false;
}

} // namespace Log4Qt

#include "moc_rolloverstrategy.cpp"
