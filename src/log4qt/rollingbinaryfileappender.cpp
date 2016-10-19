#include "rollingbinaryfileappender.h"

#include "layout.h"
#include "helpers/datetime.h"

#include <QMetaEnum>
#include <QFile>

namespace Log4Qt
{

RollingBinaryFileAppender::RollingBinaryFileAppender(QObject *parent)
    : BinaryFileAppender(parent)
    , mMaxBackupIndex(1)
    , mMaximumFileSize(10 * 1024 * 1024)
    , mFrequency(DAILY_ROLLOVER)
{
    setDatePattern(DAILY_ROLLOVER);
}

void RollingBinaryFileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    computeFrequency();
    if (!mActiveDatePattern.isEmpty())
        computeRollOverTime();

    BinaryFileAppender::activateOptions();
}

void RollingBinaryFileAppender::computeFrequency()
{
    const DateTime start_time(QDate(1999, 1, 1), QTime(0, 0));
    const QString start_string = start_time.toString(mDatePattern);
    mActiveDatePattern.clear();

    if (start_string != static_cast<DateTime>(start_time.addSecs(60)).toString(mDatePattern))
        mFrequency = MINUTELY_ROLLOVER;
    else if (start_string != static_cast<DateTime>(start_time.addSecs(60 * 60)).toString(mDatePattern))
        mFrequency = HOURLY_ROLLOVER;
    else if (start_string != static_cast<DateTime>(start_time.addSecs(60 * 60 * 12)).toString(mDatePattern))
        mFrequency = HALFDAILY_ROLLOVER;
    else if (start_string != static_cast<DateTime>(start_time.addDays(1)).toString(mDatePattern))
        mFrequency = DAILY_ROLLOVER;
    else if (start_string != static_cast<DateTime>(start_time.addDays(7)).toString(mDatePattern))
        mFrequency = WEEKLY_ROLLOVER;
    else if (start_string != static_cast<DateTime>(start_time.addMonths(1)).toString(mDatePattern))
        mFrequency = MONTHLY_ROLLOVER;
    else
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("The pattern '%1' does not specify a frequency for appender '%2'"),
                                         APPENDER_INVALID_PATTERN_ERROR);
        e << mDatePattern << name();
        logger()->error(e);
        return;
    }

    mActiveDatePattern = mDatePattern;
    logger()->trace("Frequency set to %2 using date pattern %1", mActiveDatePattern, frequencyToString());
}


void RollingBinaryFileAppender::append(const LoggingEvent &rEvent)
{
    if (checkForTimeRollOver())
        rollOverTime();

    BinaryFileAppender::append(rEvent);

    if (checkForSizeRollOver())
        rollOverSize();
}

bool RollingBinaryFileAppender::checkForTimeRollOver() const
{
    return QDateTime::currentDateTime() > mRollOverTime;
}

bool RollingBinaryFileAppender::checkForSizeRollOver() const
{
    return writer()->device()->size() > mMaximumFileSize;
}

void RollingBinaryFileAppender::rollOverSize()
{
    logger()->debug("Rolling over with maxBackupIndex = %1", mMaxBackupIndex);

    // close current file first
    closeFile();

    // remove highest file first, eg. file.log.10
    QFile f;
    f.setFileName(file() + QLatin1Char('.') + QString::number(mMaxBackupIndex));
    if (f.exists() && !removeFile(f))
        return;

    // now move all lower files one step up, e.g. file.log.9 --> file.log.10, file.log.8 --> file.log.9, ..., file.log.1 --> file.log.2
    QString target_file_name;
    int i;
    for (i = mMaxBackupIndex - 1; i >= 1; i--)
    {
        f.setFileName(file() + QLatin1Char('.') + QString::number(i));
        if (f.exists())
        {
            target_file_name = file() + QLatin1Char('.') + QString::number(i + 1);
            if (!renameFile(f, target_file_name))
                return;
        }
    }

    // Now renmae the current file from file.log --> file.log.1
    f.setFileName(file());
    target_file_name = file() + QLatin1String(".1");
    if (!renameFile(f, target_file_name))
        return;

    // open a new file
    openFile();
}

void RollingBinaryFileAppender::rollOverTime()
{
    Q_ASSERT_X(!mActiveDatePattern.isEmpty(), "DailyRollingFileAppender::rollOver()", "No active date pattern");

    QString roll_over_suffix = mRollOverSuffix;
    computeRollOverTime();
    if (roll_over_suffix == mRollOverSuffix)
        return;

    closeFile();

    QString target_file_name = file() + roll_over_suffix;
    QFile f(target_file_name);
    if (f.exists() && !removeFile(f))
        return;
    f.setFileName(file());
    if (!renameFile(f, target_file_name))
        return;
    openFile();
}

