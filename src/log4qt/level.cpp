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

#include "level.h"
#include "logger.h"

#include <QDataStream>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::Level)

int Level::syslogEquivalent() const
{
    const int value = toInt();
    
    switch (value)
    {
    case NULL_INT:
    case ALL_INT:
    case TRACE_INT:
    case DEBUG_INT:
        return 7;
    case INFO_INT:
        return 6;
    case WARN_INT:
        return 4;
    case ERROR_INT:
        return 3;
    case FATAL_INT:
    case OFF_INT:
        return 0;
    default:
        Q_ASSERT_X(false, "Level::syslogEquivalent()", "Unknown level value");
        return 7;
    }
}

QString Level::toString() const
{
    // Level names are part of the log format: tooling parses them and
    // fromString() must accept them again, so they are locale-independent
    // literals. The returned QStrings reference static literal data, which
    // also keeps toString() usable while the process is shutting down.
    switch (toInt())
    {
    case NULL_INT:  return u"NULL"_s;
    case ALL_INT:   return u"ALL"_s;
    case TRACE_INT: return u"TRACE"_s;
    case DEBUG_INT: return u"DEBUG"_s;
    case INFO_INT:  return u"INFO"_s;
    case WARN_INT:  return u"WARN"_s;
    case ERROR_INT: return u"ERROR"_s;
    case FATAL_INT: return u"FATAL"_s;
    case OFF_INT:   return u"OFF"_s;
    }
    Q_ASSERT_X(false, "Level::toString()", "Unknown level value");
    return u"NULL"_s;
}

Level Level::fromString(QStringView level, bool *ok)
{
    if (ok != nullptr)
        *ok = true;

    if (level == u"OFF"_s)
        return OFF_INT;
    if (level == u"FATAL"_s)
        return FATAL_INT;
    if (level == u"ERROR"_s)
        return ERROR_INT;
    if (level == u"WARN"_s)
        return WARN_INT;
    if (level == u"INFO"_s)
        return INFO_INT;
    if (level == u"DEBUG"_s)
        return DEBUG_INT;
    if (level == u"TRACE"_s)
        return TRACE_INT;
    if (level == u"ALL"_s)
        return ALL_INT;
    if (level == u"NULL"_s)
        return NULL_INT;

    logger()->warn(u"Use of invalid level string '%1'. Using 'Level::NULL_INT' instead."_s, level);
    if (ok != nullptr)
        *ok = false;
    return NULL_INT;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out,
                        Log4Qt::Level level)
{
    const quint8 l = static_cast<quint8>(level.toInt());
    out << l;
    return out;
}

QDataStream &operator>>(QDataStream &in,
                        Level &level)
{
    quint8 l;
    in >> l;
    level.mValue = static_cast<Level::Value>(l);
    return in;
}
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt
