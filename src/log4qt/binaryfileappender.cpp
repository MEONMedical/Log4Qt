#include "binaryfileappender.h"
#include "binaryloggingevent.h"

#include "layout.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

// if we are in WIN*
#if defined(__WIN32__) || defined(WIN) || defined(WIN32) || defined(Q_OS_WIN32)
#include <windows.h>
#endif

namespace Log4Qt
{

BinaryFileAppender::BinaryFileAppender(QObject *pParent) :
    BinaryWriterAppender(pParent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(),
    mpFile(0),
    mpDataStream(0),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::BinaryFileAppender(const QString &rFileName, QObject *pParent) :
    BinaryWriterAppender(pParent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(rFileName),
    mpFile(0),
    mpDataStream(0),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::BinaryFileAppender(const QString &rFileName, bool append, QObject *pParent) :
    BinaryWriterAppender(pParent),
    mAppendFile(append),
    mBufferedIo(true),
    mFileName(rFileName),
    mpFile(0),
    mpDataStream(0),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::BinaryFileAppender(const QString &rFileName, bool append, bool buffered, QObject *pParent) :
    BinaryWriterAppender(pParent),
    mAppendFile(append),
    mBufferedIo(buffered),
    mFileName(rFileName),
    mpFile(0),
    mpDataStream(0),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::~BinaryFileAppender()
{
    close();
}

void BinaryFileAppender::activateOptions()
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
    BinaryWriterAppender::activateOptions();
}

void BinaryFileAppender::close()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    BinaryWriterAppender::close();
    closeFile();
}

bool BinaryFileAppender::checkEntryConditions() const
{
    if (!mpFile || !mpDataStream)
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without open file"),
                                         APPENDER_NO_OPEN_FILE_ERROR);
        e << name();
        logger()->error(e);
        return false;
    }

    return BinaryWriterAppender::checkEntryConditions();
}


void BinaryFileAppender::closeFile()
{
    if (mpFile)
        logger()->debug("Closing file '%1' for appender '%2'", mpFile->fileName(), name());

    setWriter(0);
    delete mpDataStream;
    mpDataStream = 0;
    delete mpFile;
    mpFile = 0;
}

bool BinaryFileAppender::handleIoErrors() const
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

void BinaryFileAppender::createDataStream()
{
    mpDataStream = new QDataStream(mpFile);
    mpDataStream->setByteOrder(mByteOrder);
    mpDataStream->setFloatingPointPrecision(mFloatingPointPrecision);
    mpDataStream->setVersion(mStreamVersion);
}

void BinaryFileAppender::openFile()
{
    Q_ASSERT_X(mpFile == 0 && mpDataStream == 0, "BinaryFileAppender::openFile()", "Opening file without closing previous file");

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

    createDataStream();
    setWriter(mpDataStream);
    logger()->debug("Opened file '%1' for appender '%2'", mpFile->fileName(), name());
}

bool BinaryFileAppender::removeFile(QFile &rFile) const
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

bool BinaryFileAppender::renameFile(QFile &rFile,
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
