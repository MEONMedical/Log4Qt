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

#include "fileappender.h"
#include "abstractlayout.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

using namespace Qt::StringLiterals;

// if we are in WIN*
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Log4Qt
{

FileAppender::FileAppender(QObject *parent) :
    WriterAppender(parent),
    mAppendFile(false),
    mBufferedIo(true),
    mFile(nullptr),
    mTextStream(nullptr)
{
}

FileAppender::FileAppender(const LayoutSharedPtr &layout,
                           const QString &fileName,
                           QObject *parent) :
    WriterAppender(layout, parent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(fileName),
    mFile(nullptr),
    mTextStream(nullptr)
{
}

FileAppender::FileAppender(const LayoutSharedPtr &layout,
                           const QString &fileName,
                           bool append,
                           QObject *parent) :
    WriterAppender(layout, parent),
    mAppendFile(append),
    mBufferedIo(true),
    mFileName(fileName),
    mFile(nullptr),
    mTextStream(nullptr)
{
}


FileAppender::FileAppender(const LayoutSharedPtr &layout,
                           const QString &fileName,
                           bool append,
                           bool buffered,
                           QObject *parent) :
    WriterAppender(layout, parent),
    mAppendFile(append),
    mBufferedIo(buffered),
    mFileName(fileName),
    mFile(nullptr),
    mTextStream(nullptr)
{
}

FileAppender::~FileAppender()
{
    closeInternal();
}

QString FileAppender::file() const
{
    QMutexLocker locker(&mObjectGuard);
    return mFileName;
}

void FileAppender::setFile(const QString &fileName)
{
    QMutexLocker locker(&mObjectGuard);
    mFileName = fileName;
}

void FileAppender::activateOptions()
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
    WriterAppender::activateOptions();
}

void FileAppender::close()
{
    closeInternal();
    WriterAppender::close();

}

void FileAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    closeFile();
}

bool FileAppender::checkEntryConditions() const
{
    if (!mFile || !mTextStream)
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without open file"),
                                         AppenderNoOpenFileError);
        e << name();
        logger()->error(e);
        return false;
    }

    return WriterAppender::checkEntryConditions();
}

void FileAppender::closeFile()
{
    if (mFile)
        logger()->debug(u"Closing file '%1' for appender '%2'"_s, mFile->fileName(), name());

    setWriter(nullptr);
    mTextStream.reset();
    mFile.reset();
}

bool FileAppender::handleIoErrors() const
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


void FileAppender::openFile()
{
    Q_ASSERT_X(!mFile && !mTextStream, "FileAppender::openFile()", "Opening file without closing previous file");

#ifdef Q_OS_WIN
    // Let Windows resolve any environment variables in the file path BEFORE
    // the parent directory is derived and created below — otherwise a junk
    // directory literally named '%VAR%' is created while the expanded path's
    // parent never is. Query the required buffer size instead of relying on
    // MAX_PATH, which silently truncated long expansions.
    {
        const std::wstring rawPath = mFileName.toStdWString();
        const DWORD required = ExpandEnvironmentStringsW(rawPath.c_str(), nullptr, 0);
        if (required > 0)
        {
            std::wstring expanded(required, L'\0');
            if (ExpandEnvironmentStringsW(rawPath.c_str(), expanded.data(), required) > 0)
                mFileName = QString::fromWCharArray(expanded.c_str());
        }
    }
#endif

    QFileInfo file_info(mFileName);
    const QString parent_path = file_info.absolutePath();
    if (!QDir(parent_path).exists())
    {
        logger()->trace(u"Creating missing parent directory for file %1"_s, mFileName);
        if (!QDir().mkpath(parent_path))
        {
            LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to create parent directory '%1' for file '%2' of appender '%3'"),
                                             AppenderOpeningFileError);
            e << parent_path << mFileName << name();
            logger()->error(e);
            return;
        }
    }

    mFile = std::make_unique<QFile>(mFileName);
    QFile::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    if (mAppendFile)
        mode |= QIODevice::Append;
    else
        mode |= QIODevice::Truncate;
    if (!mBufferedIo)
        mode |= QIODevice::Unbuffered;
    if (!mFile->open(mode))
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to open file '%1' for appender '%2'"),
                                         AppenderOpeningFileError);
        e << mFileName << name();
        e.addCausingError(LogError(mFile->errorString(), mFile->error()));
        logger()->error(e);
        return;
    }
    // Skip the header when appending to a non-empty existing file —
    // the header is already present from the previous run.
    if (mAppendFile.load(std::memory_order_relaxed) && mFile->size() > 0)
        mSuppressNextHeader = true;
    mTextStream = std::make_unique<QTextStream>(mFile.get());
    setWriter(mTextStream.get());
    logger()->debug(u"Opened file '%1' for appender '%2'"_s, mFile->fileName(), name());
}


void FileAppender::writeHeader() const
{
    if (mSuppressNextHeader) {
        mSuppressNextHeader = false;
        return;
    }
    WriterAppender::writeHeader();
}

bool FileAppender::removeFile(QFile &file) const
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

bool FileAppender::renameFile(QFile &file,
                              const QString &fileName) const
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

} // namespace Log4Qt

#include "moc_fileappender.cpp"
