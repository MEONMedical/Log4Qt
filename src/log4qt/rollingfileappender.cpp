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

namespace Log4Qt
{

RollingFileAppender::RollingFileAppender(QObject *parent) :
    FileAppender(parent),
    mMaxBackupIndex(1),
    mMaximumFileSize(10 * 1024 * 1024)
{
}

RollingFileAppender::RollingFileAppender(const LayoutSharedPtr &layout,
        const QString &fileName,
        QObject *parent) :
    FileAppender(layout, fileName, parent),
    mMaxBackupIndex(1),
    mMaximumFileSize(10 * 1024 * 1024)
{
}

RollingFileAppender::RollingFileAppender(const LayoutSharedPtr &layout,
        const QString &fileName,
        bool append,
        QObject *parent) :
    FileAppender(layout, fileName, append, parent),
    mMaxBackupIndex(1),
    mMaximumFileSize(10 * 1024 * 1024)
{
}

void RollingFileAppender::setMaxFileSize(const QString &maxFileSize)
{
    bool ok;
    qint64 max_file_size = OptionConverter::toFileSize(maxFileSize, &ok);
    if (ok)
        setMaximumFileSize(max_file_size);
}

void RollingFileAppender::append(const LoggingEvent &event)
{
    FileAppender::append(event);
    if (writer()->device()->size() > this->mMaximumFileSize)
        rollOver();
}

void RollingFileAppender::openFile()
{
    // if we do not append, we roll the file to avoid data loss
    if (appendFile())
        FileAppender::openFile();
    else
        rollOver();
}

void RollingFileAppender::rollOver()
{
    logger()->debug(QStringLiteral("Rolling over with maxBackupIndex = %1"), mMaxBackupIndex);

    closeFile();

    QFile f;
    f.setFileName(file() + QLatin1Char('.') + QString::number(mMaxBackupIndex));
    if (f.exists() && !removeFile(f))
        return;

    for (int i = mMaxBackupIndex - 1; i >= 1; i--)
    {
        f.setFileName(file() + QLatin1Char('.') + QString::number(i));
        if (f.exists())
        {
            const QString target_file_name = file() + QLatin1Char('.') + QString::number(i + 1);
            if (!renameFile(f, target_file_name))
                return;
        }
    }

    f.setFileName(file());
    // it may not exist on first startup, don't output a warning in this case
    if (f.exists())
    {
        const QString target_file_name = file() + QStringLiteral(".1");
        if (!renameFile(f, target_file_name))
            return;
    }

    FileAppender::openFile();
}

} // namespace Log4Qt

#include "moc_rollingfileappender.cpp"
