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

#include "writerappender.h"

#include "layout.h"
#include "loggingevent.h"

#if QT_VERSION < 0x060000
#include <QTextCodec>
#endif

namespace Log4Qt
{

WriterAppender::WriterAppender(QObject *parent) :
    AppenderSkeleton(false, parent),
#if QT_VERSION < 0x060000
    mEncoding(nullptr),
#else
    mEncoding(QStringConverter::Encoding::Utf8),
#endif
    mWriter(nullptr),
    mImmediateFlush(true)
{
}

WriterAppender::WriterAppender(const LayoutSharedPtr &layout,
                               QObject *parent) :
    AppenderSkeleton(false, layout, parent),
#if QT_VERSION < 0x060000
    mEncoding(nullptr),
#else
    mEncoding(QStringConverter::Encoding::System),
#endif
    mWriter(nullptr),
    mImmediateFlush(true)
{
}


WriterAppender::WriterAppender(const LayoutSharedPtr &layout,
                               QTextStream *textStream,
                               QObject *parent) :
    AppenderSkeleton(false, layout, parent),
#if QT_VERSION < 0x060000
    mEncoding(nullptr),
#else
    mEncoding(QStringConverter::Encoding::System),
#endif
    mWriter(textStream),
    mImmediateFlush(true)
{
}

WriterAppender::~WriterAppender()
{
    closeInternal();
}

#if QT_VERSION < 0x060000
void WriterAppender::setEncoding(QTextCodec *encoding)
#else
void WriterAppender::setEncoding(QStringConverter::Encoding encoding)
#endif
{
    QMutexLocker locker(&mObjectGuard);
    if (mEncoding == encoding)
        return;

    mEncoding = encoding;
    if (mWriter != nullptr)
    {
#if QT_VERSION < 0x060000
        if (mEncoding != nullptr)
            mWriter->setCodec(mEncoding);
#else
        mWriter->setEncoding(mEncoding);
#endif
    }
}

void WriterAppender::setWriter(QTextStream *textStream)
{
    QMutexLocker locker(&mObjectGuard);

    closeWriter();

    mWriter = textStream;
#if QT_VERSION < 0x060000
    if ((mEncoding != nullptr) && (mWriter != nullptr))
        mWriter->setCodec(mEncoding);
#else
    if (mWriter != nullptr)
        mWriter->setEncoding(mEncoding);
#endif
    writeHeader();
}

void WriterAppender::activateOptions()
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

void WriterAppender::close()
{
    closeInternal();
    AppenderSkeleton::close();
}

void WriterAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    closeWriter();
}

bool WriterAppender::requiresLayout() const
{
    return true;
}

void WriterAppender::append(const LoggingEvent &event)
{
    Q_ASSERT_X(layout(), "WriterAppender::append()", "Layout must not be null");

    QString message(layout()->format(event));

    *mWriter << message;
    if (handleIoErrors())
        return;

    if (immediateFlush())
    {
        mWriter->flush();
        if (handleIoErrors())
            return;
    }
}

bool WriterAppender::checkEntryConditions() const
{
    if (writer() == nullptr)
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without a writer set"),
                                         APPENDER_USE_MISSING_WRITER_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

void WriterAppender::closeWriter()
{
    if (mWriter == nullptr)
        return;

    writeFooter();
    mWriter = nullptr;
}

bool WriterAppender::handleIoErrors() const
{
    return false;
}

void WriterAppender::writeFooter() const
{
    if (!layout() || (mWriter == nullptr))
        return;

    QString footer = layout()->footer();
    if (footer.isEmpty())
        return;

    *mWriter << footer << Layout::endOfLine();
    if (handleIoErrors())
        return;
}

void WriterAppender::writeHeader() const
{
    if (!layout() || (mWriter == nullptr))
        return;

    QString header = layout()->header();
    if (header.isEmpty())
        return;

    *mWriter << header << Layout::endOfLine();
    if (handleIoErrors())
        return;
}

} // namespace Log4Qt

#include "moc_writerappender.cpp"
