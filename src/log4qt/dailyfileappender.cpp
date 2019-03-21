/******************************************************************************
 *
 * package:
 * file:        dailyfileappender.cpp
 * created:     Jaenner 2015
 * author:      Johann Anhofer
 *
 *
 * Copyright 2007 Martin Heinrich
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

#include "dailyfileappender.h"

#include "layout.h"
#include "loggingevent.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QtConcurrentRun>

namespace Log4Qt
{

IDateRetriever::~IDateRetriever() = default;

QDate DefaultDateRetriever::currentDate() const
{
    return QDate::currentDate();
}

static const char defaultDatePattern[] = "_yyyy_MM_dd";

DailyFileAppender::DailyFileAppender(QObject *parent)
    : FileAppender(parent)
    , mDateRetriever(new DefaultDateRetriever)
    , mDatePattern(defaultDatePattern)
    , mKeepDays(0)
{
}

DailyFileAppender::DailyFileAppender(const LayoutSharedPtr &layout, const QString &fileName, const QString &datePattern, const int keepDays, QObject *parent)
    : FileAppender(layout, fileName, parent)
    , mDateRetriever(new DefaultDateRetriever)
    , mDatePattern(datePattern.isEmpty() ? defaultDatePattern : datePattern)
    , mKeepDays(keepDays)
{
}

QString DailyFileAppender::datePattern() const
{
    QMutexLocker locker(&mObjectGuard);
    return mDatePattern;
}

void DailyFileAppender::setDatePattern(const QString &datePattern)
{
    QMutexLocker locker(&mObjectGuard);
    mDatePattern = datePattern;
}

int DailyFileAppender::keepDays() const
{
    QMutexLocker locker(&mObjectGuard);
    return mKeepDays;
}

void DailyFileAppender::setKeepDays(const int keepDays)
{
    QMutexLocker locker(&mObjectGuard);
    mKeepDays = keepDays;
}

namespace
{

void deleteObsoleteFiles(
        QDate currentDate,
        const QString &datePattern,
        int keepDays,
        const QString &originalFilename)
{
    if (keepDays <= 0) return;
    if (originalFilename.isEmpty()) return;

    const QFileInfo fi(originalFilename);
    const QDir logDir(fi.absolutePath());
    const auto logFileNames(
                logDir.entryList(
                    QStringList(QStringLiteral("*.") + fi.completeSuffix()),
                    QDir::NoSymLinks | QDir::Files));

    const QRegularExpression creationDateExtractor(
                fi.baseName() % QStringLiteral("(.*)") % QStringLiteral(".") % fi.completeSuffix());

    const auto startOfLogging(currentDate.addDays(-keepDays));

    QStringList obsoleteLogFileNames;

    for (const auto &fileName : logFileNames)
    {
        // determine creation date from file name instead of using file attributes, since file might
        // have been moved around, modified by user etc.
        const auto match(creationDateExtractor.match(fileName));
        if (match.hasMatch())
        {
            const auto creationDate(QDate::fromString(match.captured(1), datePattern));

            if (creationDate.isValid() && creationDate < startOfLogging)
            {
                obsoleteLogFileNames += fileName;
            }
        }
    }

    for (const auto &fileName : qAsConst(obsoleteLogFileNames))
    {
        QFile::remove(logDir.filePath(fileName));
    }
}

}

void DailyFileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    Q_ASSERT_X(mDateRetriever, "DailyFileAppender::append()", "No date retriever set");

    closeFile();
    setLogFileForCurrentDay();
    deleteObsoleteFiles(mDateRetriever->currentDate(), mDatePattern, mKeepDays, mOriginalFilename);
    FileAppender::activateOptions();
}

QString DailyFileAppender::appendDateToFilename() const
{
    QFileInfo fi(mOriginalFilename);
    return fi.absolutePath() % QStringLiteral("/") % fi.baseName() %  mLastDate.toString(mDatePattern) % QStringLiteral(".") % fi.completeSuffix();
}

void DailyFileAppender::append(const LoggingEvent &event)
{
    Q_ASSERT_X(mDateRetriever, "DailyFileAppender::append()", "No date retriever set");

    const auto currentDate(mDateRetriever->currentDate());

    if (currentDate != mLastDate)
    {
        rollOver();

        // schedule check for obsolete files for asynchronous execution, destructor will wait for
        // completion of each executor
        mDeleteObsoleteFilesExecutors.addFuture(
                    QtConcurrent::run(
                        deleteObsoleteFiles,
                        currentDate, mDatePattern, mKeepDays, mOriginalFilename));
    }
    FileAppender::append(event);
}

void DailyFileAppender::setDateRetriever(const QSharedPointer<const IDateRetriever> &dateRetriever)
{
    QMutexLocker locker(&mObjectGuard);

    mDateRetriever = dateRetriever;
}

void DailyFileAppender::setLogFileForCurrentDay()
{
    if (mOriginalFilename.isEmpty())
        mOriginalFilename = file();

    Q_ASSERT_X(mDateRetriever, "DailyFileAppender::setLogFileForCurrentDay()", "No date retriever set");

    mLastDate = mDateRetriever->currentDate();
    setFile(appendDateToFilename());
}

void DailyFileAppender::rollOver()
{
    closeFile();
    setLogFileForCurrentDay();
    openFile();
}

}

#include "moc_dailyfileappender.cpp"
