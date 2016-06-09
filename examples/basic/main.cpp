#include "logger.h"

#include "loggerobject.h"
#include "loggerobjectprio.h"
#include "loggerstatic.h"
#include "propertyconfigurator.h"
#include "loggerrepository.h"
#include "consoleappender.h"
#include "logger.h"
#include "ttcclayout.h"
#include "logmanager.h"
#include "fileappender.h"

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

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    LoggerObject *object = new LoggerObject(&application);
    LoggerObjectPrio *object1 = new LoggerObjectPrio(&application);
    Q_UNUSED(object1)

    QObject::connect(object, &LoggerObject::exit, &application, &QCoreApplication::exit);

    initializeRootLogger();
    logStartup();

    int ret;
    {
        QScopedPointer<LoggerStatic> object2(new LoggerStatic());

        ret = application.exec();
    }

    logShutdown();
    shutdownRootLogger();

    return ret;
}

void initializeRootLogger()
{
    setupRootLogger("Root logger is setup.");
}

void shutdownRootLogger()
{
    shutDownRootLogger("Root logger was shutdown.");
}

void logStartup()
{
    auto logger = Log4Qt::Logger::rootLogger();

    logger->info("################################################################");
    logger->info("#                          START                               #");
    logger->info("################################################################");
}

void logShutdown()
{
    auto logger = Log4Qt::Logger::rootLogger();

    logger->info("################################################################");
    logger->info("#                          STOP                                #");
    logger->info("################################################################");
}

void setupRootLogger(const QString &introMessage)
{
    // Create a layout
    auto logger = Log4Qt::Logger::rootLogger();
    Log4Qt::TTCCLayout *layout = new Log4Qt::TTCCLayout();
    layout->setName(QLatin1String("My Layout"));
    layout->activateOptions();
    // Create a console appender
    Log4Qt::ConsoleAppender *consoleAppender = new Log4Qt::ConsoleAppender(layout, Log4Qt::ConsoleAppender::STDOUT_TARGET);
    consoleAppender->setName(QLatin1String("My Appender"));
    consoleAppender->activateOptions();
    // Add appender on root logger
    logger->addAppender(consoleAppender);
    // Create a file appender
    Log4Qt::FileAppender *fileAppender = new Log4Qt::FileAppender(layout, QCoreApplication::applicationDirPath() + "/basic.log", true);
    fileAppender->setName(QLatin1String("My file appender"));
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
