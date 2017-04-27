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

#ifndef LOG4QT_LOGSTREAM_H
#define LOG4QT_LOGSTREAM_H

#include "level.h"

#include <QTextStream>
#include <QString>
#include <QSharedPointer>
#include <QPointer>

namespace Log4Qt
{
class Logger;

class LOG4QT_EXPORT LogStream
{
public:
    LogStream(const Logger &iLogger, Level iLevel);
    template<typename T>
    LogStream &operator<<(const T &t)
    {
        stream->ts << t;
        return *this;
    }

private:
    struct Stream
    {
        Stream(const Logger *iLogger, Level iLevel);
        ~Stream();

        QTextStream ts;
        QString buffer;
        QPointer<const Logger> mLogger;
        Level mLevel;
    };
    QSharedPointer<Stream> stream;
};
}

#endif // LOG4QT_LOGSTREAM_H
