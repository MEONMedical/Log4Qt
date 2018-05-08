#ifndef LOG4QT_BINARYLAYOUT_H
#define LOG4QT_BINARYLAYOUT_H

#include "layout.h"

#include <QByteArray>

namespace Log4Qt
{

class LoggingEvent;
class BinaryLoggingEvent;

class LOG4QT_EXPORT BinaryLayout : public Layout
{
    Q_OBJECT
    Q_PROPERTY(QByteArray binaryFooter READ binaryFooter WRITE setBinaryFooter)
    Q_PROPERTY(QByteArray binaryHeader READ binaryHeader WRITE setBinaryHeader)

public:
    explicit BinaryLayout(QObject *parent = nullptr);

    virtual QByteArray binaryFormat(const BinaryLoggingEvent &event) const;
    virtual QString format(const LoggingEvent &event) override;

    virtual QString contentType() const override;

    virtual QByteArray binaryFooter() const;
    void setBinaryFooter(const QByteArray &footer);

    virtual QByteArray binaryHeader() const;
    void setBinaryHeader(const QByteArray &header);

private:
    Q_DISABLE_COPY(BinaryLayout)

    QByteArray mFooter;
    QByteArray mHeader;
};

inline QByteArray BinaryLayout::binaryFooter() const
{
    return mFooter;
}

inline void BinaryLayout::setBinaryFooter(const QByteArray &footer)
{
    mFooter = footer;
}

inline QByteArray BinaryLayout::binaryHeader() const
{
    return mHeader;
}

inline void BinaryLayout::setBinaryHeader(const QByteArray &header)
{
    mHeader = header;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYLAYOUT_H
