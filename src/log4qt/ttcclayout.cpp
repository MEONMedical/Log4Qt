/******************************************************************************
 *
 * package:     Log4Qt
 * file:        ttcclayout.cpp
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

#include "ttcclayout.h"

#include "helpers/datetime.h"
#include "helpers/patternformatter.h"
#include "logger.h"
#include "loggingevent.h"

#include <QDateTime>

namespace Log4Qt
{

TTCCLayout::TTCCLayout(QObject *parent) :
    Layout(parent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mThreadPrinting(true)
{
    setDateFormat(RELATIVE);
}

TTCCLayout::TTCCLayout(const QString &dateFormat,
                       QObject *parent) :
    Layout(parent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mDateFormat(dateFormat),
    mThreadPrinting(true)
{
}

TTCCLayout::TTCCLayout(DateFormat dateFormat,
                       QObject *parent) :
    Layout(parent),
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
    case NONE:
        setDateFormat(QStringLiteral("NONE"));
        break;
    case ISO8601:
        setDateFormat(QStringLiteral("ISO8601"));
        break;
    case ABSOLUTE:
        setDateFormat(QStringLiteral("ABSOLUTE"));
        break;
    case DATE:
        setDateFormat(QStringLiteral("DATE"));
        break;
    case RELATIVE:
        setDateFormat(QStringLiteral("RELATIVE"));
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

    pattern += QStringLiteral("%d{") +  mDateFormat + QStringLiteral("}");
    if (mThreadPrinting)
        pattern += QStringLiteral(" [%t]");
    pattern += QStringLiteral(" %-5p");
    if (mCategoryPrefixing)
        pattern += QStringLiteral(" %c");
    if (mContextPrinting)
        pattern += QStringLiteral(" %x");
    pattern += QStringLiteral(" - %m%n");

    mPatternFormatter.reset(new PatternFormatter(pattern));
}

} // namespace Log4Qt

#include "moc_ttcclayout.cpp"
