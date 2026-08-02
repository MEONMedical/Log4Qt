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

#include "jsonconfigurator.h"

#include "helpers/configuratorhelper.h"
#include "helpers/properties.h"
#include "logger.h"
#include "propertyconfigurator.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(staticLogger, Log4Qt::JsonConfigurator)

bool JsonConfigurator::doConfigure(const QString &configFileName,
                                   LoggerRepository *loggerRepository)
{
    Properties properties;
    if (!jsonToProperties(configFileName, properties))
        return false;

    PropertyConfigurator configurator;
    return configurator.doConfigure(properties, loggerRepository);
}

bool JsonConfigurator::configure(const QString &configFilename)
{
    return doConfigure(configFilename);
}

bool JsonConfigurator::configureAndWatch(const QString &configFilename)
{
    ConfiguratorHelper::setConfigurationFile();
    if (configFilename.isEmpty())
        return true;

    const bool result = doConfigure(configFilename);
    ConfiguratorHelper::setConfigurationFile(configFilename, configure);
    return result;
}

bool JsonConfigurator::jsonToProperties(const QString &file, Properties &properties)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LogError e = LOG4QT_ERROR("Unable to open JSON file '%1'",
                                  ConfiguratorOpeningFileError,
                                  "Log4Qt::JsonConfigurator");
        e << file;
        e.addCausingError(LogError(f.errorString(), f.error()));
        staticLogger()->error(e);
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &parseError);
    if (f.error())
    {
        LogError e = LOG4QT_ERROR("Unable to read JSON file '%1'",
                                  ConfiguratorReadingFileError,
                                  "Log4Qt::JsonConfigurator");
        e << file;
        e.addCausingError(LogError(f.errorString(), f.error()));
        staticLogger()->error(e);
        return false;
    }

    if (parseError.error != QJsonParseError::NoError)
    {
        LogError e = LOG4QT_ERROR("Unable to read JSON file '%1'",
                                  ConfiguratorReadingFileError,
                                  "Log4Qt::JsonConfigurator");
        e << file;
        e.addCausingError(LogError(parseError.errorString(), parseError.error));
        staticLogger()->error(e);
        return false;
    }

    if (!doc.isObject())
    {
        LogError e = LOG4QT_ERROR("Unable to read JSON file '%1'",
                                  ConfiguratorReadingFileError,
                                  "Log4Qt::JsonConfigurator");
        e << file;
        e.addCausingError(LogError(u"Root element is not a JSON object"_s, 0));
        staticLogger()->error(e);
        return false;
    }

    flattenJsonObject(doc.object(), QString(), properties);
    return true;
}

void JsonConfigurator::flattenJsonObject(const QJsonObject &object,
                                         const QString &prefix,
                                         Properties &properties)
{
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        const QString fullKey = prefix.isEmpty() ? it.key() : prefix + it.key();
        flattenJsonValue(it.value(), fullKey, properties);
    }
}

void JsonConfigurator::flattenJsonArray(const QJsonArray &array,
                                        const QString &prefix,
                                        Properties &properties)
{
    // Arrays flatten to numeric key segments, equivalent to a JSON object
    // with "0", "1", ... keys — e.g. "appenderRef": [{"ref": "console"}]
    // yields rootLogger.appenderRef.0.ref.
    for (qsizetype i = 0; i < array.size(); ++i)
        flattenJsonValue(array.at(i), prefix + u"."_s + QString::number(i),
                         properties);
}

void JsonConfigurator::flattenJsonValue(const QJsonValue &value,
                                        const QString &fullKey,
                                        Properties &properties)
{
    if (value.isObject())
    {
        flattenJsonObject(value.toObject(), fullKey + u"."_s, properties);
    }
    else if (value.isArray())
    {
        flattenJsonArray(value.toArray(), fullKey, properties);
    }
    else if (value.isString())
    {
        properties.setProperty(fullKey, value.toString());
    }
    else if (value.isBool())
    {
        properties.setProperty(fullKey, value.toBool() ? u"true"_s : u"false"_s);
    }
    else if (value.isDouble())
    {
        double d = value.toDouble();
        if (d == static_cast<qint64>(d))
            properties.setProperty(fullKey, QString::number(static_cast<qint64>(d)));
        else
            properties.setProperty(fullKey, QString::number(d));
    }
    else if (value.isNull())
    {
        properties.setProperty(fullKey, QString());
    }
}

} // namespace Log4Qt
