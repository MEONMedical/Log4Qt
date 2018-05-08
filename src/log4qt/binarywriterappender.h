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
    BinaryWriterAppender(QObject *parent = nullptr);
    BinaryWriterAppender(QDataStream *dataStream, QObject *parent = nullptr);
    ~BinaryWriterAppender() override;

    bool requiresLayout() const override;
    QDataStream *writer() const;

    void setWriter(QDataStream *dataStream);

    void activateOptions() override;
    void close() override;

protected:
    void append(const LoggingEvent &event) override;
    bool checkEntryConditions() const override;

    void closeWriter();

    virtual bool handleIoErrors() const;
    void writeFooter() const;
    void writeHeader() const;

private:
    Q_DISABLE_COPY(BinaryWriterAppender)
    void writeRawData(const QByteArray &data) const;
    void closeInternal();
    BinaryLayout *binaryLayout() const;

    QDataStream *mWriter;
};

inline QDataStream *BinaryWriterAppender::writer() const
{
    return mWriter;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYWRITTERAPPENDER_H
