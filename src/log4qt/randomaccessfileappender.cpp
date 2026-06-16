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

#include "randomaccessfileappender.h"
#include "abstractstringlayout.h"
#include "abstractlayout.h"
#include "loggingevent.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>

using namespace Qt::StringLiterals;

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Log4Qt
{

// Convenience alias: the per-thread staging buffer lives in
// AbstractStringLayout so all appenders can share it without each
// having their own thread_local.
static inline QByteArray &encodedMessageBuffer()
{
    return AbstractStringLayout::threadLocalBuffer();
}

RandomAccessFileAppender::RandomAccessFileAppender(QObject *parent)
    : AppenderSkeleton(false, parent)
    , mAppendFile(false)
    , mBufferSize(256 * 1024)
    , mImmediateFlush(false)
{
}

RandomAccessFileAppender::RandomAccessFileAppender(const LayoutSharedPtr &layout,
                                                   const QString &fileName,
                                                   QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , mAppendFile(false)
    , mBufferSize(256 * 1024)
    , mImmediateFlush(false)
    , mFileName(fileName)
{
}

RandomAccessFileAppender::RandomAccessFileAppender(const LayoutSharedPtr &layout,
                                                   const QString &fileName,
                                                   bool append,
                                                   QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , mAppendFile(append)
    , mBufferSize(256 * 1024)
    , mImmediateFlush(false)
    , mFileName(fileName)
{
}

RandomAccessFileAppender::~RandomAccessFileAppender()
{
    closeInternal();
}

bool RandomAccessFileAppender::requiresLayout() const
{
    return true;
}

void RandomAccessFileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (mFileName.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Activation of Appender '%1' that requires file and has no file set"),
                                         AppenderActivateMissingFileError);
        e << name();
        logger()->error(e);
        return;
    }

    closeFile();
    openFile();

    if (mFile && mFile->isOpen())
    {
        mByteBuffer.reserve(mBufferSize.load(std::memory_order_relaxed));
        AppenderSkeleton::activateOptions();
    }
}

void RandomAccessFileAppender::close()
{
    closeInternal();
    AppenderSkeleton::close();
}

void RandomAccessFileAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    closeFile();
}

bool RandomAccessFileAppender::checkEntryConditions() const
{
    if (!mFile || !mFile->isOpen())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without open file"),
                                         AppenderNoOpenFileError);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

bool RandomAccessFileAppender::handleIoErrors() const
{
    if (mFile->error() == QFile::NoError)
        return false;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to write to file '%1' for appender '%2'"),
                                     AppenderWritingFileError);
    e << mFileName << name();
    e.addCausingError(LogError(mFile->errorString(), mFile->error()));
    logger()->error(e);
    return true;
}

void RandomAccessFileAppender::preAppend(const LoggingEvent &event, const LayoutSharedPtr &layout)
{
    // Called outside mObjectGuard — format and encode here so that append()
    // (which runs under the lock) only needs to copy bytes into the buffer.
    QByteArray &buf = encodedMessageBuffer();
    buf.clear();
    if (auto *sl = qobject_cast<AbstractStringLayout *>(layout.data()))
        sl->formatTo(event, buf);
    else
        buf = layout->format(event).toUtf8();
}

void RandomAccessFileAppender::append(const LoggingEvent &event)
{
    Q_UNUSED(event)
    // s_encodedMessage was filled by preAppend() outside the lock.
    // If it is empty (e.g. close() raced between preAppend and append),
    // there is nothing to write.
    QByteArray &encoded = encodedMessageBuffer();
    if (encoded.isEmpty())
        return;

    if (mByteBuffer.size() + encoded.size() > mBufferSize.load(std::memory_order_relaxed))
        flushBuffer();

    mByteBuffer.append(encoded);
    encoded.clear();

    if (mImmediateFlush.load(std::memory_order_relaxed))
        flushBuffer();
}

void RandomAccessFileAppender::flushBuffer()
{
    if (mByteBuffer.isEmpty())
        return;

    mFile->write(mByteBuffer);
    handleIoErrors();
    mByteBuffer.clear();
    // Note: clear() preserves the reserved capacity for reuse
}

