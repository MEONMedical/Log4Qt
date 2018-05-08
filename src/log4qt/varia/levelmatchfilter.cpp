/******************************************************************************
 *
 * package:     Log4Qt
 * file:        levelmatchfilter.cpp
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

#include "varia/levelmatchfilter.h"

#include "loggingevent.h"

namespace Log4Qt
{

LevelMatchFilter::LevelMatchFilter(QObject *parent) :
    Filter(parent),
    mAcceptOnMatch(true),
    mLevelToMatch(Level::NULL_INT)
{}

Filter::Decision LevelMatchFilter::decide(const LoggingEvent &event) const
{
    if (mLevelToMatch == Level::NULL_INT ||
            event.level() != mLevelToMatch)
        return Filter::NEUTRAL;

    if (mAcceptOnMatch)
        return Filter::ACCEPT;
    return Filter::DENY;
}

} // namespace Log4Qt

#include "moc_levelmatchfilter.cpp"
