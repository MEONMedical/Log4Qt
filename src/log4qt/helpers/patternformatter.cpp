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

#include "helpers/patternformatter.h"

#include "helpers/datetime.h"
#include "helpers/logerror.h"
#include "abstractlayout.h"
#include "logger.h"
#include "loggingevent.h"
#include "logmanager.h"

#include <QString>
#include <QStringBuilder>

#include <utility>

using namespace Qt::StringLiterals;

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
    Q_DISABLE_COPY_MOVE(PatternConverter)

public:
    void format(QString &format, const LoggingEvent &loggingEvent) const;

protected:
    virtual void convert(QString &format, const LoggingEvent &loggingEvent) const = 0;

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
        MessageConverter,
        NdcConverter,
        LevelConverter,
        ThreadConverter,
        FilenameConverter,
        FunctionNameConverter,
        LineNumberConverter,
        LocationConverter,
        CategoryNameConverter
    };

public:
    BasicPatternConverter(Log4Qt::FormattingInfo formattingInfo,
                          Type type) :
        PatternConverter(formattingInfo),
        mType(type)
    {}

private:
    Q_DISABLE_COPY_MOVE(BasicPatternConverter)

protected:
    void convert(QString &format, const LoggingEvent &loggingEvent) const override;

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
    Q_DISABLE_COPY_MOVE(DatePatternConverter)

protected:
    void convert(QString &format, const LoggingEvent &loggingEvent) const override;

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
    Q_DISABLE_COPY_MOVE(LiteralPatternConverter)

protected:
    void convert(QString &format, const LoggingEvent &loggingEvent) const override;

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
    Q_DISABLE_COPY_MOVE(LoggepatternConverter)

protected:
    void convert(QString &format, const LoggingEvent &loggingEvent) const override;

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
    Q_DISABLE_COPY_MOVE(MDCPatternConverter)

protected:
    void convert(QString &format, const LoggingEvent &loggingEvent) const override;

private:
    QString mKey;
};


/*!
 * \brief The class PropertyPatternConverter resolves named properties via
 *        the QObject property system.
 *
 * Handles the \c %P{key} conversion character. At format time it calls
 * \c (*mSourceRef)->property(mKey).toString() on the property-source object
 * registered with the owning PatternFormatter. Both static Q_PROPERTY
 * members and dynamic properties (set via QObject::setProperty()) are
 * resolved.
 *
 * The converter holds a \c const QObject* const* — a pointer to the
 * \c mPropertySource member of the \c PatternFormatter that created it.
 * Because \c PatternFormatter is \c Q_DISABLE_COPY_MOVE this address is
 * stable for the formatter's lifetime, so calling \c setPropertySource()
 * after construction is reflected immediately on the next \c format() call.
 *
 * \sa PatternFormatter::setPropertySource()
 */
class PropertyPatternConverter : public PatternConverter
{
public:
    PropertyPatternConverter(Log4Qt::FormattingInfo formattingInfo,
                             const QByteArray &key,
                             const QObject * const *sourceRef)
        : PatternConverter(formattingInfo),
          mKey(key),
          mSourceRef(sourceRef)
    {}

private:
    Q_DISABLE_COPY_MOVE(PropertyPatternConverter)

protected:
    void convert(QString &format, [[maybe_unused]] const LoggingEvent &loggingEvent) const override
    {
        if (mSourceRef && *mSourceRef)
            format.append((*mSourceRef)->property(mKey).toString());
    }

private:
    QByteArray mKey;
    const QObject * const *mSourceRef;
};

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::PatternFormatter)

PatternFormatter::PatternFormatter(const QString &pattern) :
    mIgnoreCharacters(u"C"_s),
    mConversionCharacters(u"cdmprtxXFMLlP"_s),
    // Conversion characters that may be followed by a {option}. 'X' belongs
    // here: without it the parser creates the MDC converter immediately and
    // the '{key}' that follows is emitted as literal text.
    mOptionCharacters(u"cdPX"_s),
    mPattern(pattern)
{
    parse();
}


