/******************************************************************************
 *
 * package:     Log4Qt
 * file:        patternlayout.cpp
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

#include "patternlayout.h"

#include "helpers/patternformatter.h"
#include "loggingevent.h"

namespace Log4Qt
{


PatternLayout::PatternLayout(QObject *parent) :
    Layout(parent)
{
    setConversionPattern(DEFAULT_CONVERSION_PATTERN);
}

PatternLayout::PatternLayout(const QString &pattern,
                             QObject *parent) :
    Layout(parent)
{
    setConversionPattern(pattern);
}

PatternLayout::PatternLayout(ConversionPattern conversionPattern,
                             QObject *parent) :
    Layout(parent)
{
    setConversionPattern(conversionPattern);
}

void PatternLayout::setConversionPattern(ConversionPattern conversionPattern)
{
    switch (conversionPattern)
    {
    case DEFAULT_CONVERSION_PATTERN:
        setConversionPattern(QStringLiteral("%m%n"));
        break;
    case TTCC_CONVERSION_PATTERN:
        setConversionPattern(QStringLiteral("%r [%t] %p %c %x - %m%n"));
        break;
    default:
        Q_ASSERT_X(false, "PatternLayout::setConversionFormat", "Unknown ConversionFormat");
        setConversionPattern(QString());
    }
}

QString PatternLayout::format(const LoggingEvent &event)
{
    Q_ASSERT_X(mPatternFormatter, "PatternLayout::format()", "mpPatternConverter must not be null");

    return mPatternFormatter->format(event);
}

void PatternLayout::updatePatternFormatter()
{
    mPatternFormatter.reset(new PatternFormatter(mPattern));
}

} // namespace Log4Qt

#include "moc_patternlayout.cpp"

