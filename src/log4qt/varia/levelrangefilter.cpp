/******************************************************************************
 *
 * package:     Log4Qt
 * file:        levelrangefilter.cpp
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


#include "varia/levelrangefilter.h"

#include "loggingevent.h"

namespace Log4Qt
{

LevelRangeFilter::LevelRangeFilter(QObject *pParent) :
    Filter(pParent),
    mAcceptOnMatch(true),
    mLevelMin(Level::NULL_INT),
    mLevelMax(Level::OFF_INT)
{}


Filter::Decision LevelRangeFilter::decide(const LoggingEvent &rEvent) const
{
    if (rEvent.level() < mLevelMin)
        return Filter::DENY;

    if (rEvent.level() > mLevelMax)
        return Filter::DENY;

    if (mAcceptOnMatch)
        return Filter::ACCEPT;
    else
        return Filter::NEUTRAL;
}

} // namespace Log4Qt