PatternFormatter::~PatternFormatter() = default;


QString PatternFormatter::format(const LoggingEvent &loggingEvent) const
{
    QString result;
    // Optimize: reserve capacity to avoid reallocations during string building
    result.reserve(256);
    for (auto &&p_converter : std::as_const(mPatternConverters))
        p_converter->format(result, loggingEvent);
    return result;
}


bool PatternFormatter::requiresLocation() const
{
    return mRequiresLocation;
}


bool PatternFormatter::addDigit(QChar digit,
                                int &value) const
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
        mPatternConverters.push_back(std::make_unique<LoggepatternConverter>(formattingInfo,
                           parseIntegerOption(option)));
        break;
    case 'd':
    {
        QString format = option;
        if (option.isEmpty())
            format = u"ISO8601"_s;
        else if (option == QLatin1String("locale:long"))
            format = QLocale().dateTimeFormat(QLocale::LongFormat);
        else if (option == QLatin1String("locale:short"))
            format = QLocale().dateTimeFormat(QLocale::ShortFormat);
        else if (option == QLatin1String("locale:narrow"))
            format = QLocale().dateTimeFormat(QLocale::NarrowFormat);
        else if (option == QLatin1String("locale"))
            format = QLocale().dateTimeFormat(QLocale::ShortFormat);
        mPatternConverters.push_back(std::make_unique<DatePatternConverter>(formattingInfo,
                                                       format));
        break;
    }
    case 'm':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::MessageConverter));
        break;
    case 'p':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::LevelConverter));
        break;
    case 'r':
        mPatternConverters.push_back(std::make_unique<DatePatternConverter>(formattingInfo,
                                                       u"RELATIVE"_s));
        break;
    case 't':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::ThreadConverter));
        break;
    case 'x':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::NdcConverter));
        break;
    case 'X':
        mPatternConverters.push_back(std::make_unique<MDCPatternConverter>(formattingInfo,
                                                      option));
        break;
    case 'P':
        if (option.isEmpty())
            logger()->warn(u"%%P requires a property key in braces (e.g. %%P{key}) in pattern '%1'"_s, mPattern);
        mPatternConverters.push_back(
            std::make_unique<PropertyPatternConverter>(
                formattingInfo, option.toLatin1(), &mPropertySource));
        break;
    case 'F':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::FilenameConverter));
        mRequiresLocation = true;
        break;
    case 'M':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::FunctionNameConverter));
        mRequiresLocation = true;
        break;
    case 'L':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::LineNumberConverter));
        mRequiresLocation = true;
        break;
    case 'l':
        mPatternConverters.push_back(std::make_unique<BasicPatternConverter>(formattingInfo,
                                                        BasicPatternConverter::LocationConverter));
        mRequiresLocation = true;
        break;
    default:
        Q_ASSERT_X(false, "PatternFormatter::createConverter", "Unknown pattern character");
    }
}


void PatternFormatter::createLiteralConverter(const QString &literal)
{
    logger()->trace(u"Creating literal LiteralConverter with Literal '%1'"_s,
                    literal);
    mPatternConverters.push_back(std::make_unique<LiteralPatternConverter>(literal));
}


