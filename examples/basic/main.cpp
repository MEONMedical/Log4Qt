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

#include <QCoreApplication>
#include <QScopedPointer>
#include <QStringBuilder>
#include <QString>
#include <QFile>
#include <QLoggingCategory>

static void initializeRootLogger();
static void shutdownRootLogger();
static void logStartup();
static void logShutdown();
static void setupRootLogger(const QString &introMessage);
static void shutDownRootLogger(const QString &extroMessage);

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    auto *object = new LoggerObject(&application);
    auto *object1 = new LoggerObjectPrio(&application);
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
    // Create a layout
    auto logger = Log4Qt::Logger::rootLogger();
    auto *layout = new Log4Qt::TTCCLayout();
    layout->setName(QStringLiteral("My Layout"));
    layout->activateOptions();
    // Create a console appender
    Log4Qt::ConsoleAppender *consoleAppender = new Log4Qt::ConsoleAppender(layout, Log4Qt::ConsoleAppender::STDOUT_TARGET);
    consoleAppender->setName(QStringLiteral("My Appender"));
    consoleAppender->activateOptions();
    // Add appender on root logger
    logger->addAppender(consoleAppender);
    // Create a file appender
    Log4Qt::FileAppender *fileAppender = new Log4Qt::FileAppender(layout, QCoreApplication::applicationDirPath() + "/basic.log", true);
    fileAppender->setName(QStringLiteral("My file appender"));
    fileAppender->activateOptions();
    // Add appender on root logger
    logger->addAppender(fileAppender);

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
}
