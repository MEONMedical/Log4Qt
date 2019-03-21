/******************************************************************************
 *
 * package:     Log4Qt
 * file:        patternformatter.h
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

#ifndef LOG4QT_PATTERNFORMATTER_H
#define LOG4QT_PATTERNFORMATTER_H

#include <log4qt/log4qtshared.h>

#include <QList>
#include <QString>

namespace Log4Qt
{

class FormattingInfo;
class PatternConverter;
class LoggingEvent;

/*!
 * \brief The class PatternFormatter formats a logging event based on a
 *        pattern string.
 *
 * The class PatternFormatter formats a LoggingEvent base on a pattern
 * string. It is used by the patternLayout and TTCCLayout class to
 * implement the formatting.
 *
 * On object construction the provided patterns tring is parsed. Based on
 * the information found a chain of PatternConverter is created. Each
 * PatternConverter handles a certain member of a LoggingEvent.
 *
 * \sa PatternLayout::format()
 * \sa TTCCLayout::format()
 */
class  LOG4QT_EXPORT PatternFormatter
{
public:
    /*!
     * Creates a PatternFormatter using a the specified \a pattern.
     */
    PatternFormatter(const QString &pattern);

    /*!
     * Destroys the PatternFormatter and all PatternConverter.
     */
    virtual ~PatternFormatter();

private:
    Q_DISABLE_COPY(PatternFormatter)

public:
    /*!
     * Formats the given \a loggingEvent using the chain of
     * PatternConverter created during construction from the specified
     * pattern.
     */
    QString format(const LoggingEvent &loggingEvent) const;

private:
    /*!
     * If the character \a digit is a digit the digit is added to the
     * integer \a value and the function returns true. Otherwise the
     * function returns false.
     *
     * The function adds the digit by multiplying the existing value
     * with ten and adding the numerical value of the digit. If the
     * maximum integer value would be exceeded by the operation
     * \a value is set to INT_MAX.
     */
    bool addDigit(QChar digit,
                  int &value);

    /*!
     * Creates a PatternConverter based on the specified conversion
     * character \a rChar, the formatting information
     * \a formattingInfo and the option \a option.
     *
     * The PatternConverter converter is appended to the list of
     * PatternConverters.
     */
    void createConverter(QChar character,
                         Log4Qt::FormattingInfo formattingInfo,
                         const QString &option = QString());

    /*!
     * Creates a LiteralPatternConverter with the string literal
     * \a literal.
     *
     * The PatternConverter converter is appended to the list of
     * PatternConverters.
     */
    void createLiteralConverter(const QString &literal);

    /*!
     * Parses the pattern string specified on construction and creates
     * PatternConverter according to it.
     */
    void parse();

    /*!
     * Parses an integer option from an option string. If the string is
     * not a valid integer or the integer value is less then zero, zero
     * is returned. Returns the end of line seperator for the operating
     * system.
     */
    int parseIntegeoption(const QString &option);

private:
    const QString mIgnoreCharacters;
    const QString mConversionCharacters;
    const QString mOptionCharacters;
    QString mPattern;
    QList<PatternConverter *> mPatternConverters;
};


} // namespace Log4Qt


Q_DECLARE_TYPEINFO(Log4Qt::PatternFormatter, Q_MOVABLE_TYPE);


#endif // LOG4QT_PATTERNFORMATTER_H
