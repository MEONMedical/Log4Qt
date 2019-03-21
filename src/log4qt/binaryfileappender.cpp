#include "binaryfileappender.h"
#include "binaryloggingevent.h"

#include "layout.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

// if we are in WIN*
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Log4Qt
{

BinaryFileAppender::BinaryFileAppender(QObject *parent) :
    BinaryWriterAppender(parent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(),
    mFile(nullptr),
    mDataStream(nullptr),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::BinaryFileAppender(const QString &fileName, QObject *parent) :
    BinaryWriterAppender(parent),
    mAppendFile(false),
    mBufferedIo(true),
    mFileName(fileName),
    mFile(nullptr),
    mDataStream(nullptr),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::BinaryFileAppender(const QString &fileName, bool append, QObject *parent) :
    BinaryWriterAppender(parent),
    mAppendFile(append),
    mBufferedIo(true),
    mFileName(fileName),
    mFile(nullptr),
    mDataStream(nullptr),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::BinaryFileAppender(const QString &fileName, bool append, bool buffered, QObject *parent) :
    BinaryWriterAppender(parent),
    mAppendFile(append),
    mBufferedIo(buffered),
    mFileName(fileName),
    mFile(nullptr),
    mDataStream(nullptr),
    mByteOrder(QDataStream::LittleEndian),
    mFloatingPointPrecision(QDataStream::DoublePrecision),
    mStreamVersion(QDataStream::Qt_5_3)
{
}

BinaryFileAppender::~BinaryFileAppender()
{
    closeInternal();
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
    closeInternal();
    BinaryWriterAppender::close();
}

void BinaryFileAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    closeFile();
}

bool BinaryFileAppender::checkEntryConditions() const
{
    if ((mFile == nullptr) || (mDataStream == nullptr))
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
    if (mFile != nullptr)
        logger()->debug(QStringLiteral("Closing file '%1' for appender '%2'"), mFile->fileName(), name());

    setWriter(nullptr);
    delete mDataStream;
    mDataStream = nullptr;
    delete mFile;
    mFile = nullptr;
}

bool BinaryFileAppender::handleIoErrors() const
{
    if (mFile->error() == QFile::NoError)
        return false;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to write to file '%1' for appender '%2'"),
                                     APPENDER_WRITING_FILE_ERROR);
    e << mFileName << name();
    e.addCausingError(LogError(mFile->errorString(), mFile->error()));
    logger()->error(e);
    return true;
}

void BinaryFileAppender::createDataStream()
{
    mDataStream = new QDataStream(mFile);
    mDataStream->setByteOrder(mByteOrder);
    mDataStream->setFloatingPointPrecision(mFloatingPointPrecision);
    mDataStream->setVersion(mStreamVersion);
}

void BinaryFileAppender::openFile()
{
    Q_ASSERT_X(mFile == nullptr && mDataStream == nullptr, "BinaryFileAppender::openFile()", "Opening file without closing previous file");

    QFileInfo file_info(mFileName);
    QDir parent_dir = file_info.dir();
    if (!parent_dir.exists())
    {
        logger()->trace(QStringLiteral("Creating missing parent directory for file %1"), mFileName);
        QString name = parent_dir.dirName();
        parent_dir.cdUp();
        parent_dir.mkdir(name);
    }

#ifdef Q_OS_WIN
    // Let windows resolve any environment variables included in the file path
    wchar_t buffer[MAX_PATH];
    if (ExpandEnvironmentStringsW(mFileName.toStdWString().c_str(), buffer, MAX_PATH))
        mFileName = QString::fromWCharArray(buffer);
#endif

    mFile = new QFile(mFileName);
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
                                         APPENDER_OPENING_FILE_ERROR);
        e << mFileName << name();
        e.addCausingError(LogError(mFile->errorString(), mFile->error()));
        logger()->error(e);
        return;
    }

    createDataStream();
    setWriter(mDataStream);
    logger()->debug(QStringLiteral("Opened file '%1' for appender '%2'"), mFile->fileName(), name());
}

bool BinaryFileAppender::removeFile(QFile &file) const
{
    if (file.remove())
        return true;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to remove file '%1' for appender '%2'"),
                                     APPENDER_REMOVE_FILE_ERROR);
    e << file.fileName() << name();
    e.addCausingError(LogError(file.errorString(), file.error()));
    logger()->error(e);
    return false;
}

bool BinaryFileAppender::renameFile(QFile &file,
                                    const QString &fileName) const
{
    logger()->debug(QStringLiteral("Renaming file '%1' to '%2'"), file.fileName(), fileName);
    if (file.rename(fileName))
        return true;

    LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Unable to rename file '%1' to '%2' for appender '%3'"),
                                     APPENDER_RENAMING_FILE_ERROR);
    e << file.fileName() << fileName << name();
    e.addCausingError(LogError(file.errorString(), file.error()));
    logger()->error(e);
    return false;
}

} // namespace Log4Qt

#include "moc_binaryfileappender.cpp"
