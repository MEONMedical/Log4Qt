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

#include <QCoreApplication>
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
    // Cache the translated strings on first use. The locale active at first
    // call is the one captured for the lifetime of the process — log-level
    // names are conventionally not retranslated at runtime.
    //
    // The cache is intentionally leaked (held through a pointer, so no
    // destructor is registered): Log4Qt logs during static destruction — the
    // atexit-registered LogManager::shutdown() formats a shutdown event after
    // function-local statics have already been destroyed. Destructible
    // statics here would be read after destruction (use-after-free at exit).
    // This matches the library's design of leaking its singletons so that
    // logging stays usable until the very end of the process.
    static const char *const p_context = "Level";
    struct Names
    {
        QString null, all, trace, debug, info, warn, error, fatal, off;
    };
    static const Names *const s_names = new Names {
        QCoreApplication::translate(p_context, "NULL"),
        QCoreApplication::translate(p_context, "ALL"),
        QCoreApplication::translate(p_context, "TRACE"),
        QCoreApplication::translate(p_context, "DEBUG"),
        QCoreApplication::translate(p_context, "INFO"),
        QCoreApplication::translate(p_context, "WARN"),
        QCoreApplication::translate(p_context, "ERROR"),
        QCoreApplication::translate(p_context, "FATAL"),
        QCoreApplication::translate(p_context, "OFF"),
    };

    switch (toInt())
    {
    case NULL_INT:  return s_names->null;
    case ALL_INT:   return s_names->all;
    case TRACE_INT: return s_names->trace;
    case DEBUG_INT: return s_names->debug;
    case INFO_INT:  return s_names->info;
    case WARN_INT:  return s_names->warn;
    case ERROR_INT: return s_names->error;
    case FATAL_INT: return s_names->fatal;
    case OFF_INT:   return s_names->off;
    }
    Q_ASSERT_X(false, "Level::toString()", "Unknown level value");
    return s_names->null;
}

Level Level::fromString(QStringView level, bool *ok)
{
    const char *context = "Level";
    if (ok != nullptr)
        *ok = true;

    if (level == u"OFF"_s ||
            level == QCoreApplication::translate(context, "OFF"))
        return OFF_INT;
    if (level == u"FATAL"_s ||
            level == QCoreApplication::translate(context, "FATAL"))
        return FATAL_INT;
    if (level == u"ERROR"_s ||
            level == QCoreApplication::translate(context, "ERROR"))
        return ERROR_INT;
    if (level == u"WARN"_s ||
            level == QCoreApplication::translate(context, "WARN"))
        return WARN_INT;
    if (level == u"INFO"_s ||
            level == QCoreApplication::translate(context, "INFO"))
        return INFO_INT;
    if (level == u"DEBUG"_s ||
            level == QCoreApplication::translate(context, "DEBUG"))
        return DEBUG_INT;
    if (level == u"TRACE"_s ||
            level == QCoreApplication::translate(context, "TRACE"))
        return TRACE_INT;
    if (level == u"ALL"_s ||
            level == QCoreApplication::translate(context, "ALL"))
        return ALL_INT;
    if (level == u"NULL"_s ||
            level == QCoreApplication::translate(context, "NULL"))
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
