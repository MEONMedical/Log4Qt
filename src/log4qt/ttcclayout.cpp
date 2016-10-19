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

TTCCLayout::TTCCLayout(QObject *pParent) :
    Layout(pParent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mDateFormat(),
    mThreadPrinting(true),
    mpPatternFormatter(Q_NULLPTR)
{
    setDateFormat(RELATIVE);
}


TTCCLayout::TTCCLayout(const QString &rDateFormat,
                       QObject *pParent) :
    Layout(pParent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mDateFormat(rDateFormat),
    mThreadPrinting(true),
    mpPatternFormatter(Q_NULLPTR)
{
}


TTCCLayout::TTCCLayout(DateFormat dateFormat,
                       QObject *pParent) :
    Layout(pParent),
    mCategoryPrefixing(true),
    mContextPrinting(true),
    mDateFormat(),
    mThreadPrinting(true),
    mpPatternFormatter(Q_NULLPTR)
{
    setDateFormat(dateFormat);
}


TTCCLayout::~TTCCLayout()
{
    delete mpPatternFormatter;
}


void TTCCLayout::setDateFormat(DateFormat dateFormat)
{
    switch (dateFormat)
    {
    case NONE:
        setDateFormat(QLatin1String("NONE"));
        break;
    case ISO8601:
        setDateFormat(QLatin1String("ISO8601"));
        break;
    case ABSOLUTE:
        setDateFormat(QLatin1String("ABSOLUTE"));
        break;
    case DATE:
        setDateFormat(QLatin1String("DATE"));
        break;
    case RELATIVE:
        setDateFormat(QLatin1String("RELATIVE"));
        break;
    default:
        Q_ASSERT_X(false, "TTCCLayout::setDateFormat", "Unkown DateFormat");
        setDateFormat(QString());
    }
}


QString TTCCLayout::format(const LoggingEvent &rEvent)
{
    Q_ASSERT_X(mpPatternFormatter, "TTCCLayout::format()", "mpPatternConverter must not be null");

    return mpPatternFormatter->format(rEvent);
}


void TTCCLayout::updatePatternFormatter()
{
    QString pattern;

    pattern += QLatin1String("%d{") +  mDateFormat + QLatin1String("}");
    if (mThreadPrinting)
        pattern += QLatin1String(" [%t]");
    pattern += QLatin1String(" %-5p");
    if (mCategoryPrefixing)
        pattern += QLatin1String(" %c");
    if (mContextPrinting)
        pattern += QLatin1String(" %x");
    pattern += QLatin1String(" - %m%n");

    delete mpPatternFormatter;
    mpPatternFormatter = new PatternFormatter(pattern);
}

} // namespace Log4Qt
