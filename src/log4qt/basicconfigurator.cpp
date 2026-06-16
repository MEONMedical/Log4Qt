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

#include "basicconfigurator.h"

#include "consoleappender.h"
#include "helpers/configuratorhelper.h"
#include "logmanager.h"
#include "patternlayout.h"
#include "varia/listappender.h"

#include <QCoreApplication>
#include <QThread>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

bool BasicConfigurator::configure()
{
    auto *list = new ListAppender;
    list->setName(u"BasicConfigurator"_s);
    list->setConfiguratorList(true);
    list->setThreshold(Level::ERROR_INT);
    AppenderSharedPtr listPtr(list);
    LogManager::logLogger()->addAppender(listPtr);

    LayoutSharedPtr p_layout(new PatternLayout(PatternLayout::TtccPattern));
    p_layout->setName(u"BasicConfigurator TTCC"_s);
    p_layout->activateOptions();
    auto *p_appender = new ConsoleAppender(p_layout, ConsoleAppender::StdOut);
    p_appender->setName(u"BasicConfigurator stdout"_s);
    p_appender->activateOptions();
    LogManager::rootLogger()->addAppender(AppenderSharedPtr(p_appender));

    LogManager::logLogger()->removeAppender(listPtr);
    ConfiguratorHelper::setConfigureError(list->list());
    return (list->list().size() == 0);
}

void BasicConfigurator::configure(Appender *pAppender)
{
    LogManager::rootLogger()->addAppender(AppenderSharedPtr(pAppender));
}

void BasicConfigurator::resetConfiguration()
{
    LogManager::resetConfiguration();
}

} // namespace Log4Qt
