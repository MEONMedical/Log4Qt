/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#include "log4qt/logger.h"

#include "loggerobject.h"
#include "loggerobjectprio.h"
#include "loggerstatic.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/loggerrepository.h"

#include <QCoreApplication>
#include <QScopedPointer>
#include <QStringBuilder>
#include <QString>
#include <QFile>

static void initializeRootLogger();
static void shutdownRootLogger();
static void logStartup();
static void logShutdown();
static void setupRootLogger(const QString &introMessage);
static void shutDownRootLogger(const QString &extroMessage);


// Shows how to configure logging with a property file
int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    const auto *object = new LoggerObject(&application);
    const auto *object1 = new LoggerObjectPrio(&application);
    Q_UNUSED(object1)

    QObject::connect(object, &LoggerObject::exit, &application, &QCoreApplication::exit);

    initializeRootLogger();
    logStartup();

    int ret;
    {
        QScopedPointer<LoggerStatic> object2(new LoggerStatic());

        ret = QCoreApplication::exec();
    }

    logShutdown();
    shutdownRootLogger();

    return ret;
}

void initializeRootLogger()
{
    setupRootLogger(QStringLiteral("Root logger is setup."));
}

void shutdownRootLogger()
{
    shutDownRootLogger(QStringLiteral("Root logger was shutdown."));
}

void logStartup()
{
    auto logger = Log4Qt::Logger::rootLogger();

    logger->info(QStringLiteral("################################################################"));
    logger->info(QStringLiteral("#                          START                               #"));
    logger->info(QStringLiteral("################################################################"));
}

void logShutdown()
{
    auto logger = Log4Qt::Logger::rootLogger();

    logger->info(QStringLiteral("################################################################"));
    logger->info(QStringLiteral("#                          STOP                                #"));
    logger->info(QStringLiteral("################################################################"));
}

void setupRootLogger(const QString &introMessage)
{
    QString configFile = QCoreApplication::applicationFilePath() + QStringLiteral(".log4qt.properties");
    if (QFile::exists(configFile))
        Log4Qt::PropertyConfigurator::configureAndWatch(configFile);
    if (!introMessage.isEmpty())
        Log4Qt::Logger::rootLogger()->info(introMessage);
}

void shutDownRootLogger(const QString &extroMessage)
{
    auto logger = Log4Qt::Logger::rootLogger();

    if (!extroMessage.isEmpty())
        logger->info(extroMessage);
    logger->removeAllAppenders();
    logger->loggerRepository()->shutdown();
}
