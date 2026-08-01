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

#ifndef LOG4QT_XMLCONFIGURATOR_H
#define LOG4QT_XMLCONFIGURATOR_H

#include "log4qtshared.h"

class QXmlStreamReader;

namespace Log4Qt
{

class Properties;
class LoggerRepository;

/*!
 * \brief The class XmlConfigurator allows the configuration of the
 *        package from an XML file.
 *
 * The XML structure maps directly to the flat property keys used by
 * PropertyConfigurator: element names become dot-separated key segments
 * verbatim, element text becomes the value, and XML attributes are
 * flattened as child properties. For example:
 *
 * \code{.xml}
 * <?xml version="1.0" encoding="UTF-8"?>
 * <configuration>
 *     <appender>
 *         <console>
 *             <type>Console</type>
 *             <layout conversionPattern="%-5p %c - %m%n">
 *                 <type>PatternLayout</type>
 *             </layout>
 *         </console>
 *     </appender>
 *     <rootLogger level="ALL">
 *         <appenderRef>
 *             <ref0 ref="console"/>
 *         </appenderRef>
 *     </rootLogger>
 *     <logger>
 *         <MyApp name="MyApp" level="ERROR" additivity="false">
 *             <appenderRef>
 *                 <ref0 ref="console"/>
 *             </appenderRef>
 *         </MyApp>
 *     </logger>
 * </configuration>
 * \endcode
 *
 * This produces the equivalent flat properties:
 * \code
 * appender.console.type=Console
 * appender.console.layout.type=PatternLayout
 * appender.console.layout.conversionPattern=%-5p %c - %m%n
 * rootLogger.level=ALL
 * rootLogger.appenderRef.ref0.ref=console
 * logger.MyApp.name=MyApp
 * logger.MyApp.level=ERROR
 * logger.MyApp.additivity=false
 * logger.MyApp.appenderRef.ref0.ref=console
 * \endcode
 *
 * \note The Log4j2 XML dialect (\c &lt;Configuration&gt;&lt;Appenders&gt;
 *       \c &lt;Console name="..."&gt; ...) is \e not supported — element
 *       names are not translated, so such a file flattens to keys the
 *       configurator ignores and configures nothing.
 *
 * Variable substitution (\c ${varname}) works in attribute values
 * and text content, resolved by OptionConverter::findAndSubst() after
 * flattening.
 *
 * The XML file is flattened into a Properties object and then
 * delegated to PropertyConfigurator for the actual configuration.
 *
 * During automatic initialization the LogManager searches for
 * \c log4qt.xml after \c log4qt.json, so \c .properties and
 * \c .json files take priority. The \c Configuration setting also
 * accepts \c .xml files.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \sa PropertyConfigurator, JsonConfigurator, LogManager::startup()
 */
class LOG4QT_EXPORT XmlConfigurator
{
public:
    XmlConfigurator() = delete;

    /*!
     * Reads the configuration from the XML file \a configFileName
     * and configures the \a loggerRepository.
     *
     * \sa ConfiguratorHelper::configureError()
     */
    static bool doConfigure(const QString &configFileName,
                            LoggerRepository *loggerRepository = nullptr);

    /*!
     * Reads the configuration from the XML file \a configFilename
     * and configures the default LoggerRepository.
     *
     * \sa ConfiguratorHelper::configureError()
     */
    static bool configure(const QString &configFilename);

    /*!
     * Reads the configuration from the XML file \a configFilename,
     * configures the default LoggerRepository, and watches the file
     * for changes.
     *
     * \sa ConfiguratorHelper::configureError(),
     *     ConfiguratorHelper::configurationFile()
     */
    static bool configureAndWatch(const QString &configFilename);

private:
    static bool xmlToProperties(const QString &file, Properties &properties);
    static void flattenXmlElement(QXmlStreamReader &xml,
                                  const QString &prefix,
                                  Properties &properties);
};

} // namespace Log4Qt

#endif // LOG4QT_XMLCONFIGURATOR_H
