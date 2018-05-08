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
    explicit BinaryFileAppender(QObject *parent = nullptr);
    BinaryFileAppender(const QString &fileName,
                       QObject *parent = nullptr);
    BinaryFileAppender(const QString &fileName,
                       bool append,
                       QObject *parent = nullptr);
    BinaryFileAppender(const QString &fileName,
                       bool append,
                       bool buffered,
                       QObject *parent = nullptr);
    virtual ~BinaryFileAppender();

    // properties
    bool appendFile() const;
    void setAppendFile(bool append);
    bool bufferedIo() const;
    void setBufferedIo(bool buffered);
    QString file() const;
    void setFile(const QString &fileName);
    QDataStream::ByteOrder byteOrder() const;
    void setByteOrder(QDataStream::ByteOrder byteorder);
    QDataStream::FloatingPointPrecision floatingPointPrecision() const;
    void setFloatingPointPrecision(QDataStream::FloatingPointPrecision floatingpointprecision);
    QDataStream::Version streamVersion() const;
    void setStreamVersion(QDataStream::Version version);

    // public members
    void activateOptions() override;
    void close() override;

protected:
    bool checkEntryConditions() const override;
    bool handleIoErrors() const override;

    void closeFile();
    void openFile();
    bool removeFile(QFile &file) const;
    bool renameFile(QFile &file, const QString &fileName) const;

private:
    BinaryFileAppender(const BinaryFileAppender &other); // Not implemented
    BinaryFileAppender &operator=(const BinaryFileAppender &other); // Not implemented
    void createDataStream();

    volatile bool mAppendFile;
    volatile bool mBufferedIo;
    QString mFileName;
    QFile *mFile;
    QDataStream *mDataStream;
    QDataStream::ByteOrder mByteOrder;
    QDataStream::FloatingPointPrecision mFloatingPointPrecision;
    QDataStream::Version mStreamVersion;
    void closeInternal();
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

inline void BinaryFileAppender::setFile(const QString &fileName)
{
    QMutexLocker locker(&mObjectGuard);
    mFileName = fileName;
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
