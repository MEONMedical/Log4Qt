/******************************************************************************
 *
 * package:     Log4Qt
 * file:        patternformatter.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes      Feb 2009, Martin Heinrich
 *              - Fixed VS 2008 unreferenced formal parameter warning by using
 *                Q_UNUSED in LiteralPatternConverter::convert.
 *
 *
 * Copyright 2007 - 2009 Martin Heinrich
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

#include "helpers/patternformatter.h"

#include "helpers/datetime.h"
#include "helpers/logerror.h"
#include "layout.h"
#include "logger.h"
#include "loggingevent.h"
#include "logmanager.h"

#include <QString>

#include <limits>

namespace Log4Qt
{

/*!
 * \brief The class FormattingInfo stores the formatting modifier for a
 * pattern converter.
 *
 * \sa PatternConverter
 */
class FormattingInfo
{
public:
    FormattingInfo()
    {
        clear();
    }

    void clear();
    static QString intToString(int i);

public:
    int mMinLength;
    int mMaxLength;
    bool mLeftAligned;
};

/*!
 * \brief The class PatternConverter is the abstract base class for all
 * pattern converters.
 *
 * PatternConverter handles the minimum and maximum modifier for a
 * conversion character. The actual conversion is by calling the
 * convert() member function of the derived class.
 *
 * \sa PatternLayout::format()
 */
class PatternConverter
{
public:
    PatternConverter(Log4Qt::FormattingInfo formattingInfo = FormattingInfo()) :
        mFormattingInfo(formattingInfo)
    {}
    virtual ~PatternConverter() = default;

private:
    Q_DISABLE_COPY(PatternConverter)

public:
    void format(QString &format, const LoggingEvent &loggingEvent) const;

protected:
    virtual QString convert(const LoggingEvent &loggingEvent) const = 0;

protected:
    FormattingInfo mFormattingInfo;
};


/*!
 * \brief The class BasicPatternConverter converts several members of a
 *        LoggingEvent to a string.
 *
 * BasicPatternConverter is used by PatternLayout to convert members that
 * do not reuquire additional formatting to a string as part of formatting
 * the LoggingEvent. It handles the following conversion characters:
 * 'm', 'p', 't', 'x'
 *
 * \sa PatternLayout::format()
 * \sa PatternConverter::format()
 */
class BasicPatternConverter : public PatternConverter
{
public:
    enum Type
    {
        MESSAGE_CONVERTER,
        NDC_CONVERTER,
        LEVEL_CONVERTER,
        THREAD_CONVERTER,
        FILENAME_CONVERTER,
        FUNCTIONNAME_CONVERTER,
        LINENUMBER_CONVERTER,
        LOCATION_CONVERTER,
        CATEGORYNAME_CONVERTER
    };

public:
    BasicPatternConverter(Log4Qt::FormattingInfo formattingInfo,
                          Type type) :
        PatternConverter(formattingInfo),
        mType(type)
    {}

private:
    Q_DISABLE_COPY(BasicPatternConverter)

protected:
    QString convert(const LoggingEvent &loggingEvent) const override;

private:
    Type mType;
};


/*!
 * \brief The class DatePatternConverter converts the time stamp of a
 *        LoggingEvent to a string.
 *
 * DatePatternConverter is used by PatternLayout to convert the time stamp
 * of a LoggingEvent to a string as part of formatting the LoggingEvent.
 * It handles the 'd' and 'r' conversion character.
 *
 * \sa PatternLayout::format()
 * \sa PatternConverter::format()
 */
class DatePatternConverter : public PatternConverter
{
public:
    DatePatternConverter(Log4Qt::FormattingInfo formattingInfo,
                         const QString &format) :
        PatternConverter(formattingInfo),
        mFormat(format)
    {}

private:
    Q_DISABLE_COPY(DatePatternConverter)

protected:
    QString convert(const LoggingEvent &loggingEvent) const override;

private:
    QString mFormat;
};


/*!
 * \brief The class LiteralPatternConverter provides string literals.
 *
 * LiteralPatternConverter is used by PatternLayout to embed string
 * literals as part of formatting the LoggingEvent. It handles string
 * literals and the 'n' conversion character.
 *
 * \sa PatternLayout::format()
 * \sa PatternConverter::format()
 */
class LiteralPatternConverter : public PatternConverter
{
public:
    LiteralPatternConverter(const QString &literal) :
        mLiteral(literal)
    {}

private:
    Q_DISABLE_COPY(LiteralPatternConverter)

protected:
    QString convert(const LoggingEvent &loggingEvent) const override;

private:
    QString mLiteral;
};


