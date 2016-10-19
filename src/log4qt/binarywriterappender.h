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
    Q_PROPERTY(QDataStream *writer READ writer WRITE setWriter)
public:
    BinaryWriterAppender(QObject *pParent = Q_NULLPTR);
    BinaryWriterAppender(QDataStream *pDataStream, QObject *pParent = Q_NULLPTR);
    virtual ~BinaryWriterAppender();

    virtual bool requiresLayout() const Q_DECL_OVERRIDE;
    QDataStream *writer() const;

    void setWriter(QDataStream *pDataStream);

    virtual void activateOptions() Q_DECL_OVERRIDE;
    virtual void close() Q_DECL_OVERRIDE;

protected:
    virtual void append(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;
    virtual bool checkEntryConditions() const Q_DECL_OVERRIDE;

    void closeWriter();

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
