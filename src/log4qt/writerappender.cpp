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

#include "writerappender.h"

#include "abstractlayout.h"
#include "loggingevent.h"

namespace Log4Qt
{

WriterAppender::WriterAppender(QObject *parent) :
    AppenderSkeleton(false, parent),
    mEncoding(QStringConverter::Encoding::Utf8),
    mWriter(nullptr),
    mImmediateFlush(true)
{
}

WriterAppender::WriterAppender(const LayoutSharedPtr &layout,
                               QObject *parent) :
    AppenderSkeleton(false, layout, parent),
    mEncoding(QStringConverter::Encoding::System),
    mWriter(nullptr),
    mImmediateFlush(true)
{
}

WriterAppender::WriterAppender(const LayoutSharedPtr &layout,
                               QTextStream *textStream,
                               QObject *parent) :
    AppenderSkeleton(false, layout, parent),
    mEncoding(QStringConverter::Encoding::System),
    mWriter(textStream),
    mImmediateFlush(true)
{
}

WriterAppender::~WriterAppender()
{
    closeInternal();
}

void WriterAppender::setEncoding(QStringConverter::Encoding encoding)
{
    QMutexLocker locker(&mObjectGuard);
    if (mEncoding == encoding)
        return;

    mEncoding = encoding;
    if (mWriter != nullptr)
        mWriter->setEncoding(mEncoding);
}

void WriterAppender::setWriter(QTextStream *textStream)
{
    QMutexLocker locker(&mObjectGuard);

    closeWriter();

    mWriter = textStream;
    if (mWriter != nullptr)
        mWriter->setEncoding(mEncoding);
    writeHeader();
}

void WriterAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (writer() == nullptr)
    {
        LogError e = LOG4QT_QCLASS_ERROR("Activation of Appender '%1' that requires writer and has no writer set",
                                         AppenderActivateMissingWriterError);
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
    // Called under mObjectGuard by doAppend Phase 5 — read mpLayout directly
    // to avoid an extra recursive-mutex acquisition and a QSharedPointer copy.
    const LayoutSharedPtr &layoutSnap = layoutSnapshot();
    if (!layoutSnap)
        return;

    *mWriter << layoutSnap->format(event);
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
        LogError e = LOG4QT_QCLASS_ERROR("Use of appender '%1' without a writer set",
                                         AppenderUseMissingWriterError);
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

void WriterAppender::suppressNextFooter()
{
    mSuppressNextFooter = true;
}

void WriterAppender::writeFooter() const
{
    if (mSuppressNextFooter) {
        mSuppressNextFooter = false;
        return;
    }

    if (!layout() || (mWriter == nullptr))
        return;

    QString footer = layout()->footer();
    if (footer.isEmpty())
        return;

    *mWriter << footer << AbstractLayout::endOfLine();
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

    *mWriter << header << AbstractLayout::endOfLine();
    if (handleIoErrors())
        return;
}

} // namespace Log4Qt

#include "moc_writerappender.cpp"
