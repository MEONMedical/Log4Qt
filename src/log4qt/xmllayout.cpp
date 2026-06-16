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

#include "xmllayout.h"

#include "loggingevent.h"

#include <QXmlStreamWriter>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

XMLLayout::XMLLayout(QObject *parent)
    : AbstractStringLayout(parent)
{
}

QString XMLLayout::contentType() const
{
    return u"application/xml; charset=UTF-8"_s;
}

QString XMLLayout::format(const LoggingEvent &event)
{
    QString output;
    QXmlStreamWriter writer(&output);

    writer.writeStartElement(u"log4j:event"_s);
    writer.writeAttribute(u"logger"_s, event.loggername());
    writer.writeAttribute(u"timestamp"_s, QString::number(event.timeStamp()));
    writer.writeAttribute(u"level"_s, event.level().toString());
    writer.writeAttribute(u"thread"_s, event.threadName());

    writer.writeStartElement(u"log4j:message"_s);
    writer.writeCDATA(event.message());
    writer.writeEndElement();

    if (!event.ndc().isEmpty())
    {
        writer.writeStartElement(u"log4j:NDC"_s);
        writer.writeCDATA(event.ndc());
        writer.writeEndElement();
    }

    auto props = event.properties();
    if (!props.isEmpty())
    {
        writer.writeStartElement(u"log4j:properties"_s);
        for (const auto &[key, value] : props.asKeyValueRange())
        {
            writer.writeStartElement(u"log4j:data"_s);
            writer.writeAttribute(u"name"_s, key);
            writer.writeAttribute(u"value"_s, value);
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }
    writer.writeEndElement();

    return output;
}

}

#include "moc_xmllayout.cpp"
