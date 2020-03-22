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

#include "binarylogstream.h"
#include "binarylogger.h"
#include "binaryloggingevent.h"

#include <QByteArray>
#include <QPointer>

namespace Log4Qt
{

struct BinaryLogStream::Stream
{
    Stream(const Logger *logger, Level level)
        : logger(logger)
        , level(level)
    {
    }
    ~Stream();

    QByteArray buffer;
    QPointer<const Logger> logger;
    Level level;
};

BinaryLogStream::Stream::~Stream()
{
    if (auto blogger = qobject_cast<const BinaryLogger *>(logger))
        blogger->log(level, buffer);
    else if (logger->isEnabledFor(level))
    {
        BinaryLoggingEvent event(logger, level, buffer);
        logger->callAppenders(event);
    }
}

BinaryLogStream::BinaryLogStream(const Logger *logger, Level level)
    : mStream(new Stream(logger, level))
{
}

BinaryLogStream &BinaryLogStream::operator<<(const QByteArray &data)
{
    mStream->buffer.append(data);
    return *this;
}

} // namespace Log4Qt