void RandomAccessFileAppender::openFile()
{
    Q_ASSERT_X(!mFile, "RandomAccessFileAppender::openFile()", "Opening file without closing previous file");

    QFileInfo fileInfo(mFileName);
    const QString parentPath = fileInfo.absolutePath();
    if (!QDir(parentPath).exists())
    {
        logger()->trace(u"Creating missing parent directory for file %1"_s, mFileName);
        if (!QDir().mkpath(parentPath))
        {
            LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to create parent directory '%1' for file '%2' of appender '%3'"),
                                             AppenderOpeningFileError);
            e << parentPath << mFileName << name();
            logger()->error(e);
            return;
        }
    }

#ifdef Q_OS_WIN
    wchar_t buffer[MAX_PATH];
    if (ExpandEnvironmentStringsW(mFileName.toStdWString().c_str(), buffer, MAX_PATH))
        mFileName = QString::fromWCharArray(buffer);
#endif

    mFile = std::make_unique<QFile>(mFileName);
    QFile::OpenMode mode = QIODevice::WriteOnly;
    // No QIODevice::Text — we write raw UTF-8; the layout's endOfLine() already
    // returns the correct platform-specific line ending.
    // No QIODevice::Unbuffered — we manage the buffer ourselves.
    if (mAppendFile.load(std::memory_order_relaxed))
        mode |= QIODevice::Append;
    else
        mode |= QIODevice::Truncate;

    if (!mFile->open(mode))
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to open file '%1' for appender '%2'"),
                                         AppenderOpeningFileError);
        e << mFileName << name();
        e.addCausingError(LogError(mFile->errorString(), mFile->error()));
        logger()->error(e);
        mFile.reset();
        return;
    }
    logger()->debug(u"Opened file '%1' for appender '%2'"_s, mFile->fileName(), name());

    // Write the layout header (if any) into the buffer so it is included in
    // the first flush. Skip when appending to a non-empty existing file —
    // the header is already present from the previous run.
    const bool isNewFile = !mAppendFile.load(std::memory_order_relaxed) || mFile->size() == 0;
    if (isNewFile) {
        const LayoutSharedPtr l = layout();
        if (l && !l->header().isEmpty())
            mByteBuffer += l->header().toUtf8() + AbstractLayout::endOfLine().toUtf8();
    }
}

void RandomAccessFileAppender::closeFile()
{
    if (mFile)
    {
        logger()->debug(u"Closing file '%1' for appender '%2'"_s, mFile->fileName(), name());

        // Write the layout footer (if any) before the final flush so it is
        // included in the last data written to disk.
        const LayoutSharedPtr l = layout();
        if (l && !l->footer().isEmpty())
            mByteBuffer += l->footer().toUtf8() + AbstractLayout::endOfLine().toUtf8();

        flushBuffer();
    }
    mFile.reset();
    mByteBuffer.clear();
}

bool RandomAccessFileAppender::removeFile(QFile &file) const
{
    if (file.remove())
        return true;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to remove file '%1' for appender '%2'"),
                                     AppenderRemoveFileError);
    e << file.fileName() << name();
    e.addCausingError(LogError(file.errorString(), file.error()));
    logger()->error(e);
    return false;
}

bool RandomAccessFileAppender::renameFile(QFile &file, const QString &fileName) const
{
    logger()->debug(u"Renaming file '%1' to '%2'"_s, file.fileName(), fileName);
    if (file.rename(fileName))
        return true;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to rename file '%1' to '%2' for appender '%3'"),
                                     AppenderRenamingFileError);
    e << file.fileName() << fileName << name();
    e.addCausingError(LogError(file.errorString(), file.error()));
    logger()->error(e);
    return false;
}

QString RandomAccessFileAppender::file() const
{
    QMutexLocker locker(&mObjectGuard);
    return mFileName;
}

void RandomAccessFileAppender::setFile(const QString &fileName)
{
    QMutexLocker locker(&mObjectGuard);
    mFileName = fileName;
}

void RandomAccessFileAppender::setBufferSize(int bufferSize)
{
    QMutexLocker locker(&mObjectGuard);
    mBufferSize.store(bufferSize, std::memory_order_relaxed);
    if (mFile && mFile->isOpen())
        mByteBuffer.reserve(bufferSize);
}

} // namespace Log4Qt

#include "moc_randomaccessfileappender.cpp"