/*!
 * \brief The class LoggepatternConverter converts the Logger name of a
 *        LoggingEvent to a string.
 *
 * LoggepatternConverter is used by PatternLayout to convert the Logger
 * name of a LoggingEvent to a string as part of formatting the
 * LoggingEvent. It handles the 'c' conversion character.
 *
 * \sa PatternLayout::format()
 * \sa PatternConverter::format()
 */
class LoggepatternConverter : public PatternConverter
{
public:
    LoggepatternConverter(Log4Qt::FormattingInfo formattingInfo,
                           int precision) :
        PatternConverter(formattingInfo),
        mPrecision(precision)
    {}

private:
    Q_DISABLE_COPY(LoggepatternConverter)

protected:
    QString convert(const LoggingEvent &loggingEvent) const override;

private:
    int mPrecision;
};



/*!
 * \brief The class MDCPatternConverter converts the MDC data of a
 *        LoggingEvent to a string.
 *
 * MDCPatternConverter is used by PatternLayout to convert the MDC data of
 * a LoggingEvent to a string as part of formatting the LoggingEvent. It
 * handles the 'X' conversion character.
 *
 * \sa PatternLayout::format()
 * \sa PatternConverter::format()
 */
class MDCPatternConverter : public PatternConverter
{
public:
    MDCPatternConverter(Log4Qt::FormattingInfo formattingInfo,
                        const QString &key) :
        PatternConverter(formattingInfo),
        mKey(key)
    {}

private:
    Q_DISABLE_COPY(MDCPatternConverter)

protected:
    QString convert(const LoggingEvent &loggingEvent) const override;

private:
    QString mKey;
};

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::PatternFormatter)

PatternFormatter::PatternFormatter(const QString &pattern) :
    mIgnoreCharacters(QStringLiteral("C")),
    mConversionCharacters(QStringLiteral("cdmprtxXFMLl")),
    mOptionCharacters(QStringLiteral("cd")),
    mPattern(pattern)
{
    parse();
}


PatternFormatter::~PatternFormatter()
{
    qDeleteAll(mPatternConverters);
}


QString PatternFormatter::format(const LoggingEvent &loggingEvent) const
{
    QString result;
    for (auto &&p_converter : qAsConst(mPatternConverters))
        p_converter->format(result, loggingEvent);
    return result;
}


bool PatternFormatter::addDigit(QChar digit,
                                int &value)
{
    if (!digit.isDigit())
        return false;

    int digit_value = digit.digitValue();
    if (value > (INT_MAX - digit_value) / 10)
        value = INT_MAX;
    else
        value = value * 10 + digit_value;
    return true;
}


void PatternFormatter::createConverter(QChar character,
                                       Log4Qt::FormattingInfo formattingInfo,
                                       const QString &option)
{
    Q_ASSERT_X(mConversionCharacters.indexOf(character) >= 0, "PatternFormatter::createConverter", "Unknown conversion character" );

    LogError e("Creating Converter for character '%1' min %2, max %3, left %4 and option '%5'");
    e << QString(character)
      << FormattingInfo::intToString(formattingInfo.mMinLength)
      << FormattingInfo::intToString(formattingInfo.mMaxLength)
      << formattingInfo.mLeftAligned
      << option;
    logger()->trace(e);

    switch (character.toLatin1())
    {
    case 'c':
        mPatternConverters << new LoggepatternConverter(formattingInfo,
                           parseIntegeoption(option));
        break;
    case 'd':
    {
        QString format = option;
        if (option.isEmpty())
            format = QStringLiteral("ISO8601");
        else if (option == QLatin1String("locale:long"))
            format = QLocale().dateTimeFormat(QLocale::LongFormat);
        else if (option == QLatin1String("locale:short"))
            format = QLocale().dateTimeFormat(QLocale::ShortFormat);
        else if (option == QLatin1String("locale:narrow"))
            format = QLocale().dateTimeFormat(QLocale::NarrowFormat);
        else if (option == QLatin1String("locale"))
            format = QLocale().dateTimeFormat(QLocale::ShortFormat);
        mPatternConverters << new DatePatternConverter(formattingInfo,
                                                       format);
        break;
    }
    case 'm':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::MESSAGE_CONVERTER);
        break;
    case 'p':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::LEVEL_CONVERTER);
        break;
    case 'r':
        mPatternConverters << new DatePatternConverter(formattingInfo,
                                                       QStringLiteral("RELATIVE"));
        break;
    case 't':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::THREAD_CONVERTER);
        break;
    case 'x':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::NDC_CONVERTER);
        break;
    case 'X':
        mPatternConverters << new MDCPatternConverter(formattingInfo,
                                                      option);
        break;
    case 'F':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::FILENAME_CONVERTER);
        break;
    case 'M':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::FUNCTIONNAME_CONVERTER);
        break;
    case 'L':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::LINENUMBER_CONVERTER);
        break;
    case 'l':
        mPatternConverters << new BasicPatternConverter(formattingInfo,
                                                        BasicPatternConverter::LOCATION_CONVERTER);
        break;
    default:
        Q_ASSERT_X(false, "PatternFormatter::createConverter", "Unknown pattern character");
    }
}


