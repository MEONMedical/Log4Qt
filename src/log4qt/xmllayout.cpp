/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#include "xmllayout.h"
#include "loggingevent.h"

#include <QXmlStreamWriter>

namespace Log4Qt
{

XMLLayout::XMLLayout(QObject *parent)
    : Layout(parent)

{
}

QString XMLLayout::format(const LoggingEvent &event)
{
    QString output;
    QXmlStreamWriter writer(&output);

    writer.writeStartElement(QStringLiteral("log4j:event"));
    writer.writeAttribute(QStringLiteral("logger"), event.loggename());
    writer.writeAttribute(QStringLiteral("timestamp"), QString::number(event.timeStamp()));
    writer.writeAttribute(QStringLiteral("level"), event.level().toString());
    writer.writeAttribute(QStringLiteral("thread"), event.threadName());

    writer.writeStartElement(QStringLiteral("log4j:message"));
    writer.writeCDATA(event.message());
    writer.writeEndElement();

    if (!event.ndc().isEmpty())
    {
        writer.writeStartElement(QStringLiteral("log4j:NDC"));
        writer.writeCDATA(event.ndc());
        writer.writeEndElement();
    }

    auto props = event.properties();
    if (!props.isEmpty())
    {
        writer.writeStartElement(QStringLiteral("log4j:properties"));
        for (auto pos = props.constBegin(); pos != props.constEnd(); ++pos)
        {
            writer.writeStartElement(QStringLiteral("log4j:data"));
            writer.writeAttribute(QStringLiteral("name"), pos.key());
            writer.writeAttribute(QStringLiteral("value"), pos.value());
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }
    writer.writeEndElement();

    return output;
}

}

#include "moc_xmllayout.cpp"
