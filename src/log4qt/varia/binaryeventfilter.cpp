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

#include "varia/binaryeventfilter.h"

#include "binaryloggingevent.h"

namespace Log4Qt
{

BinaryEventFilter::BinaryEventFilter(QObject *parent)
    : Filter(parent)
    , mAcceptBinaryEvents{true}
{
}

Filter::Decision BinaryEventFilter::decide(const LoggingEvent &event) const
{
    bool isBinaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&event) != nullptr;

    if (!isBinaryEvent)
        return Filter::NEUTRAL;

    if (mAcceptBinaryEvents)
        return Filter::ACCEPT;
    return Filter::DENY;
}

} // namespace Log4Qt

#include "moc_binaryeventfilter.cpp"
