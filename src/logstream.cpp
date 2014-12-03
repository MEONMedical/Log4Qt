/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logstream.h
 * created:     March, 2011
 * author:      Tim Besard
 *
 *
 * Copyright 2011 Tim Besard
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

#include "logstream.h"
#include "logger.h"

namespace Log4Qt
{

LogStream::LogStream(const Logger &iLogger, Level iLevel)
    : stream(new Stream(&iLogger, iLevel))
{
}

LogStream::Stream::Stream(const Logger *iLogger, Level iLevel)
    : ts(&buffer, QIODevice::WriteOnly)
    , mLogger(iLogger)
    , mLevel(iLevel)
{
}

LogStream::Stream::~Stream()
{
    if (!mLogger.isNull())
        mLogger->log(mLevel, buffer);
}

} // namespace Log4Qt
