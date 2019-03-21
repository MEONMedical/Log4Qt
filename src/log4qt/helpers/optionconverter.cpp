/******************************************************************************
 *
 * package:     Log4Qt
 * file:        optionconverter.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes      Feb 2009, Martin Heinrich
 *              - Fixed a problem were OptionConverter::toBoolean would not
 *                return the default value, if the conversion fails.
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

#include "helpers/optionconverter.h"

#include "helpers/logerror.h"
#include "helpers/properties.h"
#include "logger.h"
#include "consoleappender.h"

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt::OptionConverter)

QString OptionConverter::findAndSubst(const Properties &properties,
                                      const QString &key)
{
    QString value = properties.property(key);
    if (value.isNull())
        return value;

    const QString begin_subst = QStringLiteral("${");
    const QString end_subst = QStringLiteral("}");
    const int begin_length = begin_subst.length();
    const int end_length = end_subst.length();

    // Don't return a null string, the null string indicates that the
    // property key does not exist.
    QString result = QLatin1String("");

    int i = 0;
    int begin;
    int end;
    while (i < value.length())
    {
        begin = value.indexOf(begin_subst, i);
        if (begin == -1)
        {
            result += value.midRef(i);
            i = value.length();
        }
        else
        {
            result += value.midRef(i, begin - i);
            end = value.indexOf(end_subst, i + begin_length);
            if (end == -1)
            {
                LogError e = LOG4QT_ERROR(QT_TR_NOOP("Missing closing bracket for opening bracket at %1. Invalid subsitution in value %2."),
                                          CONFIGURATOR_INVALID_SUBSTITUTION_ERROR,
                                          "Log4Qt::OptionConverter");
                e << begin << value;
                logger()->error(e);
                return result;
            }
            auto keyName = value.mid(begin + begin_length, end - begin - end_length - 1);
            auto subValue = findAndSubst(properties, keyName);
            if (subValue.isNull() && keyName.startsWith(QLatin1String("LOG4QT_")))
                subValue = qgetenv(qPrintable(keyName));
            result +=subValue;
            i = end + end_length;
        }
    }
    return result;
}

QString OptionConverter::classNameJavaToCpp(const QString &className)
{
    const QLatin1Char java_class_delimiter('.');
    const QLatin1String cpp_class_delimiter("::");

    QString result = className;
    return result.replace(java_class_delimiter, cpp_class_delimiter);
}

bool OptionConverter::toBoolean(const QString &option,
                                bool *ok)
{
    const QLatin1String str_true("true");
    const QLatin1String str_enabled("enabled");
    const QLatin1String str_one("1");
    const QLatin1String str_false("false");
    const QLatin1String str_disabled("disabled");
    const QLatin1String str_zero("0");

    if (ok)
        *ok = true;
    QString s = option.trimmed().toLower();
    if (s == str_true || s == str_enabled || s == str_one)
        return true;
    if (s == str_false || s == str_disabled || s == str_zero)
        return false;

    if (ok)
        *ok = false;
    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Invalid option string '%1' for a boolean"),
                              CONFIGURATOR_INVALID_OPTION_ERROR,
                              "Log4Qt::OptionConverter");
    e << option;
    logger()->error(e);
    return false;
}

bool OptionConverter::toBoolean(const QString &option,
                                bool defaultValue)
{
    bool ok;
    bool result = toBoolean(option, &ok);
    if (ok)
        return result;

    return defaultValue;
}

qint64 OptionConverter::toFileSize(const QString &option,
                                   bool *ok)
{
    // - Search for unit
    // - Convert characters befor unit to int
    // - Error, if
    //   - the conversion failed
    //   - the value < 0
    //   - there is text after the unit characters

    if (ok != nullptr)
        *ok = false;
    QString s = option.trimmed().toLower();
    qint64 f = 1;
    int i;
    i = s.indexOf(QStringLiteral("kb"));
    if (i >= 0)
        f = 1024;
    else
    {
        i = s.indexOf(QStringLiteral("mb"));
        if (i >= 0)
            f = 1024 * 1024;
        else
        {
            i = s.indexOf(QStringLiteral("gb"));
            if (i >= 0)
                f = 1024 * 1024 * 1024;
        }
    }
    if (i < 0)
        i = s.length();
    bool convertOk;
    qint64 value = s.left(i).toLongLong(&convertOk);
    if (!convertOk || value < 0 || s.length() > i + 2)
    {
        LogError e = LOG4QT_ERROR(QT_TR_NOOP("Invalid option string '%1' for a file size"),
                                  CONFIGURATOR_INVALID_OPTION_ERROR,
                                  "Log4Qt::OptionConverter");
        e << option;
        logger()->error(e);
        return 0;
    }
    if (ok != nullptr)
        *ok = true;
    return value * f;
}

int OptionConverter::toInt(const QString &option,
                           bool *ok)
{
    int value = option.trimmed().toInt(ok);
    if (*ok)
        return value;

    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Invalid option string '%1' for an integer"),
                              CONFIGURATOR_INVALID_OPTION_ERROR,
                              "Log4Qt::OptionConverter");
    e << option;
    logger()->error(e);
    return 0;
}

Level OptionConverter::toLevel(const QString &option,
                               bool *ok)
{
    bool convertOk;
    Level level = Level::fromString(option.toUpper().trimmed(), &convertOk);
    if (ok != nullptr)
        *ok = convertOk;
    if (convertOk)
        return level;

    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Invalid option string '%1' for a level"),
                              CONFIGURATOR_INVALID_OPTION_ERROR,
                              "Log4Qt::OptionConverter");
    e << option;
    logger()->error(e);
    return level;
}

Level OptionConverter::toLevel(const QString &option,
                               Log4Qt::Level defaultValue)
{
    bool ok;
    Level result = toLevel(option, &ok);
    if (ok)
        return result;

    return defaultValue;
}

int OptionConverter::toTarget(const QString &option,
                              bool *ok)
{
    const QLatin1String java_stdout("system.out");
    const QLatin1String cpp_stdout("stdout_target");
    const QLatin1String java_stderr("system.err");
    const QLatin1String cpp_stderr("stderr_target");

    if (ok)
        *ok = true;
    QString s = option.trimmed().toLower();
    if (s == java_stdout || s == cpp_stdout)
        return ConsoleAppender::STDOUT_TARGET;
    if (s == java_stderr || s == cpp_stderr)
        return ConsoleAppender::STDERR_TARGET;

    if (ok)
        *ok = false;
    LogError e = LOG4QT_ERROR(QT_TR_NOOP("Invalid option string '%1' for a target"),
                              CONFIGURATOR_INVALID_OPTION_ERROR,
                              "Log4Qt::OptionConverter");
    e << option;
    logger()->error(e);
    return ConsoleAppender::STDOUT_TARGET;
}

} // namespace Log4Qt
