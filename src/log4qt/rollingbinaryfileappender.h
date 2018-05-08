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
    explicit RollingBinaryFileAppender(QObject *parent = nullptr);

    enum DatePattern
    {
        MINUTELY_ROLLOVER = 0,  /*! The minutely date pattern string is "'.'yyyy-MM-dd-hh-mm". */
        HOURLY_ROLLOVER,        /*! The hourly date pattern string is "'.'yyyy-MM-dd-hh". */
        HALFDAILY_ROLLOVER,     /*! The half-daily date pattern string is "'.'yyyy-MM-dd-a". */
        DAILY_ROLLOVER,         /*! The daily date pattern string is "'.'yyyy-MM-dd". */
        WEEKLY_ROLLOVER,        /*! The weekly date pattern string is "'.'yyyy-ww". */
        MONTHLY_ROLLOVER        /*! The monthly date pattern string is "'.'yyyy-MM". */
    };
    Q_ENUM(DatePattern)

    int maxBackupIndex() const;
    void setMaxBackupIndex(int maxBackupIndex);
    qint64 maximumFileSize() const;
    void setMaximumFileSize(qint64 maximumFileSize);
    QString datePattern() const;
    void setDatePattern(DatePattern datePattern);
    void setDatePattern(const QString &datePattern);

protected:
    void append(const LoggingEvent &event) override;
    void activateOptions() override;

    bool checkFotimeRollOver() const;
    void rollOvetime();
    bool checkForSizeRollOver() const;
    void rollOverSize();

private:
    void computeFrequency();
    void computeRollOvetime();
    QString frequencyToString() const;

    int mMaxBackupIndex;
    qint64 mMaximumFileSize;

    QString mDatePattern;
    DatePattern mFrequency;
    QString mActiveDatePattern;
    QDateTime mRollOvetime;
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

inline void RollingBinaryFileAppender::setDatePattern(const QString &datePattern)
{
    mDatePattern = datePattern;
}

}

#endif // LOG4QT_ROLLINGBINARYFILEAPPENDER_H
