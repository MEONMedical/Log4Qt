#include "xmllayout.h"
#include "loggingevent.h"

#include <QXmlStreamWriter>

namespace Log4Qt
{

XMLLayout::XMLLayout(QObject *pParent)
    : Layout(pParent)

{
}

XMLLayout::~XMLLayout()
{
}

QString XMLLayout::format(const LoggingEvent &rEvent)
{
    QString output;
    QXmlStreamWriter writer(&output);

    writer.writeStartElement(QStringLiteral("log4j:event"));
    writer.writeAttribute(QStringLiteral("logger"), rEvent.loggerName());
    writer.writeAttribute(QStringLiteral("timestamp"), QString::number(rEvent.timeStamp()));
    writer.writeAttribute(QStringLiteral("level"), rEvent.level().toString());
    writer.writeAttribute(QStringLiteral("thread"), rEvent.threadName());

    writer.writeStartElement(QStringLiteral("log4j:message"));
    writer.writeCDATA(rEvent.message());
    writer.writeEndElement();

    if (!rEvent.ndc().isEmpty())
    {
        writer.writeStartElement(QStringLiteral("log4j:NDC"));
        writer.writeCDATA(rEvent.ndc());
        writer.writeEndElement();
    }

    auto props = rEvent.properties();
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
