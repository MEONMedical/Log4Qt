#ifndef LOG4QT_ROLLINGBINARYFILEAPPENDER_H
#define LOG4QT_ROLLINGBINARYFILEAPPENDER_H

#include "binaryfileappender.h"

#include <QDateTime>

namespace Log4Qt
{

class RollingBinaryFileAppender : public BinaryFileAppender
{
    Q_OBJECT
    Q_PROPERTY(int maxBackupIndex READ maxBackupIndex WRITE setMaxBackupIndex)
    Q_PROPERTY(qint64 maximumFileSize READ maximumFileSize WRITE setMaximumFileSize)
    Q_PROPERTY(QString datePattern READ datePattern WRITE setDatePattern)
public:
    explicit RollingBinaryFileAppender(QObject *parent = Q_NULLPTR);

    enum DatePattern
    {
        MINUTELY_ROLLOVER = 0,  /*! The minutely date pattern string is "'.'yyyy-MM-dd-hh-mm". */
        HOURLY_ROLLOVER,        /*! The hourly date pattern string is "'.'yyyy-MM-dd-hh". */
        HALFDAILY_ROLLOVER,     /*! The half-daily date pattern string is "'.'yyyy-MM-dd-a". */
        DAILY_ROLLOVER,         /*! The daily date pattern string is "'.'yyyy-MM-dd". */
        WEEKLY_ROLLOVER,        /*! The weekly date pattern string is "'.'yyyy-ww". */
        MONTHLY_ROLLOVER        /*! The monthly date pattern string is "'.'yyyy-MM". */
    };
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    Q_ENUMS(DatePattern)
#else
    Q_ENUM(DatePattern)
#endif

    int maxBackupIndex() const;
    void setMaxBackupIndex(int maxBackupIndex);
    qint64 maximumFileSize() const;
    void setMaximumFileSize(qint64 maximumFileSize);
    QString datePattern() const;
    void setDatePattern(DatePattern datePattern);
    void setDatePattern(const QString &rDatePattern);

protected:
    virtual void append(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;
    virtual void activateOptions() Q_DECL_OVERRIDE;

    virtual bool checkForTimeRollOver() const;
    virtual void rollOverTime();
    virtual bool checkForSizeRollOver() const;
    virtual void rollOverSize();

private:
    void computeFrequency();
    void computeRollOverTime();
    QString frequencyToString() const;

    int mMaxBackupIndex;
    qint64 mMaximumFileSize;

    QString mDatePattern;
    DatePattern mFrequency;
    QString mActiveDatePattern;
    QDateTime mRollOverTime;
    QString mRollOverSuffix;
};


inline int RollingBinaryFileAppender::maxBackupIndex() const
{
    return mMaxBackupIndex;
}

inline qint64 RollingBinaryFileAppender::maximumFileSize() const
{
    return mMaximumFileSize;
}

inline void RollingBinaryFileAppender::setMaxBackupIndex(int maxBackupIndex)
{
    mMaxBackupIndex = maxBackupIndex;
}

inline void RollingBinaryFileAppender::setMaximumFileSize(qint64 maximumFileSize)
{
    mMaximumFileSize = maximumFileSize;
}

inline QString RollingBinaryFileAppender::datePattern() const
{
    return mDatePattern;
}

inline void RollingBinaryFileAppender::setDatePattern(const QString &rDatePattern)
{
    mDatePattern = rDatePattern;
}

}

#endif // LOG4QT_ROLLINGBINARYFILEAPPENDER_H