void PatternFormatter::parse()
{
    enum State
    {
        LiteralState,
        EscapeState,
        MinState,
        DotState,
        MaxState,
        CharacterState,
        PossibleOptionState,
        OptionState
    };

    int i = 0;
    QChar c;
    char ch;
    State state = LiteralState;
    FormattingInfo formatting_info;
    QString literal;
    int converter_start = 0;
    int option_start = 0;
    while (i < mPattern.size())
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
        case LiteralState:
            if (ch == '%')
            {
                formatting_info.clear();
                converter_start = i;
                state = EscapeState;
            }
            else
                literal += c;
            break;
        case EscapeState:
            if (ch == '%')
            {
                literal += c;
                state = LiteralState;
            }
            else if (ch == 'n')
            {
                literal += AbstractLayout::endOfLine();
                state = LiteralState;
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
                    state = MinState;
                }
                else if (ch == '.')
                    state = DotState;
                else
                {
                    state = CharacterState;
                    continue;
                }
            }
            break;
        case MinState:
            if (!addDigit(c, formatting_info.mMinLength))
            {
                if (ch == '.')
                    state = DotState;
                else
                {
                    state = CharacterState;
                    continue;
                }
            }
            break;
        case DotState:
            if (c.isDigit())
            {
                formatting_info.mMaxLength = c.digitValue();
                state = MaxState;
            }
            else
            {
                LogError e = LOG4QT_ERROR("Found character '%1' where digit was expected.",
                                          LayoutExpectedDigitError,
                                          "Log4Qt::PatternFormatter");
                e << QString(c);
                logger()->error(e);
            }
            break;
        case MaxState:
            if (!addDigit(c, formatting_info.mMaxLength))
            {
                state = CharacterState;
                continue;
            }
            break;
        case CharacterState:
            if (mIgnoreCharacters.indexOf(c) >= 0)
                state = LiteralState;
            else if (mOptionCharacters.indexOf(c) >= 0)
                state = PossibleOptionState;
            else if (mConversionCharacters.indexOf(c) >= 0)
            {
                createConverter(c, formatting_info);
                state = LiteralState;
            }
            else
            {
                logger()->warn(u"Invalid conversion character '%1' at %2 in pattern '%3'"_s,
                               c, i, mPattern);
                createLiteralConverter(mPattern.mid(converter_start, i - converter_start + 1));
                state = LiteralState;
            }
            break;
        case PossibleOptionState:
            if (ch == '{')
            {
                option_start = i;
                state = OptionState;
            }
            else
            {
                createConverter(mPattern.at(i - 1),
                                formatting_info);
                state = LiteralState;
                continue;
            }
            break;
        case OptionState:
            if (ch == '}')
            {
                createConverter(mPattern.at(option_start - 1),
                                formatting_info,
                                mPattern.mid(option_start + 1, i - option_start - 1));
                state = LiteralState;
            }
            break;
        default:
            Q_ASSERT_X(false, "PatternFormatter::parse()", "Unknown parsing state constant");
            state = LiteralState;
        }
        i++;
    }

    if (state == PossibleOptionState)
    {
        // The pattern ended directly after an option-capable conversion
        // character (%c, %d, %P): no '{option}' follows, so create the
        // converter with default options instead of treating the tail as an
        // error and emitting it as literal text.
        createConverter(mPattern.at(i - 1), formatting_info);
    }
    else if (state != LiteralState)
    {
        logger()->warn(u"Unexpected end of pattern '%1'"_s, mPattern);
        if (state == EscapeState)
            literal += c;
        else
            literal += mPattern.mid(converter_start);
    }

    if (!literal.isEmpty())
        createLiteralConverter(literal);
}


