#include "binarywriterappender.h"
#include "binaryloggingevent.h"
#include "binarylayout.h"
#include <QDataStream>

namespace Log4Qt
{

BinaryWriterAppender::BinaryWriterAppender(QObject *pParent) :
    AppenderSkeleton(false, pParent),
    mpWriter(nullptr)
{
}

BinaryWriterAppender::BinaryWriterAppender(QDataStream *pDataStream,
        QObject *pParent) :
    AppenderSkeleton(false, pParent),
    mpWriter(pDataStream)
{
}

BinaryWriterAppender::~BinaryWriterAppender()
{
    close();
}

void BinaryWriterAppender::setWriter(QDataStream *pDataStream)
{
    QMutexLocker locker(&mObjectGuard);

    closeWriter();

    mpWriter = pDataStream;
    writeHeader();
}


void BinaryWriterAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (!writer())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Activation of Appender '%1' that requires writer and has no writer set"),
                                         APPENDER_ACTIVATE_MISSING_WRITER_ERROR);
        e << name();
        logger()->error(e);
        return;
    }

    AppenderSkeleton::activateOptions();
}

void BinaryWriterAppender::close()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    AppenderSkeleton::close();
    closeWriter();
}

bool BinaryWriterAppender::requiresLayout() const
{
    return false;
}

void BinaryWriterAppender::append(const LoggingEvent &rEvent)
{
    const BinaryLoggingEvent *binEvent = dynamic_cast<const BinaryLoggingEvent *>(&rEvent);
    const LayoutSharedPtr l = layout();
    const BinaryLayout *bl = qobject_cast<BinaryLayout *>(l.data());

    if (binEvent)
    {
        // handle binary events
        if (bl)
            *mpWriter << bl->binaryFormat(*binEvent);   // if it's a binary message and we have a binary layout output the binary message via the binary layout.
        else
            *mpWriter << binEvent->binaryMessage();     // binary message, but no layout or not a binary layout, output the binary message without the layout
    }
    else
    {
        // handle non binary events
        if (l && !bl)
            *mpWriter << l->format(rEvent); // if the message and the layout are not binary, output it as in WriterAppender
        else
            *mpWriter << rEvent.message();  // if the message is not binary and there is no layout or the layout is binary, output it without the layout
    }

    handleIoErrors();
}

bool BinaryWriterAppender::checkEntryConditions() const
{
    if (!writer())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without a writer set"),
                                         APPENDER_USE_MISSING_WRITER_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}


void BinaryWriterAppender::closeWriter()
{
    if (!mpWriter)
        return;

    writeFooter();
    mpWriter = nullptr;
}

bool BinaryWriterAppender::handleIoErrors() const
{
    return false;
}

void BinaryWriterAppender::writeHeader() const
{
    if (layout() && mpWriter)
        writeRawData(binaryLayout()->binaryHeader());
}

void BinaryWriterAppender::writeFooter() const
{
    if (layout() && mpWriter)
        writeRawData(binaryLayout()->binaryFooter());
}

void BinaryWriterAppender::writeRawData(const QByteArray &data) const
{
    if (data.isEmpty())
        return;

    mpWriter->writeRawData(data.constData(), data.size());

    if (handleIoErrors())
        return;
}

BinaryLayout *BinaryWriterAppender::binaryLayout() const
{
    return qobject_cast<BinaryLayout *>(layout().data());
}

} // namespace Log4Qt

#include "moc_binarywriterappender.cpp"
