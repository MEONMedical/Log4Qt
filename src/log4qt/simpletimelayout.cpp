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

QString SimpleTimeLayout::format(const LoggingEvent &event)
{
    return DateTime::fromMSecsSinceEpoch(event.timeStamp()).toString(QStringLiteral("dd.MM.yyyy hh:mm"))
           + QStringLiteral("[") + event.threadName() + QStringLiteral("]")
           + QStringLiteral(" ") + event.level().toString()
           + QStringLiteral(" ") + event.loggename()
           + QStringLiteral(" - ") + event.message() + Layout::endOfLine();
}


} // namespace Log4Qt

#include "moc_simpletimelayout.cpp"
