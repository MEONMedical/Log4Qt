/******************************************************************************
 *
 * package:
 * file:        fileappender.cpp
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


#include "fileappender.h"
#include "layout.h"
#include "loggingevent.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTextCodec>

// if we are in WIN*
#if defined(__WIN32__) || defined(WIN) || defined(WIN32) || defined(Q_OS_WIN32)
#include <windows.h>
#endif

namespace Log4Qt
{

FileAppender::FileAppender(QObject *pParent) :
    WriterAppender(pParent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(),
    mpFile(Q_NULLPTR),
    mpTextStream(Q_NULLPTR)
{
}


FileAppender::FileAppender(LayoutSharedPtr pLayout,
                           const QString &rFileName,
                           QObject *pParent) :
    WriterAppender(pLayout, pParent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(rFileName),
    mpFile(Q_NULLPTR),
    mpTextStream(Q_NULLPTR)
{
}


FileAppender::FileAppender(LayoutSharedPtr pLayout,
                           const QString &rFileName,
                           bool append,
                           QObject *pParent) :
    WriterAppender(pLayout, pParent),
    mAppendFile(append),
    mBufferedIo(true),
    mFileName(rFileName),
    mpFile(Q_NULLPTR),
    mpTextStream(Q_NULLPTR)
{
}


FileAppender::FileAppender(LayoutSharedPtr pLayout,
                           const QString &rFileName,
                           bool append,
                           bool buffered,
                           QObject *pParent) :
    WriterAppender(pLayout, pParent),
    mAppendFile(append),
    mBufferedIo(buffered),
    mFileName(rFileName),
    mpFile(Q_NULLPTR),
    mpTextStream(Q_NULLPTR)
{
}


FileAppender::~FileAppender()
{
    close();
}


void FileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    if (mFileName.isEmpty())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Activation of Appender '%1' that requires file and has no file set"),
                                         APPENDER_ACTIVATE_MISSING_FILE_ERROR);
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
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    WriterAppender::close();
    closeFile();
}


bool FileAppender::checkEntryConditions() const
{
    if (!mpFile || !mpTextStream)
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without open file"),
                                         APPENDER_NO_OPEN_FILE_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }

    return WriterAppender::checkEntryConditions();
}


void FileAppender::closeFile()
{
    if (mpFile)
        logger()->debug("Closing file '%1' for appender '%2'", mpFile->fileName(), name());

    setWriter(Q_NULLPTR);
    delete mpTextStream;
    mpTextStream = 0;
    delete mpFile;
    mpFile = 0;
}

bool FileAppender::handleIoErrors() const
{
    if (mpFile->error() == QFile::NoError)
        return false;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to write to file '%1' for appender '%2'"),
                                     APPENDER_WRITING_FILE_ERROR);
    e << mFileName << name();
    e.addCausingError(LogError(mpFile->errorString(), mpFile->error()));
    logger()->error(e);
    return true;
}


void FileAppender::openFile()
{
    Q_ASSERT_X(mpFile == 0 && mpTextStream == 0, "FileAppender::openFile()", "Opening file without closing previous file");

    QFileInfo file_info(mFileName);
    QDir parent_dir = file_info.dir();
    if (!parent_dir.exists())
    {
        logger()->trace("Creating missing parent directory for file %1", mFileName);
        QString name = parent_dir.dirName();
        parent_dir.cdUp();
        parent_dir.mkdir(name);
    }

#if defined(__WIN32__) || defined(WIN) || defined(WIN32) || defined(Q_OS_WIN32)
    // Let windows resolve any environment variables included in the file path
    wchar_t buffer[MAX_PATH];
    if (ExpandEnvironmentStringsW(mFileName.toStdWString().c_str(), buffer, MAX_PATH))
        mFileName = QString::fromWCharArray(buffer);
#endif

    mpFile = new QFile(mFileName);
    QFile::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    if (mAppendFile)
        mode |= QIODevice::Append;
    else
        mode |= QIODevice::Truncate;
    if (!mBufferedIo)
        mode |= QIODevice::Unbuffered;
    if (!mpFile->open(mode))
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to open file '%1' for appender '%2'"),
                                         APPENDER_OPENING_FILE_ERROR);
        e << mFileName << name();
        e.addCausingError(LogError(mpFile->errorString(), mpFile->error()));
        logger()->error(e);
        return;
    }
    mpTextStream = new QTextStream(mpFile);
    setWriter(mpTextStream);
    logger()->debug("Opened file '%1' for appender '%2'", mpFile->fileName(), name());
}


bool FileAppender::removeFile(QFile &rFile) const
{
    if (rFile.remove())
        return true;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to remove file '%1' for appender '%2'"),
                                     APPENDER_REMOVE_FILE_ERROR);
    e << rFile.fileName() << name();
    e.addCausingError(LogError(rFile.errorString(), rFile.error()));
    logger()->error(e);
    return false;
}


bool FileAppender::renameFile(QFile &rFile,
                              const QString &rFileName) const
{
    logger()->debug("Renaming file '%1' to '%2'", rFile.fileName(), rFileName);
    if (rFile.rename(rFileName))
        return true;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to rename file '%1' to '%2' for appender '%3'"),
                                     APPENDER_RENAMING_FILE_ERROR);
    e << rFile.fileName() << rFileName << name();
    e.addCausingError(LogError(rFile.errorString(), rFile.error()));
    logger()->error(e);
    return false;
}

} // namespace Log4Qt
