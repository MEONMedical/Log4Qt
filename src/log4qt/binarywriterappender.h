#ifndef LOG4QT_BINARYWRITTERAPPENDER_H
#define LOG4QT_BINARYWRITTERAPPENDER_H

#include "appenderskeleton.h"

class QDataStream;
namespace Log4Qt
{

class BinaryLayout;

class LOG4QT_EXPORT BinaryWriterAppender : public AppenderSkeleton
{
    Q_OBJECT
    Q_PROPERTY(QDataStream *writer READ writer WRITE setWriter)
public:
    BinaryWriterAppender(QObject *pParent = nullptr);
    BinaryWriterAppender(QDataStream *pDataStream, QObject *pParent = nullptr);
    virtual ~BinaryWriterAppender();

    virtual bool requiresLayout() const override;
    QDataStream *writer() const;

    void setWriter(QDataStream *pDataStream);

    virtual void activateOptions() override;
    virtual void close() override;

protected:
    virtual void append(const LoggingEvent &rEvent) override;
    virtual bool checkEntryConditions() const override;

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
