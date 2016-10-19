#ifndef LOG4QT_BINARYFILEAPPENDER_H
#define LOG4QT_BINARYFILEAPPENDER_H

#include "binarywriterappender.h"

#include <QDataStream>

class QFile;

namespace Log4Qt
{

class LOG4QT_EXPORT BinaryFileAppender : public BinaryWriterAppender
{
    Q_OBJECT
    Q_PROPERTY(bool appendFile READ appendFile WRITE setAppendFile)
    Q_PROPERTY(bool bufferedIo READ bufferedIo WRITE setBufferedIo)
    Q_PROPERTY(QString file READ file WRITE setFile)
    Q_PROPERTY(QDataStream::ByteOrder byteOrder READ byteOrder WRITE setByteOrder)
    Q_PROPERTY(QDataStream::FloatingPointPrecision floatingPointPrecision READ floatingPointPrecision WRITE setFloatingPointPrecision)
    Q_PROPERTY(QDataStream::Version streamVersion READ streamVersion WRITE setStreamVersion)

public:
    explicit BinaryFileAppender(QObject *pParent = Q_NULLPTR);
    BinaryFileAppender(const QString &rFileName,
                       QObject *pParent = Q_NULLPTR);
    BinaryFileAppender(const QString &rFileName,
                       bool append,
                       QObject *pParent = Q_NULLPTR);
    BinaryFileAppender(const QString &rFileName,
                       bool append,
                       bool buffered,
                       QObject *pParent = Q_NULLPTR);
    virtual ~BinaryFileAppender();

    // properties

    bool appendFile() const;
    void setAppendFile(bool append);
    bool bufferedIo() const;
    void setBufferedIo(bool buffered);
    QString file() const;
    void setFile(const QString &rFileName);
    QDataStream::ByteOrder byteOrder() const;
    void setByteOrder(QDataStream::ByteOrder byteorder);
    QDataStream::FloatingPointPrecision floatingPointPrecision() const;
    void setFloatingPointPrecision(QDataStream::FloatingPointPrecision floatingpointprecision);
    QDataStream::Version streamVersion() const;
    void setStreamVersion(QDataStream::Version version);

    // public members
    virtual void activateOptions() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;

protected:
    virtual bool checkEntryConditions() const Q_DECL_OVERRIDE;
    virtual bool handleIoErrors() const Q_DECL_OVERRIDE;

    void closeFile();
    void openFile();
    bool removeFile(QFile &rFile) const;
    bool renameFile(QFile &rFile, const QString &rFileName) const;

private:
    BinaryFileAppender(const BinaryFileAppender &rOther); // Not implemented
    BinaryFileAppender &operator=(const BinaryFileAppender &rOther); // Not implemented
    void createDataStream();

    volatile bool mAppendFile;
    volatile bool mBufferedIo;
    QString mFileName;
    QFile *mpFile;
    QDataStream *mpDataStream;
    QDataStream::ByteOrder mByteOrder;
    QDataStream::FloatingPointPrecision mFloatingPointPrecision;
    QDataStream::Version mStreamVersion;
};

inline bool BinaryFileAppender::appendFile() const
{
    return mAppendFile;
}

inline QString BinaryFileAppender::file() const
{
    QMutexLocker locker(&mObjectGuard);
    return mFileName;
}

inline bool BinaryFileAppender::bufferedIo() const
{
    return mBufferedIo;
}

inline void BinaryFileAppender::setAppendFile(bool append)
{
    mAppendFile = append;
}

inline void BinaryFileAppender::setBufferedIo(bool buffered)
{
    mBufferedIo = buffered;
}

inline void BinaryFileAppender::setFile(const QString &rFileName)
{
    QMutexLocker locker(&mObjectGuard);
    mFileName = rFileName;
}

inline QDataStream::ByteOrder BinaryFileAppender::byteOrder() const
{
    return mByteOrder;
}

inline void BinaryFileAppender::setByteOrder(QDataStream::ByteOrder byteorder)
{
    mByteOrder = byteorder;
}

inline QDataStream::FloatingPointPrecision BinaryFileAppender::floatingPointPrecision() const
{
    return mFloatingPointPrecision;
}

inline void BinaryFileAppender::setFloatingPointPrecision(QDataStream::FloatingPointPrecision floatingpointprecision)
{
    mFloatingPointPrecision = floatingpointprecision;
}

inline QDataStream::Version BinaryFileAppender::streamVersion() const
{
    return mStreamVersion;
}

inline void BinaryFileAppender::setStreamVersion(QDataStream::Version version)
{
    mStreamVersion = version;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYFILEAPPENDER_H
