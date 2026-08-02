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

#include "xmlconfigurator.h"

#include "helpers/configuratorhelper.h"
#include "helpers/properties.h"
#include "logger.h"
#include "propertyconfigurator.h"

#include <QFile>
#include <QXmlStreamReader>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(staticLogger, Log4Qt::XmlConfigurator)

bool XmlConfigurator::doConfigure(const QString &configFileName,
                                  LoggerRepository *loggerRepository)
{
    Properties properties;
    if (!xmlToProperties(configFileName, properties))
        return false;

    PropertyConfigurator configurator;
    return configurator.doConfigure(properties, loggerRepository);
}

bool XmlConfigurator::configure(const QString &configFilename)
{
    return doConfigure(configFilename);
}

bool XmlConfigurator::configureAndWatch(const QString &configFilename)
{
    ConfiguratorHelper::setConfigurationFile();
    if (configFilename.isEmpty())
        return true;

    const bool result = doConfigure(configFilename);
    ConfiguratorHelper::setConfigurationFile(configFilename, configure);
    return result;
}

bool XmlConfigurator::xmlToProperties(const QString &file, Properties &properties)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LogError e = LOG4QT_ERROR("Unable to open XML file '%1'",
                                  ConfiguratorOpeningFileError,
                                  "Log4Qt::XmlConfigurator");
        e << file;
        e.addCausingError(LogError(f.errorString(), f.error()));
        staticLogger()->error(e);
        return false;
    }

    QXmlStreamReader xml(&f);

    // Advance to the document element
    while (!xml.atEnd() && !xml.isStartElement())
        xml.readNext();

    if (xml.hasError())
    {
        LogError e = LOG4QT_ERROR("Unable to read XML file '%1'",
                                  ConfiguratorReadingFileError,
                                  "Log4Qt::XmlConfigurator");
        e << file;
        e.addCausingError(LogError(xml.errorString(), static_cast<int>(xml.error())));
        staticLogger()->error(e);
        return false;
    }

    if (xml.atEnd())
    {
        LogError e = LOG4QT_ERROR("Unable to read XML file '%1'",
                                  ConfiguratorReadingFileError,
                                  "Log4Qt::XmlConfigurator");
        e << file;
        e.addCausingError(LogError(u"XML file contains no root element"_s, 0));
        staticLogger()->error(e);
        return false;
    }

    // Skip the root element name and flatten its children at top level
    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isStartElement())
            flattenXmlElement(xml, QString(), properties);
        else if (xml.isEndElement())
            break;
    }

    if (xml.hasError())
    {
        LogError e = LOG4QT_ERROR("Unable to read XML file '%1'",
                                  ConfiguratorReadingFileError,
                                  "Log4Qt::XmlConfigurator");
        e << file;
        e.addCausingError(LogError(xml.errorString(), static_cast<int>(xml.error())));
        staticLogger()->error(e);
        return false;
    }

    return true;
}

void XmlConfigurator::flattenXmlElement(QXmlStreamReader &xml,
                                        const QString &prefix,
                                        Properties &properties)
{
    // We are positioned on a StartElement. Read its children.
    const QString elementName = xml.name().toString();
    const QString fullKey = prefix.isEmpty() ? elementName : prefix + elementName;

    // Flatten XML attributes as child properties
    const auto attributes = xml.attributes();
    for (const auto &attr : attributes)
        properties.setProperty(fullKey + u"."_s + attr.name().toString(), attr.value().toString());

    QString textContent;
    bool hasChildElements = false;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isStartElement())
        {
            hasChildElements = true;
            flattenXmlElement(xml, fullKey + u"."_s, properties);
        }
        else if (xml.isCharacters() && !xml.isWhitespace())
        {
            textContent += xml.text().toString();
        }
        else if (xml.isEndElement())
        {
            break;
        }
    }

    if (!hasChildElements && !textContent.isEmpty())
        properties.setProperty(fullKey, textContent);
}

} // namespace Log4Qt