void PatternFormatter::createLiteralConverter(const QString &literal)
{
    logger()->trace(QStringLiteral("Creating literal LiteralConverter with Literal '%1'"),
                    literal);
    mPatternConverters << new LiteralPatternConverter(literal);
}


void PatternFormatter::parse()
{
    enum State
    {
        LITERAL_STATE,
        ESCAPE_STATE,
        MIN_STATE,
        DOT_STATE,
        MAX_STATE,
        CHARACTER_STATE,
        POSSIBLEOPTION_STATE,
        OPTION_STATE
    };

    int i = 0;
    QChar c;
    char ch;
    State state = LITERAL_STATE;
    FormattingInfo formatting_info;
    QString literal;
    int converter_start = 0;
    int option_start = 0;
    while (i < mPattern.length())
    {
        // i points to the current character
        // c contains the current character
        // ch contains the Latin1 equivalent of the current character
        // i is incremented at the end of the loop to consume the character
        // continue is used to change state without consuming the character

        c = mPattern.at(i);
        ch = c.toLatin1();
        switch (state)
        {
        case LITERAL_STATE:
            if (ch == '%')
            {
                formatting_info.clear();
                converter_start = i;
                state = ESCAPE_STATE;
            }
            else
                literal += c;
            break;
        case ESCAPE_STATE:
            if (ch == '%')
            {
                literal += c;
                state = LITERAL_STATE;
            }
            else if (ch == 'n')
            {
                literal += Layout::endOfLine();
                state = LITERAL_STATE;
            }
            else
            {
                if (!literal.isEmpty())
                {
                    createLiteralConverter(literal);
                    literal.clear();
                }
                if (ch == '-')
                    formatting_info.mLeftAligned = true;
                else if (c.isDigit())
                {
                    formatting_info.mMinLength = c.digitValue();
                    state = MIN_STATE;
                }
                else if (ch == '.')
                    state = DOT_STATE;
                else
                {
                    state = CHARACTER_STATE;
                    continue;
                }
            }
            break;
        case MIN_STATE:
            if (!addDigit(c, formatting_info.mMinLength))
            {
                if (ch == '.')
                    state = DOT_STATE;
                else
                {
                    state = CHARACTER_STATE;
                    continue;
                }
            }
            break;
        case DOT_STATE:
            if (c.isDigit())
            {
                formatting_info.mMaxLength = c.digitValue();
                state = MAX_STATE;
            }
            else
            {
                LogError e = LOG4QT_ERROR(QT_TR_NOOP("Found character '%1' where digit was expected."),
                                          LAYOUT_EXPECTED_DIGIT_ERROR,
                                          "Log4Qt::PatternFormatter");
                e << QString(c);
                logger()->error(e);
            }
            break;
        case MAX_STATE:
            if (!addDigit(c, formatting_info.mMaxLength))
            {
                state = CHARACTER_STATE;
                continue;
            }
            break;
        case CHARACTER_STATE:
            if (mIgnoreCharacters.indexOf(c) >= 0)
                state = LITERAL_STATE;
            else if (mOptionCharacters.indexOf(c) >= 0)
                state = POSSIBLEOPTION_STATE;
            else if (mConversionCharacters.indexOf(c) >= 0)
            {
                createConverter(c, formatting_info);
                state = LITERAL_STATE;
            }
            else
            {
                logger()->warn(QStringLiteral("Invalid conversion character '%1' at %2 in pattern '%3'"),
                               c, i, mPattern);
                createLiteralConverter(mPattern.mid(converter_start, i - converter_start + 1));
                state = LITERAL_STATE;
            }
            break;
        case POSSIBLEOPTION_STATE:
            if (ch == '{')
            {
                option_start = i;
                state = OPTION_STATE;
            }
            else
            {
                createConverter(mPattern.at(i - 1),
                                formatting_info);
                state = LITERAL_STATE;
                continue;
            }
            break;
        case OPTION_STATE:
            if (ch == '}')
            {
                createConverter(mPattern.at(option_start - 1),
                                formatting_info,
                                mPattern.mid(option_start + 1, i - option_start - 1));
                state = LITERAL_STATE;
            }
            break;
        default:
            Q_ASSERT_X(false, "PatternFormatter::parse()", "Unknown parsing state constant");
            state = LITERAL_STATE;
        }
        i++;
    }

    if (state != LITERAL_STATE)
    {
        logger()->warn(QStringLiteral("Unexptected end of pattern '%1'"), mPattern);
        if (state == ESCAPE_STATE)
            literal += c;
        else
            literal += mPattern.midRef(converter_start);
    }

    if (!literal.isEmpty())
        createLiteralConverter(literal);
}


