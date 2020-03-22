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

#ifndef LOG4QT_LOG4QTSHAREDPTR_H
#define LOG4QT_LOG4QTSHAREDPTR_H

#include <QSharedPointer>
#include <QObject>

#include <type_traits>

namespace Log4Qt
{

template<typename Log4QtClass>
class Log4QtSharedPtr : public QSharedPointer<Log4QtClass>
{
public:
    Log4QtSharedPtr(Log4QtClass *ptr)
        : QSharedPointer<Log4QtClass>(ptr, &Log4QtClass::deleteLater)
    {
        static_assert(std::is_base_of<QObject, Log4QtClass>::value, "Need a QObject derived class here");
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
