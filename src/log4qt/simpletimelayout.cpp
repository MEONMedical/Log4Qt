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

#include "simpletimelayout.h"

#include "loggingevent.h"
#include "helpers/datetime.h"

#include <QStringBuilder>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

QString SimpleTimeLayout::format(const LoggingEvent &event)
{
    return DateTime::formatMsecs(event.timeStamp(), u"dd.MM.yyyy hh:mm"_s)
           % u"["_s % event.threadName() % u"] "_s
           % event.level().toString()
           % u' ' % event.loggername()
           % u" - "_s % event.message()
           % AbstractLayout::endOfLine();
}


} // namespace Log4Qt

#include "moc_simpletimelayout.cpp"