int PatternFormatter::parseIntegeoption(const QString &option)
{
    if (option.isEmpty())
        return 0;

    bool ok;
    int result = option.toInt(&ok);
    if (!ok)
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Option '%1' cannot be converted into an integer"),
                                  LAYOUT_OPTION_IS_NOT_INTEGER_ERROR,
                                  "Log4Qt::Patteformatter");
        e << option;
        logger()->error(e);
    }
    if (result < 0)
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Option %1 isn't a positive integer"),
                                  LAYOUT_INTEGER_IS_NOT_POSITIVE_ERROR,
                                  "Log4Qt::Patteformatter");
        e << result;
        logger()->error(e);
        result = 0;
    }
    return result;
}

void FormattingInfo::clear()
{
    mMinLength = 0;
    mMaxLength = INT_MAX;
    mLeftAligned = false;
}


QString FormattingInfo::intToString(int i)
{
    if (i == INT_MAX)
        return QStringLiteral("INT_MAX");
    return QString::number(i);
}

void PatternConverter::format(QString &format, const LoggingEvent &loggingEvent) const
{
    Q_DECL_CONSTEXPR const QLatin1Char space(' ');
    const QString s = convert(loggingEvent);

    // If the data item is longer than the maximum field, then the extra characters
    // are removed from the beginning of the data item and not from the end.
    if (s.length() > mFormattingInfo.mMaxLength)
        format += s.rightRef(mFormattingInfo.mMaxLength);
    else if (mFormattingInfo.mLeftAligned)
        format += s.leftJustified(mFormattingInfo.mMinLength, space, false);
    else
        format += s.rightJustified(mFormattingInfo.mMinLength, space, false);
}

QString BasicPatternConverter::convert(const LoggingEvent &loggingEvent) const
{
    switch (mType)
    {
    case MESSAGE_CONVERTER:
        return loggingEvent.message();
    case NDC_CONVERTER:
        return loggingEvent.ndc();
    case LEVEL_CONVERTER:
        return loggingEvent.level().toString();
    case THREAD_CONVERTER:
        return loggingEvent.threadName();
    case FILENAME_CONVERTER:
        return loggingEvent.context().file;
    case LINENUMBER_CONVERTER:
        return QString::number(loggingEvent.context().line);
    case FUNCTIONNAME_CONVERTER:
        return loggingEvent.context().function;
    case LOCATION_CONVERTER:
        return QStringLiteral("%1:%2 - %3").arg(loggingEvent.context().file, QString::number(loggingEvent.context().line), loggingEvent.context().function);
    case CATEGORYNAME_CONVERTER:
        return loggingEvent.categoryName();
    default:
        Q_ASSERT_X(false, "BasicPatternConverter::convert()", "Unknown type constant");
        return QString();
    }
}

QString DatePatternConverter::convert(const LoggingEvent &loggingEvent) const
{
    return DateTime::fromMSecsSinceEpoch(loggingEvent.timeStamp()).toString(mFormat);
}

QString LiteralPatternConverter::convert(const LoggingEvent &loggingEvent) const
{
    Q_UNUSED(loggingEvent);
    return mLiteral;
}

QString LoggepatternConverter::convert(const LoggingEvent &loggingEvent) const
{
    if (!loggingEvent.logger())
        return QString();
    QString name;

    if (loggingEvent.logger() == LogManager::instance()->qtLogger())   // is qt logger
        if (loggingEvent.categoryName().isEmpty())
            name = LogManager::instance()->qtLogger()->name();
        else
            name = loggingEvent.categoryName();
    else
        name = loggingEvent.logger()->name();

    if (mPrecision <= 0 || (name.isEmpty()))
        return name;

    const QString separator(QStringLiteral("::"));

    int i = mPrecision;
    int begin = name.length();
    while ((i > 0) && (begin >= 0))
    {
        begin = name.lastIndexOf(separator, begin - name.length() - 1);
        i--;
    }
    if (begin < 0)
        begin = 0;
    else
        begin += 2;
    return name.mid(begin);
}

QString MDCPatternConverter::convert(const LoggingEvent &loggingEvent) const
{
    return loggingEvent.mdc().value(mKey);
}

} // namespace Log4Qt
