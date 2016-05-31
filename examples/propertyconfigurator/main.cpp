#include "logger.h"

#include "loggerobject.h"
#include "loggerobjectprio.h"
#include "loggerstatic.h"
#include "propertyconfigurator.h"
#include "loggerrepository.h"

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
    auto configFile = QCoreApplication::applicationFilePath() % QStringLiteral(".log4qt.properties");
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
