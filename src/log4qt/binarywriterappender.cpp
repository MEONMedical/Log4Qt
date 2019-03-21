#include "binarywriterappender.h"
#include "binaryloggingevent.h"
#include "binarylayout.h"
#include <QDataStream>

namespace Log4Qt
{

BinaryWriterAppender::BinaryWriterAppender(QObject *parent) :
    AppenderSkeleton(false, parent),
    mWriter(nullptr)
{
}

BinaryWriterAppender::BinaryWriterAppender(QDataStream *dataStream,
        QObject *parent) :
    AppenderSkeleton(false, parent),
    mWriter(dataStream)
{
}

BinaryWriterAppender::~BinaryWriterAppender()
{
    closeInternal();
}

void BinaryWriterAppender::setWriter(QDataStream *dataStream)
{
    QMutexLocker locker(&mObjectGuard);

    closeWriter();

    mWriter = dataStream;
    writeHeader();
}


void BinaryWriterAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (writer() == nullptr)
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
    closeInternal();
    AppenderSkeleton::close();
}

void BinaryWriterAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    closeWriter();
}

bool BinaryWriterAppender::requiresLayout() const
{
    return false;
}

void BinaryWriterAppender::append(const LoggingEvent &event)
{
    const auto *binEvent = dynamic_cast<const BinaryLoggingEvent *>(&event);
    const LayoutSharedPtr l = layout();
    const BinaryLayout *bl = qobject_cast<BinaryLayout *>(l.data());

    if (binEvent != nullptr)
    {
        // handle binary events
        if (bl != nullptr)
            *mWriter << bl->binaryFormat(*binEvent);   // if it's a binary message and we have a binary layout output the binary message via the binary layout.
        else
            *mWriter << binEvent->binaryMessage();     // binary message, but no layout or not a binary layout, output the binary message without the layout
    }
    else
    {
        // handle non binary events
        if ((l != nullptr) && (bl == nullptr))
            *mWriter << l->format(event); // if the message and the layout are not binary, output it as in WriterAppender
        else
            *mWriter << event.message();  // if the message is not binary and there is no layout or the layout is binary, output it without the layout
    }

    handleIoErrors();
}

bool BinaryWriterAppender::checkEntryConditions() const
{
    if (mWriter == nullptr)
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
    if (mWriter == nullptr)
        return;

    writeFooter();
    mWriter = nullptr;
}

bool BinaryWriterAppender::handleIoErrors() const
{
    return false;
}

void BinaryWriterAppender::writeHeader() const
{
    if ((layout() != nullptr) && (mWriter != nullptr))
        writeRawData(binaryLayout()->binaryHeader());
}

void BinaryWriterAppender::writeFooter() const
{
    if ((layout() != nullptr) && (mWriter != nullptr))
        writeRawData(binaryLayout()->binaryFooter());
}

void BinaryWriterAppender::writeRawData(const QByteArray &data) const
{
    if (data.isEmpty())
        return;

    mWriter->writeRawData(data.constData(), data.size());

    if (handleIoErrors())
        return;
}

BinaryLayout *BinaryWriterAppender::binaryLayout() const
{
    return qobject_cast<BinaryLayout *>(layout().data());
}

} // namespace Log4Qt

#include "moc_binarywriterappender.cpp"
