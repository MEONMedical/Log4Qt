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
