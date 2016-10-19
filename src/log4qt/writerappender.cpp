/******************************************************************************
 *
 * package:     Log4Qt
 * file:        writerappender.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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

#include <QTextCodec>

namespace Log4Qt
{

WriterAppender::WriterAppender(QObject *pParent) :
    AppenderSkeleton(false, pParent),
    mpEncoding(Q_NULLPTR),
    mpWriter(Q_NULLPTR),
    mImmediateFlush(true)
{
}


WriterAppender::WriterAppender(LayoutSharedPtr pLayout,
                               QObject *pParent) :
    AppenderSkeleton(false, pParent),
    mpEncoding(Q_NULLPTR),
    mpWriter(Q_NULLPTR),
    mImmediateFlush(true)
{
    setLayout(pLayout);
}


WriterAppender::WriterAppender(LayoutSharedPtr pLayout,
                               QTextStream *pTextStream,
                               QObject *pParent) :
    AppenderSkeleton(false, pParent),
    mpEncoding(Q_NULLPTR),
    mpWriter(pTextStream),
    mImmediateFlush(true)
{
    setLayout(pLayout);
}


WriterAppender::~WriterAppender()
{
    close();
}


void WriterAppender::setEncoding(QTextCodec *pEncoding)
{
    QMutexLocker locker(&mObjectGuard);

    if (mpEncoding == pEncoding)
        return;

    mpEncoding = pEncoding;
    if (mpWriter)
    {
        if (mpEncoding)
            mpWriter->setCodec(mpEncoding);
        else
            mpWriter->setCodec(QTextCodec::codecForLocale());
    }
}


void WriterAppender::setWriter(QTextStream *pTextStream)
{
    QMutexLocker locker(&mObjectGuard);

    closeWriter();

    mpWriter = pTextStream;
    if (mpEncoding && mpWriter)
        mpWriter->setCodec(mpEncoding);
    writeHeader();
}


void WriterAppender::activateOptions()
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


void WriterAppender::close()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    AppenderSkeleton::close();
    closeWriter();
}


bool WriterAppender::requiresLayout() const
{
    return true;
}


void WriterAppender::append(const LoggingEvent &rEvent)
{
    Q_ASSERT_X(layout(), "WriterAppender::append()", "Layout must not be null");

    QString message(layout()->format(rEvent));

    *mpWriter << message;
    if (handleIoErrors())
        return;

    if (immediateFlush())
    {
        mpWriter->flush();
        if (handleIoErrors())
            return;
    }
}


bool WriterAppender::checkEntryConditions() const
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

void WriterAppender::closeWriter()
{
    if (!mpWriter)
        return;

    writeFooter();
    mpWriter = 0;
}

bool WriterAppender::handleIoErrors() const
{
    return false;
}


void WriterAppender::writeFooter() const
{
    if (!layout() || !mpWriter)
        return;

    QString footer = layout()->footer();
    if (footer.isEmpty())
        return;

    *mpWriter << footer << Layout::endOfLine();
    if (handleIoErrors())
        return;
}


void WriterAppender::writeHeader() const
{
    if (!layout() || !mpWriter)
        return;

    QString header = layout()->header();
    if (header.isEmpty())
        return;

    *mpWriter << header << Layout::endOfLine();
    if (handleIoErrors())
        return;
}


} // namespace Log4Qt
