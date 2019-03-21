/******************************************************************************
 *
 * package:     Log4Qt
 * file:        level.h
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

#ifndef LOG4QT_LEVEL_H
#define LOG4QT_LEVEL_H

#include "log4qt.h"

#include <QString>
#include <QMetaType>

namespace Log4Qt
{

/*!
 * \brief The class Level defines the level of a logging event.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class LOG4QT_EXPORT Level
{
public:
    // Comparisson operators rely on the order:
    // NULL_INT < ALL_INT < TRACE_INT < ...
    // Serialisation uses unsigned 8 bit int

    /*!
     * The enumeration Value contains all possible Level values.
     */
    enum Value
    {
        /*! NULL_INT is used for no level has been specified */
        NULL_INT = 0,
        ALL_INT = 32,
        TRACE_INT = 64,
        DEBUG_INT = 96,
        INFO_INT = 128,
        WARN_INT = 150,
        ERROR_INT = 182,
        FATAL_INT = 214,
        OFF_INT = 255
    };

public:
    Level(Value value = NULL_INT)
        : mValue(value)
    {
    }

    int syslogEquivalent() const;
    int toInt() const
    {
        return mValue;
    }

    bool operator==(const Level &other) const
    {
        return mValue == other.mValue;
    }
    bool operator!=(const Level &other) const
    {
        return mValue != other.mValue;
    }
    bool operator<(const Level &other) const
    {
        return mValue < other.mValue;
    }
    bool operator<=(const Level &other) const
    {
        return mValue <= other.mValue;
    }
    bool operator>(const Level &other) const
    {
        return mValue > other.mValue;
    }
    bool operator>=(const Level &other) const
    {
        return mValue >= other.mValue;
    }
    QString toString() const;

    static Level fromString(const QString &level, bool *ok = nullptr);

private:
    volatile Value mValue;

#ifndef QT_NO_DATASTREAM
    // Needs to be friend to stream objects
    friend QDataStream &operator<<(QDataStream &out,
                                   Log4Qt::Level level);
    friend QDataStream &operator>>(QDataStream &in,
                                   Level &level);
#endif // QT_NO_DATASTREAM
};

#ifndef QT_NO_DATASTREAM
/*!
 * \relates Level
 *
 * Writes the given error \a rLevel to the given stream \a rStream,
 * and returns a reference to the stream.
 */
QDataStream &operator<<(QDataStream &out,
                        Log4Qt::Level level);

/*!
 * \relates Level
 *
 * Reads an error from the given stream \a rStream into the given
 * error \a rLevel, and returns a reference to the stream.
 */
QDataStream &operator>>(QDataStream &in,
                        Level &level);
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt

Q_DECLARE_METATYPE(Log4Qt::Level)
Q_DECLARE_TYPEINFO(Log4Qt::Level, Q_MOVABLE_TYPE);


#endif // LOG4QT_LEVEL_H
