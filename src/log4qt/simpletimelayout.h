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

#ifndef LOG4QT_SIMPLETIMELAYOUT_H
#define LOG4QT_SIMPLETIMELAYOUT_H

#include "layout.h"

namespace Log4Qt
{

/*!
 * \brief The class SimpleTimeLayout outputs the time, logger name, thread, level and message of a logging
 *        event.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT SimpleTimeLayout : public Layout
{
    Q_OBJECT

public:
    SimpleTimeLayout(QObject *parent = nullptr);

private:
    Q_DISABLE_COPY_MOVE(SimpleTimeLayout)

public:
    QString format(const LoggingEvent &event) override;
};

inline SimpleTimeLayout::SimpleTimeLayout(QObject *parent) :
    Layout(parent)
{}

} // namespace Log4Qt

#endif // LOG4QT_SIMPLETIMELAYOUT_H
