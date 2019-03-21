/******************************************************************************
 *
 * package:     Log4Qt
 * file:        configuratorhelper.cpp
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

#include "helpers/configuratorhelper.h"

#include "helpers/initialisationhelper.h"

#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>

namespace Log4Qt
{

ConfiguratorHelper::ConfiguratorHelper() :
    mConfigureFunc(nullptr),
    mConfigurationFileWatch(nullptr)
{
}


ConfiguratorHelper::~ConfiguratorHelper()
{
    delete mConfigurationFileWatch;
}

LOG4QT_IMPLEMENT_INSTANCE(ConfiguratorHelper)

void ConfiguratorHelper::doConfigurationFileChanged(const QString &fileName)
{
    if (!mConfigureFunc ||
        !QFileInfo::exists(mConfigurationFile.absoluteFilePath()))
        return;
    mConfigureFunc(fileName);
    emit configurationFileChanged(fileName, mConfigureError.count() > 0);
}

void ConfiguratorHelper::doConfigurationFileDirectoryChanged(const QString &path)
{
    Q_UNUSED(path)
    QTimer::singleShot(100, this, &ConfiguratorHelper::tryToReAddConfigurationFile);
}

void ConfiguratorHelper::tryToReAddConfigurationFile()
{
    if (!mConfigurationFileWatch->files().contains(mConfigurationFile.absoluteFilePath()))
        mConfigurationFileWatch->addPath(mConfigurationFile.absoluteFilePath());
}

void ConfiguratorHelper::doSetConfigurationFile(const QString &fileName,
        ConfigureFunc pConfigureFunc)
{
    QMutexLocker locker(&mObjectGuard);
    mConfigurationFile.setFile(fileName);
    mConfigureFunc = nullptr;
    delete mConfigurationFileWatch;
    mConfigurationFileWatch = nullptr;
    if (fileName.isEmpty() || !QFileInfo::exists(fileName))
        return;

    mConfigureFunc = pConfigureFunc;
    mConfigurationFileWatch = new QFileSystemWatcher();

    if (mConfigurationFileWatch->addPath(mConfigurationFile.absoluteFilePath()))
    {
        mConfigurationFileWatch->addPath(mConfigurationFile.absolutePath());
        connect(mConfigurationFileWatch, &QFileSystemWatcher::fileChanged,
                this, &ConfiguratorHelper::doConfigurationFileChanged);
        connect(mConfigurationFileWatch, &QFileSystemWatcher::directoryChanged,
                this, &ConfiguratorHelper::doConfigurationFileDirectoryChanged);
    }
    else
        qWarning() << "Add Path '" << fileName << "' to file system watcher failed!";
}

} // namespace Log4Qt

#include "moc_configuratorhelper.cpp"