void RollingBinaryFileAppender::setDatePattern(DatePattern datePattern)
{
    switch (datePattern)
    {
    case MINUTELY_ROLLOVER:
        setDatePattern(QLatin1String("'.'yyyy-MM-dd-hh-mm"));
        break;
    case HOURLY_ROLLOVER:
        setDatePattern(QLatin1String("'.'yyyy-MM-dd-hh"));
        break;
    case HALFDAILY_ROLLOVER:
        setDatePattern(QLatin1String("'.'yyyy-MM-dd-a"));
        break;
    case DAILY_ROLLOVER:
        setDatePattern(QLatin1String("'.'yyyy-MM-dd"));
        break;
    case WEEKLY_ROLLOVER:
        setDatePattern(QLatin1String("'.'yyyy-ww"));
        break;
    case MONTHLY_ROLLOVER:
        setDatePattern(QLatin1String("'.'yyyy-MM"));
        break;
    default:
        Q_ASSERT_X(false, "BinaryFileAppender::setDatePattern()", "Invalid datePattern constant");
        setDatePattern(DAILY_ROLLOVER);
        break;
    };
}

void RollingBinaryFileAppender::computeRollOverTime()
{
    Q_ASSERT_X(!mActiveDatePattern.isEmpty(), "BinaryFileAppender::computeRollOverTime()", "No active date pattern");

    QDateTime now = QDateTime::currentDateTime();
    QDate now_date = now.date();
    QTime now_time = now.time();
    QDateTime start;

    switch (mFrequency)
    {
    case MINUTELY_ROLLOVER:
        start = QDateTime(now_date, QTime(now_time.hour(), now_time.minute(), 0, 0));
        mRollOverTime = start.addSecs(60);
        break;
    case HOURLY_ROLLOVER:
        start = QDateTime(now_date, QTime(now_time.hour(), 0, 0, 0));
        mRollOverTime = start.addSecs(60 * 60);
        break;
    case HALFDAILY_ROLLOVER:
    {
        int hour = now_time.hour();
        if (hour >=  12)
            hour = 12;
        else
            hour = 0;
        start = QDateTime(now_date, QTime(hour, 0, 0, 0));
        mRollOverTime = start.addSecs(60 * 60 * 12);
    }
    break;
    case DAILY_ROLLOVER:
        start = QDateTime(now_date, QTime(0, 0, 0, 0));
        mRollOverTime = start.addDays(1);
        break;
    case WEEKLY_ROLLOVER:
    {
        // QT numbers the week days 1..7. The week starts on Monday.
        // Change it to being numbered 0..6, starting with Sunday.
        int day = now_date.dayOfWeek();
        if (day == Qt::Sunday)
            day = 0;
        start = QDateTime(now_date, QTime(0, 0, 0, 0)).addDays(-1 * day);
        mRollOverTime = start.addDays(7);
    }
    break;
    case MONTHLY_ROLLOVER:
        start = QDateTime(QDate(now_date.year(), now_date.month(), 1), QTime(0, 0, 0, 0));
        mRollOverTime = start.addMonths(1);
        break;
    default:
        Q_ASSERT_X(false, "BinaryFileAppender::computeInterval()", "Invalid datePattern constant");
        mRollOverTime = QDateTime::fromTime_t(0);
        break;
    }

    mRollOverSuffix = static_cast<DateTime>(start).toString(mActiveDatePattern);
    Q_ASSERT_X(static_cast<DateTime>(now).toString(mActiveDatePattern) == mRollOverSuffix,
               "BinaryFileAppender::computeRollOverTime()", "File name changes within interval");
    Q_ASSERT_X(mRollOverSuffix != static_cast<DateTime>(mRollOverTime).toString(mActiveDatePattern),
               "BinaryFileAppender::computeRollOverTime()", "File name does not change with rollover");

    logger()->trace("Computing roll over time from %1: The interval start time is %2. The roll over time is %3",
                    now.toString(),
                    start.toString(),
                    mRollOverTime.toString());
}

QString RollingBinaryFileAppender::frequencyToString() const
{
    QMetaEnum meta_enum = metaObject()->enumerator(metaObject()->indexOfEnumerator("DatePattern"));
    return QLatin1String(meta_enum.valueToKey(mFrequency));
}

}
