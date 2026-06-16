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

#ifndef LOG4QT_LOG4QTSHAREDPTR_H
#define LOG4QT_LOG4QTSHAREDPTR_H

#include <QSharedPointer>
#include <QObject>

#include <type_traits>

namespace Log4Qt
{

/*!
 * \brief A shared pointer for Log4Qt QObject-derived classes.
 *
 * This class provides a QSharedPointer that automatically uses
 * deleteLater() for cleanup, ensuring proper Qt object lifecycle
 * management.
 *
 * \note Only accepts QObject-derived types (enforced via static_assert).
 */
template<typename Log4QtClass>
class Log4QtSharedPtr : public QSharedPointer<Log4QtClass>
{
public:
    /*!
     * Constructs a Log4QtSharedPtr that takes ownership of \a ptr.
     * The object will be deleted via deleteLater() when the last
     * reference is destroyed.
     *
     * \note Explicit to prevent silent conversions from raw pointers —
     * an implicit conversion is a use-after-free risk if the source is
     * a stack pointer, a member of another object, or already owned.
     * Callers must write \c MyPtr(new T(...)) at the point of ownership
     * transfer.
     */
    explicit Log4QtSharedPtr(Log4QtClass *ptr)
        : QSharedPointer<Log4QtClass>(ptr, &Log4QtClass::deleteLater)
    {
        static_assert(std::is_base_of_v<QObject, Log4QtClass>,
                      "Log4QtClass must be derived from QObject");
    }

    Log4QtSharedPtr()
        : QSharedPointer<Log4QtClass>()
    {
    }

    Log4QtSharedPtr(const QSharedPointer<Log4QtClass> &other)
        : QSharedPointer<Log4QtClass>(other)
    {
    }

    Log4QtSharedPtr(const QWeakPointer<Log4QtClass> &other)
        : QSharedPointer<Log4QtClass>(other)
    {
    }
};

}

#endif // LOG4QT_LOG4QTSHAREDPTR_H
