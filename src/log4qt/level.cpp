/******************************************************************************
 *
 * package:     Log4Qt
 * file:        level.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::Level)

int Level::syslogEquivalent() const
{
    switch (mValue)
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
    const char *p_context = "Level";

    switch (mValue)
    {
    case NULL_INT:
        return QCoreApplication::translate(p_context, "NULL");
    case ALL_INT:
        return QCoreApplication::translate(p_context, "ALL");
    case TRACE_INT:
        return QCoreApplication::translate(p_context, "TRACE");
    case DEBUG_INT:
        return QCoreApplication::translate(p_context, "DEBUG");
    case INFO_INT:
        return QCoreApplication::translate(p_context, "INFO");
    case WARN_INT:
        return QCoreApplication::translate(p_context, "WARN");
    case ERROR_INT:
        return QCoreApplication::translate(p_context, "ERROR");
    case FATAL_INT:
        return QCoreApplication::translate(p_context, "FATAL");
    case OFF_INT:
        return QCoreApplication::translate(p_context, "OFF");
    default:
        Q_ASSERT_X(false, "Level::toString()", "Unknown level value");
        return QCoreApplication::translate(p_context, "NULL");
    }
}

Level Level::fromString(const QString &level, bool *ok)
{
    const char *context = "Level";
    if (ok != nullptr)
        *ok = true;

    if (level == QStringLiteral("OFF") ||
            level == QCoreApplication::translate(context, "OFF"))
        return OFF_INT;
    if (level == QStringLiteral("FATAL") ||
            level == QCoreApplication::translate(context, "FATAL"))
        return FATAL_INT;
    if (level == QStringLiteral("ERROR") ||
            level == QCoreApplication::translate(context, "ERROR"))
        return ERROR_INT;
    if (level == QStringLiteral("WARN") ||
            level == QCoreApplication::translate(context, "WARN"))
        return WARN_INT;
    if (level == QStringLiteral("INFO") ||
            level == QCoreApplication::translate(context, "INFO"))
        return INFO_INT;
    if (level == QStringLiteral("DEBUG") ||
            level == QCoreApplication::translate(context, "DEBUG"))
        return DEBUG_INT;
    if (level == QStringLiteral("TRACE") ||
            level == QCoreApplication::translate(context, "TRACE"))
        return TRACE_INT;
    if (level == QStringLiteral("ALL") ||
            level == QCoreApplication::translate(context, "ALL"))
        return ALL_INT;
    if (level == QStringLiteral("NULL") ||
            level == QCoreApplication::translate(context, "NULL"))
        return NULL_INT;

    logger()->warn(QStringLiteral("Use of invalid level string '%1'. Using 'Level::NULL_INT' instead."), level);
    if (ok != nullptr)
        *ok = false;
    return NULL_INT;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &out,
                        Log4Qt::Level level)
{
    quint8 l = level.mValue;
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
