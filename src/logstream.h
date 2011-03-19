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


/******************************************************************************
 * Dependencies
 ******************************************************************************/

#include <QtCore/QTextStream>
#include <QtCore/QString>
#include "level.h"


/******************************************************************************
 * Declarations
 ******************************************************************************/

namespace Log4Qt
{
    class Logger;

    class LOG4QT_EXPORT LogStream
    {

    private:
        struct Stream {
            Stream() : ts(&buffer, QIODevice::WriteOnly), ref(1) {}
            QTextStream ts;
            QString buffer;
            int ref;
        } *stream;

    public:
        inline LogStream(const Logger& iLogger, Level iLevel) : stream(new Stream()),  mLogger(iLogger), mLevel(iLevel)
        {
        }
        inline LogStream(const LogStream &o):stream(o.stream),mLogger(o.mLogger),mLevel(o.mLevel)
        {
            ++stream->ref;
        }
        ~LogStream();

        inline LogStream &operator<<(QChar t) { stream->ts << '\'' << t << '\''; return *this; }
        inline LogStream &operator<<(QBool t) { stream->ts << (bool(t != 0) ? "true" : "false"); return *this; }
        inline LogStream &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return *this; }
        inline LogStream &operator<<(char t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(signed short t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(unsigned short t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(signed int t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(unsigned int t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(signed long t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(unsigned long t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(qint64 t) { stream->ts << QString::number(t); return *this; }
        inline LogStream &operator<<(quint64 t) { stream->ts << QString::number(t); return *this; }
        inline LogStream &operator<<(float t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(double t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(const char* t) { stream->ts << QString::fromAscii(t); return *this; }
        inline LogStream &operator<<(const QString & t) { stream->ts << t ; return *this; }
        inline LogStream &operator<<(const QStringRef & t) { return operator<<(t.toString()); }
        inline LogStream &operator<<(const QLatin1String &t) { stream->ts << t.latin1(); return *this; }
        inline LogStream &operator<<(const QByteArray & t) { stream->ts  << t; return *this; }
        inline LogStream &operator<<(const void * t) { stream->ts << t; return *this; }
        inline LogStream &operator<<(QTextStreamFunction f) { stream->ts << f; return *this; }
        inline LogStream &operator<<(QTextStreamManipulator m) { stream->ts << m; return *this; }

    private:
        const Logger& mLogger;
        Level mLevel;
    };
}

#endif // LOG4QT_LOGSTREAM_H