int PatternFormatter::parseIntegerOption(QStringView option)
{
    if (option.isEmpty())
        return 0;

    bool ok;
    int result = option.toInt(&ok);
    if (!ok)
    {
        LogError e = LOG4QT_ERROR("Option '%1' cannot be converted into an integer",
                                  LayoutOptionIsNotIntegerError,
                                  "Log4Qt::PatternFormatter");
        e << option.toString();
        logger()->error(e);
    }
    if (result < 0)
    {
        LogError e = LOG4QT_ERROR("Option %1 isn't a positive integer",
                                  LayoutIntegerIsNotPositiveError,
                                  "Log4Qt::PatternFormatter");
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
        return u"INT_MAX"_s;
    return QString::number(i);
}

void PatternConverter::format(QString &format, const LoggingEvent &loggingEvent) const
{
    // Optimization: If no complex formatting needed, write directly
    if (mFormattingInfo.mMinLength == 0 && mFormattingInfo.mMaxLength == INT_MAX)
    {
        convert(format, loggingEvent);
        return;
    }

    QString s;
    s.reserve(64);
    convert(s, loggingEvent);

    constexpr QLatin1Char space(' ');

    // If the data item is longer than the maximum field, then the extra characters
    // are removed from the beginning of the data item and not from the end.
    if (s.size() > mFormattingInfo.mMaxLength)
        format.append(s.right(mFormattingInfo.mMaxLength));
    else if (mFormattingInfo.mLeftAligned)
        format.append(s.leftJustified(mFormattingInfo.mMinLength, space, false));
    else
        format.append(s.rightJustified(mFormattingInfo.mMinLength, space, false));
}

void BasicPatternConverter::convert(QString &format, const LoggingEvent &loggingEvent) const
{
    switch (mType)
    {
    case MessageConverter:
        format.append(loggingEvent.message());
        break;
    case NdcConverter:
        format.append(loggingEvent.ndc());
        break;
    case LevelConverter:
        format.append(loggingEvent.level().toString());
        break;
    case ThreadConverter:
        format.append(loggingEvent.threadName());
        break;
    case FilenameConverter:
        if (loggingEvent.context().file)
            format.append(loggingEvent.context().file);
        break;
    case LineNumberConverter:
        format.append(QString::number(loggingEvent.context().line));
        break;
    case FunctionNameConverter:
        if (loggingEvent.context().function)
            format.append(loggingEvent.context().function);
        break;
    case LocationConverter:
    {
        const auto &ctx = loggingEvent.context();
        // QStringBuilder avoids the per-event arg()-parser and produces a
        // single allocation; QLatin1String wraps the stable __FILE__ /
        // Q_FUNC_INFO C-strings without a UTF-8 conversion pass.
        format += QLatin1String(ctx.file ? ctx.file : "")
                  % QLatin1Char(':') % QString::number(ctx.line)
                  % QLatin1String(" - ")
                  % QLatin1String(ctx.function ? ctx.function : "");
        break;
    }
    case CategoryNameConverter:
        format.append(loggingEvent.categoryName());
        break;
    default:
        Q_ASSERT_X(false, "BasicPatternConverter::convert()", "Unknown type constant");
    }
}

void DatePatternConverter::convert(QString &format, const LoggingEvent &loggingEvent) const
{
    format.append(DateTime::formatMsecs(loggingEvent.timeStamp(), mFormat));
}

void LiteralPatternConverter::convert(QString &format, [[maybe_unused]] const LoggingEvent &loggingEvent) const
{
    format.append(mLiteral);
}

void LoggepatternConverter::convert(QString &format, const LoggingEvent &loggingEvent) const
{
    if (!loggingEvent.logger())
        return;
    QString name;

    if (loggingEvent.logger() == LogManager::instance()->qtLogger())   // is qt logger
        if (loggingEvent.categoryName().isEmpty())
            name = LogManager::instance()->qtLogger()->name();
        else
            name = loggingEvent.categoryName();
    else
        name = loggingEvent.logger()->name();

    if (mPrecision <= 0 || (name.isEmpty()))
    {
        format.append(name);
        return;
    }

    const QString separator(u"::"_s);

    int i = mPrecision;
    int begin = name.size();
    while ((i > 0) && (begin >= 0))
    {
        begin = name.lastIndexOf(separator, begin - name.size() - 1);
        i--;
    }
    if (begin < 0)
        begin = 0;
    else
        begin += 2;
    format.append(name.mid(begin));
}

void MDCPatternConverter::convert(QString &format, const LoggingEvent &loggingEvent) const
{
    if (!mKey.isEmpty())
    {
        format.append(loggingEvent.mdc().value(mKey));
        return;
    }

    // A bare %X renders the whole MDC as '{key=value, key=value}', the way
    // log4j does. The keys are sorted so the output does not depend on the
    // hash order.
    const QHash<QString, QString> mdc = loggingEvent.mdc();
    QStringList keys = mdc.keys();
    keys.sort();

    QStringList entries;
    entries.reserve(keys.size());
    for (const QString &key : std::as_const(keys))
        entries.append(key % u'=' % mdc.value(key));

    format.append(u'{' % entries.join(u", "_s) % u'}');
}

void PatternFormatter::setPropertySource(const QObject *source)
{
    mPropertySource = source;
}

} // namespace Log4Qt
