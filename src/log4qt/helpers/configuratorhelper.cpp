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

#include "helpers/configuratorhelper.h"

#include "helpers/initialisationhelper.h"

#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>

namespace Log4Qt
{

ConfiguratorHelper::ConfiguratorHelper(QObject *parent) :
    QObject(parent),
    mConfigureFunc(nullptr),
    mConfigurationFileWatch(nullptr)
{
}


ConfiguratorHelper::~ConfiguratorHelper() = default;

LOG4QT_IMPLEMENT_INSTANCE(ConfiguratorHelper)

void ConfiguratorHelper::doConfigurationFileChanged(const QString &fileName)
{
    ConfigureFunc func = nullptr;
    QString configPath;
    {
        QMutexLocker locker(&mObjectGuard);
        func = mConfigureFunc;
        configPath = mConfigurationFile.absoluteFilePath();
    }

    if (!func || !QFileInfo::exists(configPath))
        return;

    // Run the user callback outside the lock; it may call back into
    // setConfigureError() and would self-deadlock on a non-recursive mutex.
    func(fileName);

    bool hadError;
    {
        QMutexLocker locker(&mObjectGuard);
        hadError = !mConfigureError.isEmpty();
    }
    Q_EMIT configurationFileChanged(fileName, hadError);
}

void ConfiguratorHelper::doConfigurationFileDirectoryChanged([[maybe_unused]] const QString &path)
{
    QTimer::singleShot(100, this, &ConfiguratorHelper::tryToReAddConfigurationFile);
}

void ConfiguratorHelper::tryToReAddConfigurationFile()
{
    QMutexLocker locker(&mObjectGuard);
    if (!mConfigurationFileWatch)
        return;
    const QString path = mConfigurationFile.absoluteFilePath();
    if (!mConfigurationFileWatch->files().contains(path))
        mConfigurationFileWatch->addPath(path);
}

void ConfiguratorHelper::doSetConfigurationFile(const QString &fileName,
        ConfigureFunc pConfigureFunc)
{
    QMutexLocker locker(&mObjectGuard);
    mConfigurationFile.setFile(fileName);
    mConfigureFunc = nullptr;
    if (mConfigurationFileWatch)
    {
        // Retire the old watcher via deleteLater: it lives in the helper's
        // thread (see below), so a plain delete from the calling thread
        // would destroy a QObject cross-thread — and could destroy it while
        // its own fileChanged emission is still on the call stack.
        auto *oldWatch = mConfigurationFileWatch.release();
        oldWatch->disconnect(this);
        oldWatch->deleteLater();
    }
    if (fileName.isEmpty() || !QFileInfo::exists(fileName))
        return;

    mConfigureFunc = pConfigureFunc;
    mConfigurationFileWatch = std::make_unique<QFileSystemWatcher>();

    if (mConfigurationFileWatch->addPath(mConfigurationFile.absoluteFilePath()))
    {
        mConfigurationFileWatch->addPath(mConfigurationFile.absolutePath());
        connect(mConfigurationFileWatch.get(), &QFileSystemWatcher::fileChanged,
                this, &ConfiguratorHelper::doConfigurationFileChanged);
        connect(mConfigurationFileWatch.get(), &QFileSystemWatcher::directoryChanged,
                this, &ConfiguratorHelper::doConfigurationFileDirectoryChanged);
        // The watcher must live in the helper's thread: change notifications
        // need a running event loop in the watcher's thread — the thread
        // calling configureAndWatch() may have none or may exit — and
        // tryToReAddConfigurationFile() runs on the helper's thread and
        // calls watcher methods directly.
        mConfigurationFileWatch->moveToThread(thread());
    }
    else
        qWarning() << "Add Path '" << fileName << "' to file system watcher failed!";
}

} // namespace Log4Qt

#include "moc_configuratorhelper.cpp"
