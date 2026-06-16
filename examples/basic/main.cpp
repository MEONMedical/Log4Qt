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

#include "log4qt/logger.h"

#include "loggerobject.h"
#include "loggerobjectprio.h"
#include "loggerstatic.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/loggerrepository.h"
#include "log4qt/consoleappender.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/logmanager.h"
#include "log4qt/fileappender.h"
#include "log4qt/abstractlayout.h"
#include "log4qt/jsonlayout.h"
#include "log4qt/spi/headerfooterprovider.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QScopedPointer>
#include <QStringBuilder>
#include <QString>
#include <QFile>
#include <QLoggingCategory>

// Custom provider that writes the device serial number into every log file
// header. The serial number is typically read from hardware at application
// startup and injected once via setSerialNumber().
class SerialNumberHeaderProvider : public Log4Qt::HeaderFooterProvider
{
public:
    using Log4Qt::HeaderFooterProvider::HeaderFooterProvider;

    void setSerialNumber(const QString &serialNumber)
    {
        mSerialNumber = serialNumber;
    }

    QString header() const override
    {
        return QStringLiteral("Device S/N: ") + mSerialNumber;
    }

private:
    QString mSerialNumber;
};

// Provider for JSON log files: emits header and footer as JSON objects so
// the serial number and session timestamps are machine-readable.
class JsonSerialNumberHeaderProvider : public Log4Qt::HeaderFooterProvider
{
public:
    using Log4Qt::HeaderFooterProvider::HeaderFooterProvider;

    void setSerialNumber(const QString &serialNumber)
    {
        mSerialNumber = serialNumber;
    }

    QString header() const override
    {
        return QStringLiteral(R"({"event":"start","serialNumber":"%1","time":"%2"})")
            .arg(mSerialNumber,
                 QDateTime::currentDateTime().toString(Qt::ISODate));
    }

    QString footer() const override
    {
        return QStringLiteral(R"({"event":"end","serialNumber":"%1","time":"%2"})")
            .arg(mSerialNumber,
                 QDateTime::currentDateTime().toString(Qt::ISODate));
    }

private:
    QString mSerialNumber;
};

static void initializeRootLogger();
static void shutdownRootLogger();
static void logStartup();
static void logShutdown();
static void setupRootLogger(const QString &introMessage);
static void shutDownRootLogger(const QString &extroMessage);

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    const auto *object = new LoggerObject(&application);
    const auto *object1 = new LoggerObjectPrio(&application);
    Q_UNUSED(object1)
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                         "test.category1.debug=true");

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
    // Register the serial number provider before any appender is activated.
    // Every file opened after this point will have the serial number in its
    // header line.
    auto *provider = new SerialNumberHeaderProvider;
    provider->setSerialNumber(QStringLiteral("SN-20260001"));   // read from hardware in a real application
    Log4Qt::AbstractLayout::setGlobalHeaderFooterProvider(
        Log4Qt::HeaderFooterProviderSharedPtr(provider));

    // Create a layout
    auto logger = Log4Qt::Logger::rootLogger();
    auto *layout = new Log4Qt::TTCCLayout();
    layout->setName(QStringLiteral("My Layout"));
    layout->activateOptions();
    Log4Qt::LayoutSharedPtr layoutPtr(layout);
    // Create a console appender
    auto *consoleAppender = new Log4Qt::ConsoleAppender(layoutPtr, Log4Qt::ConsoleAppender::StdOut);
    consoleAppender->setName(QStringLiteral("My Appender"));
    consoleAppender->activateOptions();
    // Add appender on root logger
    logger->addAppender(Log4Qt::AppenderSharedPtr(consoleAppender));
    // Create a file appender
    auto *fileAppender = new Log4Qt::FileAppender(layoutPtr, QCoreApplication::applicationDirPath() + "/basic.log", true);
    fileAppender->setName(QStringLiteral("My file appender"));
    fileAppender->activateOptions();
    // Add appender on root logger
    logger->addAppender(Log4Qt::AppenderSharedPtr(fileAppender));
    // Create a JSON file appender — serial number and session timestamps
    // are written as JSON objects in the header and footer lines.
    auto *jsonSerialProvider = new JsonSerialNumberHeaderProvider;
    jsonSerialProvider->setSerialNumber(QStringLiteral("SN-20260001"));
    auto *jsonLayout = new Log4Qt::JsonLayout();
    jsonLayout->setName(QStringLiteral("My JSON Layout"));
    jsonLayout->setHeaderFooterProvider(Log4Qt::HeaderFooterProviderSharedPtr(jsonSerialProvider));
    jsonLayout->activateOptions();
    auto *jsonFileAppender = new Log4Qt::FileAppender(Log4Qt::LayoutSharedPtr(jsonLayout), QCoreApplication::applicationDirPath() + "/basic.json", true);
    jsonFileAppender->setName(QStringLiteral("My JSON file appender"));
    jsonFileAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(jsonFileAppender));

    // Set level to info
    logger->setLevel(Log4Qt::Level::INFO_INT);
    // Enable handling of Qt messages
    Log4Qt::LogManager::setHandleQtMessages(true);

    if (!introMessage.isEmpty())
        logger->info(introMessage);

    qWarning("Handling Qt messages is enabled");
}

void shutDownRootLogger(const QString &extroMessage)
{
    auto logger = Log4Qt::Logger::rootLogger();

    if (!extroMessage.isEmpty())
        logger->info(extroMessage);
    logger->removeAllAppenders();
    logger->loggerRepository()->shutdown();
    Log4Qt::AbstractLayout::setGlobalHeaderFooterProvider({});
}
