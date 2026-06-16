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

#ifndef LOG4QT_JSONCONFIGURATOR_H
#define LOG4QT_JSONCONFIGURATOR_H

#include "log4qtshared.h"

class QJsonObject;

namespace Log4Qt
{

class Properties;
class LoggerRepository;

/*!
 * \brief The class JsonConfigurator allows the configuration of the
 *        package from a JSON file using a Log4j2-style structured format.
 *
 * The JSON structure maps directly to the flat property keys used by
 * PropertyConfigurator. Nested objects produce dot-separated keys.
 * For example:
 *
 * \code{.json}
 * {
 *     "appender": {
 *         "console": {
 *             "type": "Console",
 *             "name": "console",
 *             "layout": {
 *                 "type": "PatternLayout",
 *                 "conversionPattern": "%-5p %c - %m%n"
 *             }
 *         }
 *     },
 *     "rootLogger": {
 *         "level": "ALL",
 *         "appenderRef": {
 *             "0": { "ref": "console" }
 *         }
 *     },
 *     "logger": {
 *         "MyApp": {
 *             "name": "MyApp",
 *             "level": "ERROR",
 *             "additivity": "false",
 *             "appenderRef": {
 *                 "0": { "ref": "console" }
 *             }
 *         }
 *     }
 * }
 * \endcode
 *
 * Flattening rules:
 * - Nested objects produce dot-separated keys:
 *   \c {"a":{"b":"c"}} becomes \c a.b=c
 * - JSON booleans and numbers are stringified
 *   (\c true becomes \c "true", \c 42 becomes \c "42")
 * - Variable substitution (\c ${varname}) works in string values
 *   and is resolved by OptionConverter::findAndSubst() after
 *   flattening
 *
 * The JSON file is flattened into a Properties object and then
 * delegated to PropertyConfigurator for the actual configuration.
 * This reuses all existing parsing, factory, and type-conversion
 * logic with zero duplication.
 *
 * During automatic initialization the LogManager searches for
 * \c log4qt.json after \c log4qt.properties, so \c .properties
 * files take priority. The \c Configuration setting also accepts
 * \c .json files.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \sa PropertyConfigurator, LogManager::startup()
 */
class LOG4QT_EXPORT JsonConfigurator
{
public:
    JsonConfigurator() = delete;

    /*!
     * Reads the configuration from the JSON file \a configFileName
     * and configures the \a loggerRepository.
     *
     * \sa ConfiguratorHelper::configureError()
     */
    static bool doConfigure(const QString &configFileName,
                            LoggerRepository *loggerRepository = nullptr);

    /*!
     * Reads the configuration from the JSON file \a configFilename
     * and configures the default LoggerRepository.
     *
     * \sa ConfiguratorHelper::configureError()
     */
    static bool configure(const QString &configFilename);

    /*!
     * Reads the configuration from the JSON file \a configFilename,
     * configures the default LoggerRepository, and watches the file
     * for changes.
     *
     * \sa ConfiguratorHelper::configureError(),
     *     ConfiguratorHelper::configurationFile()
     */
    static bool configureAndWatch(const QString &configFilename);

private:
    static bool jsonToProperties(const QString &file, Properties &properties);
    static void flattenJsonObject(const QJsonObject &object,
                                  const QString &prefix,
                                  Properties &properties);
};

} // namespace Log4Qt

#endif // LOG4QT_JSONCONFIGURATOR_H
