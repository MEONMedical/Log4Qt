/******************************************************************************
 *
 * package:     Log4Qt
 * file:        rollingfileappender.cpp
 * created:     September 2007
 * author:      Martin Heinrich
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

#include "rollingfileappender.h"

#include "helpers/optionconverter.h"
#include "layout.h"
#include "loggingevent.h"

#include <QFile>
#include <QTextCodec>

namespace Log4Qt
{

RollingFileAppender::RollingFileAppender(QObject *pParent) :
    FileAppender(pParent),
    mMaxBackupIndex(1),
    mMaximumFileSize(10 * 1024 * 1024)
{
}

RollingFileAppender::RollingFileAppender(LayoutSharedPtr pLayout,
        const QString &rFileName,
        QObject *pParent) :
    FileAppender(pLayout, rFileName, pParent),
    mMaxBackupIndex(1),
    mMaximumFileSize(10 * 1024 * 1024)
{
}

RollingFileAppender::RollingFileAppender(LayoutSharedPtr pLayout,
        const QString &rFileName,
        bool append,
        QObject *pParent) :
    FileAppender(pLayout, rFileName, append, pParent),
    mMaxBackupIndex(1),
    mMaximumFileSize(10 * 1024 * 1024)
{
}


RollingFileAppender::~RollingFileAppender()
{
    close();
}

void RollingFileAppender::setMaxFileSize(const QString &rMaxFileSize)
{
    bool ok;
    qint64 max_file_size = OptionConverter::toFileSize(rMaxFileSize, &ok);
    if (ok)
        setMaximumFileSize(max_file_size);
}

void RollingFileAppender::append(const LoggingEvent &rEvent)
{
    FileAppender::append(rEvent);
    if (writer()->device()->size() > this->mMaximumFileSize)
        rollOver();
}


void RollingFileAppender::rollOver()
{
    logger()->debug("Rolling over with maxBackupIndex = %1", mMaxBackupIndex);

    closeFile();

    QFile f;
    f.setFileName(file() + QLatin1Char('.') + QString::number(mMaxBackupIndex));
    if (f.exists() && !removeFile(f))
        return;

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

    f.setFileName(file());
    target_file_name = file() + QLatin1String(".1");
    if (!renameFile(f, target_file_name))
        return;

    openFile();
}

} // namespace Log4Qt
