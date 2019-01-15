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

#include "helpers/datetime.h"
#include "layout.h"
#include "loggingevent.h"

#include <QFile>
#include <QMetaEnum>
#include <QTextCodec>
#include <QFileInfo>
#include <QStringBuilder>

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
{
}

DailyFileAppender::DailyFileAppender(const LayoutSharedPtr &layout, const QString &fileName, const QString &datePattern, QObject *parent)
    : FileAppender(layout, fileName, parent)
    , mDateRetriever(new DefaultDateRetriever)
    , mDatePattern(datePattern.isEmpty() ? defaultDatePattern : datePattern)
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

void DailyFileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    closeFile();
    setLogFileForCurrentDay();
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

    if (mDateRetriever->currentDate() != mLastDate)
        rollOver();
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
