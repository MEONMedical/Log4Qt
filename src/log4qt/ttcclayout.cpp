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

#include "ttcclayout.h"

#include "helpers/patternformatter.h"
#include "loggingevent.h"

#include <QDateTime>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

TTCCLayout::TTCCLayout(QObject *parent) :
    AbstractStringLayout(parent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mThreadPrinting(true)
{
    setDateFormat(Relative);
}

TTCCLayout::TTCCLayout(const QString &dateFormat,
                       QObject *parent) :
    AbstractStringLayout(parent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mThreadPrinting(true)
{
    setDateFormat(dateFormat);
}

TTCCLayout::TTCCLayout(DateFormat dateFormat,
                       QObject *parent) :
    AbstractStringLayout(parent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mThreadPrinting(true)
{
    setDateFormat(dateFormat);
}

void TTCCLayout::setDateFormat(DateFormat dateFormat)
{
    switch (dateFormat)
    {
    case None:
        setDateFormat(u"NONE"_s);
        break;
    case Iso8601:
        setDateFormat(u"ISO8601"_s);
        break;
    case Absolute:
        setDateFormat(u"ABSOLUTE"_s);
        break;
    case Date:
        setDateFormat(u"DATE"_s);
        break;
    case Relative:
        setDateFormat(u"RELATIVE"_s);
        break;
    default:
        Q_ASSERT_X(false, "TTCCLayout::setDateFormat", "Unknown DateFormat");
        setDateFormat(QString());
    }
}


QString TTCCLayout::format(const LoggingEvent &event)
{
    Q_ASSERT_X(mPatternFormatter, "TTCCLayout::format()", "mpPatternConverter must not be null");

    return mPatternFormatter->format(event);
}


void TTCCLayout::updatePatternFormatter()
{
    QString pattern;

    pattern += u"%d{"_s +  mDateFormat + u"}"_s;
    if (mThreadPrinting)
        pattern += u" [%t]"_s;
    pattern += u" %-5p"_s;
    if (mCategoryPrefixing)
        pattern += u" %c"_s;
    if (mContextPrinting)
        pattern += u" %x"_s;
    pattern += u" - %m%n"_s;

    mPatternFormatter.reset(new PatternFormatter(pattern));
}

} // namespace Log4Qt

#include "moc_ttcclayout.cpp"
