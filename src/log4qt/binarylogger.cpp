/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#include "binarylogger.h"
#include "binaryloggingevent.h"
#include "helpers/classlogger.h"
#include "helpers/datetime.h"

namespace Log4Qt
{

BinaryLogger::BinaryLogger(LoggerRepository *loggerRepository, Level level, const QString &name, Logger *parent)
    : Logger(loggerRepository, level, name, parent)
{
}

BinaryLogger::~BinaryLogger() = default;

void BinaryLogger::forcedLog(Level level, const QByteArray &message) const
{
    BinaryLoggingEvent event(this, level, message);
    callAppenders(event);
}

BinaryLogStream BinaryLogger::log(Level level) const
{
    return BinaryLogStream(this, level);
}

void BinaryLogger::log(Level level, const QByteArray &message) const
{
    if (isEnabledFor(level))
        forcedLog(level, message);
}

void BinaryLogger::log(Level level, const QByteArray &message, QDateTime timeStamp) const
{
    if (isEnabledFor(level))
    {
        BinaryLoggingEvent event(this, level, message, timeStamp.toMSecsSinceEpoch());
        callAppenders(event);
    }
}

} // namespace Log4Qt

#include "moc_binarylogger.cpp"

