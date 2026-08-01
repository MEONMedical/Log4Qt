/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
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

#include "abstractlayout.h"
#include "loggingevent.h"
#include "spi/compositetriggeringpolicy.h"
#include "spi/defaultrolloverstrategy.h"

#include <QFile>
#include <QFileInfo>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

RollingFileAppender::RollingFileAppender(QObject *parent) :
    FileAppender(parent)
{
}

RollingFileAppender::RollingFileAppender(const LayoutSharedPtr &layout,
        const QString &fileName,
        QObject *parent) :
    FileAppender(layout, fileName, parent)
{
}

RollingFileAppender::RollingFileAppender(const LayoutSharedPtr &layout,
        const QString &fileName,
        bool append,
        QObject *parent) :
    FileAppender(layout, fileName, append, parent)
{
}

void RollingFileAppender::setTriggeringPolicy(const TriggeringPolicySharedPtr &policy)
{
    QMutexLocker locker(&mObjectGuard);
    mTriggeringPolicy = policy;
}

void RollingFileAppender::addTriggeringPolicy(const TriggeringPolicySharedPtr &policy)
{
    QMutexLocker locker(&mObjectGuard);

    if (!mTriggeringPolicy)
    {
        mTriggeringPolicy = policy;
    }
    else if (auto *composite = qobject_cast<CompositeTriggeringPolicy *>(mTriggeringPolicy.data()))
    {
        composite->addPolicy(policy);
    }
    else
    {
        auto *comp = new CompositeTriggeringPolicy;
        comp->addPolicy(mTriggeringPolicy);
        comp->addPolicy(policy);
        mTriggeringPolicy = TriggeringPolicySharedPtr(comp);
    }
}

void RollingFileAppender::setRolloverStrategy(const RolloverStrategySharedPtr &strategy)
{
    QMutexLocker locker(&mObjectGuard);
    mRolloverStrategy = strategy;
}

void RollingFileAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    // Default strategy if none set
    if (!mRolloverStrategy)
        mRolloverStrategy = RolloverStrategySharedPtr(new DefaultRolloverStrategy);

    if (mTriggeringPolicy)
        mTriggeringPolicy->activateOptions();
    mRolloverStrategy->activateOptions();

    // Remember the configured base filename. Rollovers always operate on the
    // base name so that strategies never see an already-transformed filename.
    // On re-activation, only adopt a name the user changed: file() may hold
    // the strategy-transformed active name from a previous activation or
    // rollover (e.g. a dated filename), and deriving the base from it would
    // stack the transformation ('app.2026-08-01.2026-08-01.log').
    if (mBaseFileName.isEmpty() || file() != mActiveFileName)
        mBaseFileName = file();

    // Allow the strategy to set the initial active filename (e.g. date-embedded name)
    // before the file is opened, so the correct name is used from the very first startup.
    const QString initial = mRolloverStrategy->initialFileName(mBaseFileName);
    if (initial != file())
        setFile(initial);
    mActiveFileName = initial;

    // Check startup trigger BEFORE opening the file — openFile() may truncate
    bool startupRollover = false;
    if (mTriggeringPolicy)
    {
        qint64 fileSize = 0;
        QFileInfo fi(file());
        if (fi.exists())
            fileSize = fi.size();
        startupRollover = mTriggeringPolicy->isStartupTrigger(file(), fileSize);
    }

    if (startupRollover)
    {
        // The previous run's file must be archived by rollOver() below.
        // Do not let the first open truncate it (appendFile defaults to
        // false) — open it in append mode, then roll over.
        const bool configuredAppend = appendFile();
        setAppendFile(true);
        FileAppender::activateOptions();
        setAppendFile(configuredAppend);

        if (mSkipFooterOnStartup)
            suppressNextFooter();
        rollOver();
    }
    else
    {
        FileAppender::activateOptions();
    }
}

void RollingFileAppender::append(const LoggingEvent &event)
{
    FileAppender::append(event);
    if (mTriggeringPolicy)
    {
        if (mTriggeringPolicy->isTriggeringEvent(writer()->device(), event))
            rollOver();
    }
}

void RollingFileAppender::rollOver()
{
    logger()->debug(u"Rolling over with strategy %1"_s,
                    QLatin1String(mRolloverStrategy->metaObject()->className()));

    closeFile();
    const QString baseName = mBaseFileName.isEmpty() ? file() : mBaseFileName;
    QString nextFile = mRolloverStrategy->rollover(baseName);
    if (nextFile != file())
        setFile(nextFile);
    mActiveFileName = nextFile;

    // If the file to be opened still exists, its content has not been
    // archived — either a rename/remove failed during the rollover (e.g. the
    // file is locked by another process) or the strategy reuses a dated file
    // for the current period. Open it in append mode in that case: truncating
    // would silently destroy the un-archived log content.
    if (!appendFile() && QFile::exists(file()))
    {
        setAppendFile(true);
        FileAppender::openFile();
        setAppendFile(false);
    }
    else
    {
        FileAppender::openFile();
    }
}

} // namespace Log4Qt

#include "moc_rollingfileappender.cpp"
