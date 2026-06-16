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

#include "patternlayout.h"

#include "abstractstringlayout.h"
#include "helpers/patternformatter.h"
#include "loggingevent.h"

using namespace Qt::StringLiterals;

namespace Log4Qt
{

PatternLayout::PatternLayout(QObject *parent) :
    AbstractStringLayout(parent)
{
    setConversionPattern(DefaultPattern);
}

PatternLayout::PatternLayout(const QString &pattern,
                             QObject *parent) :
    AbstractStringLayout(parent)
{
    setConversionPattern(pattern);
}

PatternLayout::PatternLayout(ConversionPattern conversionPattern,
                             QObject *parent) :
    AbstractStringLayout(parent)
{
    setConversionPattern(conversionPattern);
}

void PatternLayout::setConversionPattern(ConversionPattern conversionPattern)
{
    switch (conversionPattern)
    {
    case DefaultPattern:
        setConversionPattern(u"%m%n"_s);
        break;
    case TtccPattern:
        setConversionPattern(u"%r [%t] %p %c %x - %m%n"_s);
        break;
    default:
        Q_ASSERT_X(false, "PatternLayout::setConversionFormat", "Unknown ConversionFormat");
        setConversionPattern(QString());
    }
}

QString PatternLayout::format(const LoggingEvent &event)
{
    Q_ASSERT_X(mpPatternFormatter, "PatternLayout::format()", "mpPatternConverter must not be null");

    return mpPatternFormatter->format(event);
}

bool PatternLayout::requiresLocation() const
{
    return mpPatternFormatter && mpPatternFormatter->requiresLocation();
}

void PatternLayout::updatePatternFormatter()
{
    mpPatternFormatter = std::make_unique<PatternFormatter>(mPattern);
}

void PatternLayout::setHeaderPattern(const QString &pattern)
{
    mHeaderPattern = pattern;
    mHeaderFormatter = pattern.isEmpty() ? nullptr
                                         : std::make_unique<PatternFormatter>(pattern);
}

void PatternLayout::setFooterPattern(const QString &pattern)
{
    mFooterPattern = pattern;
    mFooterFormatter = pattern.isEmpty() ? nullptr
                                         : std::make_unique<PatternFormatter>(pattern);
}

QString PatternLayout::header() const
{
    if (auto p = headerFooterProvider()) {
        QString h = p->header();
        if (!h.isEmpty()) return h;
    }
    if (mHeaderFormatter)
        return mHeaderFormatter->format(LoggingEvent{});
    return AbstractStringLayout::header();
}

QString PatternLayout::footer() const
{
    if (auto p = headerFooterProvider()) {
        QString f = p->footer();
        if (!f.isEmpty()) return f;
    }
    if (mFooterFormatter)
        return mFooterFormatter->format(LoggingEvent{});
    return AbstractStringLayout::footer();
}

} // namespace Log4Qt

#include "moc_patternlayout.cpp"

