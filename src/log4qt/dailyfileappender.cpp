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

static const char defaultDatePattern[] = "_yyyy_MM_dd";

DailyFileAppender::DailyFileAppender(QObject *pParent)
    : FileAppender(pParent)
    , mDatePattern(defaultDatePattern)
{
}

DailyFileAppender::DailyFileAppender(LayoutSharedPtr pLayout, const QString &rFileName, const QString &rDatePattern, QObject *pParent)
    : FileAppender(pLayout, rFileName, pParent)
    , mDatePattern(rDatePattern.isEmpty() ? defaultDatePattern : rDatePattern)
{
}

DailyFileAppender::~DailyFileAppender()
{
    close();
}

QString DailyFileAppender::datePattern() const
{
    QMutexLocker locker(&mObjectGuard);
    return mDatePattern;
}

void DailyFileAppender::setDatePattern(const QString &rDatePattern)
{
    QMutexLocker locker(&mObjectGuard);
    mDatePattern = rDatePattern;
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

void DailyFileAppender::append(const LoggingEvent &rEvent)
{
    if (QDate::currentDate() != mLastDate)
        rollOver();
    FileAppender::append(rEvent);
}

void DailyFileAppender::setLogFileForCurrentDay()
{
    if (mOriginalFilename.isEmpty())
        mOriginalFilename = file();

    mLastDate = QDate::currentDate();
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
