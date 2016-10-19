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
    explicit BinaryLayout(QObject *parent = Q_NULLPTR);

    virtual QByteArray binaryFormat(const BinaryLoggingEvent &rEvent);
    virtual QString format(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;

    virtual QString contentType() const Q_DECL_OVERRIDE;

    virtual QByteArray binaryFooter() const;
    void setBinaryFooter(const QByteArray &rFooter);

    virtual QByteArray binaryHeader() const;
    void setBinaryHeader(const QByteArray &rHeader);

private:
    Q_DISABLE_COPY(BinaryLayout)

    QByteArray mFooter;
    QByteArray mHeader;
};

inline QByteArray BinaryLayout::binaryFooter() const
{
    return mFooter;
}

inline void BinaryLayout::setBinaryFooter(const QByteArray &rFooter)
{
    mFooter = rFooter;
}

inline QByteArray BinaryLayout::binaryHeader() const
{
    return mHeader;
}

inline void BinaryLayout::setBinaryHeader(const QByteArray &rHeader)
{
    mHeader = rHeader;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYLAYOUT_H
