/******************************************************************************
 *
 * package:     Log4Qt
 * file:        optionconverter.h
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

#ifndef LOG4QT_OPTIONCONVERTER_H
#define LOG4QT_OPTIONCONVERTER_H

#include <log4qt/log4qtshared.h>
#include <log4qt/level.h>

#include <QString>

namespace Log4Qt
{
class Properties;

/*!
 * \brief The class OptionConverter provides functions to convert strings
 *        to property values.
 */
class  LOG4QT_EXPORT OptionConverter
{
private:
    OptionConverter();
    Q_DISABLE_COPY(OptionConverter)
public:
    static QString findAndSubst(const Properties &properties,
                                const QString &key);

    /*!
     * Returns the JAVA class name \a className as C++ class name by
     * replacing all . characters with ::.
     */
    static QString classNameJavaToCpp(const QString &className);

    /*!
     * Converts the option \a option to a boolean value. Valid strings
     * for true are "true", "enabled" and "1". Valid strings
     * for false are "false", "disabled" and "0". If the conversion is
     * successful, the target is returned and \a ok is set to true.
     * Otherwise an error is written to the log, \a ok is set to false
     * and false is returned.
     */
    static bool toBoolean(const QString &option,
                          bool *ok = nullptr);

    static bool toBoolean(const QString &option,
                          bool defaultValue);

    /*!
     * Converts the option string \a option to a file size. The string can
     * be a positive integer followed by an optional unit suffix "KB", "MB"
     * or "GB". If a unit suffix is specified the the integer is
     * interpreted as kilobytes, megabytes or gigabytes. If the conversion
     * is successful, the size is returned and \a ok is set to true.
     * Otherwise an error is written to the log, \a ok is set to false
     * and 0 is returned.
     */
    static qint64 toFileSize(const QString &option,
                             bool *ok = nullptr);

    /*!
        * Converts the option \a option to a integer value using
        * QString::toInt(). If the conversion is successful, the integer is
        * returned and \a ok is set to true. Otherwise an error is written
        * to the log, \a ok is set to false and 0 is returned.
        */
    static int toInt(const QString &option,
                     bool *ok = nullptr);

    /*!
     * Converts the option \a option to a level value using
     * Level::fromString(). If the conversion is successful, the level
     * is returned and \a ok is set to true. Otherwise an error is
     * written to the log, \a ok is set to false and a level with
     * the value Level::NULL_INT is returned.
     *
     * \sa Level::fromString()
     */
    static Level toLevel(const QString &option,
                         bool *ok = nullptr);

    static Level toLevel(const QString &option,
                         Log4Qt::Level defaultValue);

    /*!
     * Converts the option \a option to a ConsoleAppender::Target value.
     * Valid strings for \a option are "System.out", "STDOUT_TARGET",
     * "System.err" and "STDERR_TARGET". If the conversion is successful,
     * the target is returned and \a ok is set to true. Otherwise an
     * error is written to the log, \a ok is set to false and
     * ConsoleAppender::STDOUT_TARGET is returned.
     */
    static int toTarget(const QString &option,
                        bool *ok = nullptr);
};

} // namespace Log4Qt


Q_DECLARE_TYPEINFO(Log4Qt::OptionConverter, Q_MOVABLE_TYPE);


#endif // LOG4QT_OPTIONCONVERTER_H
