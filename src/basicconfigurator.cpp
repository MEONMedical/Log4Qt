/******************************************************************************
 *
 * package:
 * file:        basicconfigurator.cpp
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


#include "basicconfigurator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include "consoleappender.h"
#include "helpers/configuratorhelper.h"
#include "helpers/logobjectptr.h"
#include "logmanager.h"
#include "patternlayout.h"
#include "varia/listappender.h"



namespace Log4Qt
{


bool BasicConfigurator::configure()
{
    LogObjectPtr<ListAppender> list = new ListAppender;
    list->setName(QLatin1String("BasicConfigurator"));
    list->setConfiguratorList(true);
    list->setThreshold(Level::ERROR_INT);
    LogManager::logLogger()->addAppender(list);

    PatternLayout *p_layout = new PatternLayout(PatternLayout::TTCC_CONVERSION_PATTERN);
    p_layout->setName(QLatin1String("BasicConfigurator TTCC"));
    p_layout->activateOptions();
    ConsoleAppender *p_appender = new ConsoleAppender(p_layout, ConsoleAppender::STDOUT_TARGET);
    p_appender->setName(QLatin1String("BasicConfigurator stdout"));
    p_appender->activateOptions();
    LogManager::rootLogger()->addAppender(p_appender);

    LogManager::logLogger()->removeAppender(list);
    ConfiguratorHelper::setConfigureError(list->list());
    return (list->list().count() == 0);
}


void BasicConfigurator::configure(Appender *pAppender)
{
    LogManager::rootLogger()->addAppender(pAppender);
}


void BasicConfigurator::resetConfiguration()
{
    LogManager::resetConfiguration();
}


} // namespace Log4Qt
