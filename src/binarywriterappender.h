#ifndef LOG4QT_BINARYWRITTERAPPENDER_H
#define LOG4QT_BINARYWRITTERAPPENDER_H

#include "appenderskeleton.h"

#include <QDataStream>

namespace Log4Qt
{

class BinaryLayout;

class LOG4QT_EXPORT BinaryWriterAppender : public AppenderSkeleton
{
    Q_OBJECT
    Q_PROPERTY(QDataStream* writer READ writer WRITE setWriter)
public:
    BinaryWriterAppender(QObject *pParent = 0);
    BinaryWriterAppender(QDataStream *pDataStream, QObject *pParent = 0);
    virtual ~BinaryWriterAppender();

    virtual bool requiresLayout() const;
    QDataStream *writer() const;

    void setWriter(QDataStream *pDataStream);

    virtual void activateOptions();
    virtual void close();

protected:
    virtual void append(const LoggingEvent &rEvent);
    virtual bool checkEntryConditions() const;

    void closeWriter();
#ifndef QT_NO_DEBUG_STREAM
    virtual QDebug debug(QDebug &rDebug) const;
#endif // QT_NO_DEBUG_STREAM

    virtual bool handleIoErrors() const;
    void writeFooter() const;
    void writeHeader() const;

private:
    Q_DISABLE_COPY(BinaryWriterAppender)
    void writeRawData(const QByteArray &data) const;
    BinaryLayout *binaryLayout() const;

    QDataStream *mpWriter;
};

inline QDataStream *BinaryWriterAppender::writer() const
{
    return mpWriter;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYWRITTERAPPENDER_H
