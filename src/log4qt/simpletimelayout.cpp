/******************************************************************************
 *
 * package:     Log4Qt
 * file:        simpletimelayout.cpp
 * created:     March 2010
 * author:      Filonenko Michael
 *
 *
 * Copyright 2010 Filonenko Michael
 *
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

#include "simpletimelayout.h"

#include "loggingevent.h"
#include "helpers/datetime.h"


namespace Log4Qt
{

QString SimpleTimeLayout::format(const LoggingEvent &rEvent)
{
    return DateTime::fromMSecsSinceEpoch(rEvent.timeStamp()).toString("dd.MM.yyyy hh:mm")
           + QLatin1String("[") + rEvent.threadName() + QLatin1String("]")
           + QLatin1String(" ") + rEvent.level().toString()
           + QLatin1String(" ") + rEvent.loggerName()
           + QLatin1String(" - ") + rEvent.message() + Layout::endOfLine();
}


} // namespace Log4Qt
