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

#include <QDebug>
#include <QFileSystemWatcher>
#include "helpers/initialisationhelper.h"

namespace Log4Qt
{

ConfiguratorHelper::ConfiguratorHelper() :
    mObjectGuard(),
    mConfigurationFile(),
    mpConfigureFunc(Q_NULLPTR),
    mpConfigurationFileWatch(Q_NULLPTR),
    mConfigureError()
{
}


ConfiguratorHelper::~ConfiguratorHelper()
{
    delete mpConfigurationFileWatch;
}


LOG4QT_IMPLEMENT_INSTANCE(ConfiguratorHelper)


void ConfiguratorHelper::doConfigurationFileChanged(const QString &rFileName)
{
    if (!mpConfigureFunc)
        return;
    mpConfigureFunc(rFileName);
    emit configurationFileChanged(rFileName, mConfigureError.count() > 0);
}

void ConfiguratorHelper::doSetConfigurationFile(const QString &rFileName,
        ConfigureFunc pConfigureFunc)
{
    QMutexLocker locker(&mObjectGuard);

    mConfigurationFile.clear();
    mpConfigureFunc = 0;
    delete mpConfigurationFileWatch;
    if (rFileName.isEmpty())
        return;

    mConfigurationFile = rFileName;
    mpConfigureFunc = pConfigureFunc;
    mpConfigurationFileWatch = new QFileSystemWatcher();
    if (mpConfigurationFileWatch->addPath(rFileName))
    {
        connect(mpConfigurationFileWatch,
                SIGNAL(fileChanged(const QString &)),
                SLOT(doConfigurationFileChanged(const QString &)));
    }
    else
        qWarning() << "Add Path '" << rFileName << "' to file system watcher failed!";
}

} // namespace Log4Qt

