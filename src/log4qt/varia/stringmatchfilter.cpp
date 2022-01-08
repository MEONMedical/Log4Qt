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

#include "varia/stringmatchfilter.h"

#include "loggingevent.h"

namespace Log4Qt
{

StringMatchFilter::StringMatchFilter(QObject *parent) :
    Filter(parent),
    mAcceptOnMatch(true),
    mStringToMatch(),
    mCaseSensitivity(Qt::CaseInsensitive)
{}


Filter::Decision StringMatchFilter::decide(const LoggingEvent &event) const
{
    if (event.message().isEmpty() ||
        mStringToMatch.isEmpty() ||
        !event.message().contains(mStringToMatch,mCaseSensitivity))
        return Filter::NEUTRAL;

    if (mAcceptOnMatch)
        return Filter::ACCEPT;

    return Filter::DENY;
}

} // namespace Log4Qt

#include "moc_stringmatchfilter.cpp"
