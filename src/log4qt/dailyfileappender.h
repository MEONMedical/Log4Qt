/******************************************************************************
 *
 * package:
 * file:        dailyfileappender.h
 * created:     Jaenner 2015
 * author:      Johann Anhofer
 *
 *
 * Copyright 2015 Johann Anhofer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#ifndef LOG4QT_DAILYFILEAPPENDER_H
#define LOG4QT_DAILYFILEAPPENDER_H

#include "fileappender.h"

#include <QDate>
#include <QFutureSynchronizer>
#include <QSharedPointer>
#include <QString>

namespace Log4Qt
{

class LOG4QT_EXPORT IDateRetriever
{
public:
    virtual ~IDateRetriever();
    virtual QDate currentDate() const = 0;
};

class LOG4QT_EXPORT DefaultDateRetriever final : public IDateRetriever
{
public:

    /**
     * Return the current date, as reported by the system clock.
     */
    QDate currentDate() const override;
};

/*!
 * \brief The class DailyFileAppender extends FileAppender so that the
 * a log file is created for each day
 */
class  LOG4QT_EXPORT DailyFileAppender : public FileAppender
{
    Q_OBJECT

    //! The property holds the date pattern used by the appender.
    Q_PROPERTY(QString datePattern READ datePattern WRITE setDatePattern)

    /**
     * Number of days that old log files will be kept on disk.
     * Set to a positive value to enable automatic deletion. Per default, all files are kept. Check
     * for obsolete files happens once a day.
     */
    Q_PROPERTY(int keepDays READ keepDays WRITE setKeepDays)

public:
    explicit DailyFileAppender(QObject *parent = nullptr);
    DailyFileAppender(const LayoutSharedPtr &layout, const QString &fileName, const QString &datePattern = QString(), int keepDays = 0, QObject *parent = nullptr);

    QString datePattern() const;
    void setDatePattern(const QString &datePattern);

    int keepDays() const;
    void setKeepDays(int keepDays);

    void activateOptions() override;

    void append(const LoggingEvent &event) override;

    void setDateRetriever(const QSharedPointer<const IDateRetriever> &dateRetriever);

private:
    Q_DISABLE_COPY(DailyFileAppender)

    void setLogFileForCurrentDay();
    void rollOver();
    QString appendDateToFilename() const;

    QSharedPointer<const IDateRetriever> mDateRetriever;

    QString mDatePattern;
    QDate mLastDate;
    int mKeepDays;
    QString mOriginalFilename;

    QFutureSynchronizer<void> mDeleteObsoleteFilesExecutors;
};

}

#endif // LOG4QT_DAILYFILEAPPENDER_H
