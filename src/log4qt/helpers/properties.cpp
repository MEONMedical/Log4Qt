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

#include "helpers/properties.h"

#include "logger.h"

#include <QIODevice>
#include <QSettings>
#include <QTextStream>

#include <utility>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::Properties)

void Properties::load(QIODevice *pDevice)
{
    const QLatin1Char append_char(msEscapeChar);

    if (pDevice == nullptr)
    {
        logger()->warn(u"No device specified for load."_s);
        return;
    }

    QTextStream stream(pDevice);
    QString line;
    int line_number = 0;
    QString property;
    int property_start_line = 1;

    // A line continues on the next one only if it ends in an ODD number of
    // escape characters — a trailing escaped backslash ('...\\') is data,
    // not a continuation.
    auto endsWithContinuation = [append_char](const QString &l)
    {
        int escapes = 0;
        for (int pos = l.size() - 1; pos >= 0 && l.at(pos) == append_char; --pos)
            ++escapes;
        return (escapes % 2) == 1;
    };

    do
    {
        line = trimLeft(stream.readLine());
        line_number++;

        // Comment lines are never continued: a '#'/'!' line ending in the
        // escape character must not swallow the following property line.
        const bool isCommentLine = property.isEmpty() && !line.isEmpty()
            && (line.at(0) == QLatin1Char('#') || line.at(0) == QLatin1Char('!'));

        if (!isCommentLine && !line.isEmpty() && endsWithContinuation(line))
            property += line.left(line.size() - 1);
        else
        {
            property += line;
            parseProperty(property, property_start_line);
            property.clear();
            property_start_line = line_number + 1;
        }
    }
    while (!line.isNull());
}


void Properties::load(const QSettings &settings)
{
    QStringList keys = settings.childKeys();
    for (const auto &key : std::as_const(keys))
        insert(key, settings.value(key).toString());
}


QString Properties::property(const QString &key) const
{
    // Null string indicates the property does not contain the key.

    if (contains(key))
    {
        QString value = this->value(key);
        if (value.isNull())
            return u""_s;
        return value;
    }

    if (mpDefaultProperties)
        return mpDefaultProperties->property(key);
    return {};
}


QString Properties::property(const QString &key,
                             const QString &defaultValue) const
{
    QString value = property(key);
    if (value.isNull())
        return defaultValue;
    return value;
}

QStringList Properties::propertyNames() const
{
    QStringList default_keys;
    if (mpDefaultProperties)
        default_keys = mpDefaultProperties->propertyNames();

    QStringList keys = this->keys();
    for (const auto &key : std::as_const(default_keys))
        if (!keys.contains(key))
            keys << key;

    return keys;
}


void Properties::parseProperty(const QString &property,
                               int line)
{
    Q_ASSERT_X(property == trimLeft(property), "parseProperty()", "property has leading spaces");

    enum State
    {
        KeyState,
        KeySpaceState,
        SpaceValueState,
        ValueState,
        KeyEscapeState,
        ValueEscapeState,
        UnicodeEscapeState
    };
    const QString value_escape_codes = QLatin1String(msValueEscapeCodes);
    const QString value_escape_chars = QLatin1String(msValueEscapeChars);
    Q_ASSERT_X(value_escape_codes.size() == value_escape_chars.size(), "parseProperty()", "Value escape sequence character definition does not map");
    const QString key_escape_codes = QLatin1String(msKeyEscapeCodes);
    const QString key_escape_chars = QLatin1String(msKeyEscapeChars);
    Q_ASSERT_X(key_escape_codes.size() == key_escape_chars.size(), "parseProperty()", "Key escape sequence character definition does not map");

    if (property.isEmpty())
        return;

    int i = 0;
    QChar c;
    State state = KeyState;
    QString key;
    QString value;
    QString *p_string = &key;
    uint ucs = 0;
    int ucs_digits = 0;
    while (i < property.size())
    {
        // i points to the current character.
        // c contains the current character
        // ch contains the Latin1 equivalent of the current character
        // i is incremented at the end of the loop to consume the character.
        // continue is used to change state without consuming the character
        char ch;
        c = property.at(i);
        ch = c.toLatin1();

        switch (state)
        {
        case KeyState:
            if (ch == '!' || ch == '#' )
                return;
            else if (c.isSpace())
            {
                p_string = &value;
                state = KeySpaceState;
            }
            else if (ch == '=' || ch == ':')
            {
                p_string = &value;
                state = SpaceValueState;
            }
            else if (ch == msEscapeChar)
                state = KeyEscapeState;
            else
                *p_string += c;
            break;
        case KeySpaceState:
            if (ch == '=' || ch == ':')
                state = SpaceValueState;
            else if (!c.isSpace())
            {
                *p_string += c;
                state = ValueState;
            }
            break;
        case SpaceValueState:
            if (!c.isSpace())
            {
                *p_string += c;
                state = ValueState;
            }
            break;
        case ValueState:
            if (ch == msEscapeChar)
                state = ValueEscapeState;
            else
                *p_string += c;
            break;
        case KeyEscapeState:
        {
            int convert = key_escape_codes.indexOf(c);
            if (convert >= 0)
                *p_string += key_escape_chars.at(convert);
            else
            {
                logger()->warn(u"Unknown escape sequence '\\%1' in key of property starting at line %2"_s,
                               QString(c),
                               line);
                *p_string += c;
            }
            state = KeyState;
            break;
        }
        case ValueEscapeState:
        {
            int convert = value_escape_codes.indexOf(c);
            if (convert >= 0)
            {
                *p_string += value_escape_chars.at(convert);
                state = ValueState;
            }
            else if (ch == 'u')
            {
                ucs = 0;
                ucs_digits = 0;
                state = UnicodeEscapeState;
            }
            else
            {
                logger()->warn(u"Unknown escape sequence '\\%1' in value of property starting at line %2"_s, QString(c), line);
                *p_string += c;
                state = ValueState;
            }
            break;
        }
        case UnicodeEscapeState:
        {
            int hex = hexDigitValue(c);
            if (hex >= 0)
            {
                ucs = ucs * 16 + hex;
                ucs_digits++;
                if (ucs_digits == 4 || i == property.size() - 1)
                {
                    *p_string += QChar(ucs);
                    state = ValueState;
                }
            }
            else
            {
                if (ucs_digits > 0)
                    *p_string += QChar(ucs);
                state = ValueState;
                continue;
            }
            break;
        }
        default:
            Q_ASSERT_X(false, "Properties::parseProperty()", "Unknown state constant");
            return;
        }
        i++;
    }

    if (key.isEmpty() && !value.isEmpty())
        logger()->warn(u"Found value with no key in property starting at line %1"_s, line);

    logger()->trace(u"Loaded property '%1' : '%2'"_s, key, value);
    insert(key, value);
}

int Properties::hexDigitValue(QChar digit)
{
    bool ok;
    int result = QString(digit).toInt(&ok, 16);
    if (!ok)
        return -1;

    return result;
}

QString Properties::trimLeft(const QString &line)
{
    int i = 0;
    while (i < line.size() && line.at(i).isSpace())
        i++;
    return line.right(line.size() - i);
}

} // namespace Log4Qt
