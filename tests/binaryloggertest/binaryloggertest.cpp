#include <QtTest/QtTest>

#include "testappender.h"

#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/consoleappender.h"
#include "log4qt/loggerrepository.h"
#include "log4qt/simplelayout.h"
#include "log4qt/varia/levelrangefilter.h"
#include "log4qt/varia/denyallfilter.h"

#include "log4qt/varia/binaryeventfilter.h"
#include "log4qt/binarytotextlayout.h"
#include "log4qt/binarylayout.h"
#include "log4qt/binaryloggingevent.h"
#include "log4qt/binarywriterappender.h"
#include "log4qt/binaryfileappender.h"
#include "log4qt/binarylogstream.h"
#include "log4qt/binarylogger.h"

#include "atscopeexit.h"
#include "elementsinarray.h"

#include <QDataStream>

const auto loggingLevel = Log4Qt::Level::ALL_INT; // set to OFF_INT to enable logging to the console;
static const char binLogger[] = "binlogger";

class BinaryLoggerTest: public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_BINARYLOGGER
public:
    explicit BinaryLoggerTest(QObject *parent = nullptr)
        : QObject(parent)
        , mAppender{nullptr}
    {
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testBinaryToTextLayout();
    void testBinaryEventFilter();
    void testBinaryWriterAppender();
    void testBinaryFileAppender();
    void testBinaryLogStream();
    void testBinaryLogger();
    void testBinaryClassLogger();
    void testBinaryLayout();

private:
    TestAppender *mAppender;
    class AppenderData
    {
    public:
        void setAppender(Log4Qt::BinaryWriterAppender *newAppender)
        {
            appender = newAppender;
        }

        void reset()
        {
            data.clear();
            pds.reset(new QDataStream(&data, QIODevice::WriteOnly));
            pds->setByteOrder(QDataStream::LittleEndian);
            Q_ASSERT_X(!appender.isNull(), __FUNCTION__, "No Appender");
            appender->setWriter(pds.data());
            appender->activateOptions();
        }

        QByteArray flush()
        {
            return data;
        }

    private:
        QByteArray data;
        QScopedPointer<QDataStream> pds;
        QPointer<Log4Qt::BinaryWriterAppender> appender;
    } mAppenderData[3];
    QMap<QString, int> mLoggenameToAppenderDataIndex;

    AppenderData &getAppenderDataFromLogger(Log4Qt::Logger *plogger)
    {
        auto loggename = plogger->name();

        if (!mLoggenameToAppenderDataIndex.contains(loggename))
            mLoggenameToAppenderDataIndex.insert(loggename, mLoggenameToAppenderDataIndex.size());

        return mAppenderData[mLoggenameToAppenderDataIndex[loggename]];
    }

    void log(const QByteArray &data, const QString &loggename = QString{})
    {
        auto mylogger = Log4Qt::Logger::logger(loggename);
        Log4Qt::BinaryLoggingEvent event(mylogger, Log4Qt::Level::INFO_INT, data);
        mylogger->callAppenders(event);
    }

    void configureLogger(Log4Qt::Logger *mylogger)
    {
        auto &appenderData = getAppenderDataFromLogger(mylogger);
        auto writerAppender = new Log4Qt::BinaryWriterAppender(mylogger);
        writerAppender->setName(QString{QStringLiteral("Appender for '%1'")} .arg(mylogger->name()));
        appenderData.setAppender(writerAppender);
        mylogger->addAppender(writerAppender);
        mylogger->setAdditivity(false);
        mylogger->setLevel(Log4Qt::Level::ALL_INT);
    }

    QByteArray flushLogger(Log4Qt::Logger *mylogger)
    {
        return getAppenderDataFromLogger(mylogger).flush();
    }
};

LOG4QT_DECLARE_STATIC_BINARYLOGGER(unitTestLogger, StaticBinaryLogger)

void BinaryLoggerTest::initTestCase()
{
    Log4Qt::Logger *rootLogger = Log4Qt::Logger::rootLogger();
    Log4Qt::LogManager::setHandleQtMessages(true);

    Log4Qt::LayoutSharedPtr layout(new Log4Qt::TTCCLayout(rootLogger));
    static_cast<Log4Qt::TTCCLayout *>(layout.data())->setDateFormat(QStringLiteral("dd.MM.yyyy hh:mm:ss.zzz"));
    static_cast<Log4Qt::TTCCLayout *>(layout.data())->setContextPrinting(false);

    Log4Qt::LayoutSharedPtr binlayout(new Log4Qt::BinaryToTextLayout(layout, rootLogger));

    auto *consoleAppender = new Log4Qt::ConsoleAppender(rootLogger);
    consoleAppender->setLayout(binlayout);
    consoleAppender->setTarget(Log4Qt::ConsoleAppender::STDOUT_TARGET);
    consoleAppender->activateOptions();

    Log4Qt::Filter *denyall = new Log4Qt::DenyAllFilter;
    denyall->activateOptions();
    auto *levelFilter = new Log4Qt::LevelRangeFilter(rootLogger);
    levelFilter->setNext(Log4Qt::FilterSharedPtr(denyall));
    levelFilter->setLevelMin(Log4Qt::Level::NULL_INT);
    levelFilter->setLevelMax(loggingLevel);
    levelFilter->activateOptions();
    consoleAppender->addFilter(Log4Qt::FilterSharedPtr(levelFilter));
    rootLogger->addAppender(consoleAppender);

    // add appender for tests
    Log4Qt::LayoutSharedPtr simpleLayout(new Log4Qt::SimpleLayout(rootLogger));
    Log4Qt::LayoutSharedPtr binlayout1(new Log4Qt::BinaryToTextLayout(simpleLayout, rootLogger));
    auto *appender = new TestAppender(rootLogger);
    appender->setLayout(binlayout1);
    appender->activateOptions();
    mAppender = appender;
    rootLogger->addAppender(appender);

    configureLogger(Log4Qt::Logger::logger(binLogger));
    configureLogger(unitTestLogger());
    configureLogger(logger());

    QCOMPARE(rootLogger->metaObject()->className(), "Log4Qt::Logger");
    QCOMPARE(Log4Qt::Logger::logger(binLogger)->metaObject()->className(), "Log4Qt::Logger");
    QCOMPARE(unitTestLogger()->metaObject()->className(), "Log4Qt::BinaryLogger");
    QCOMPARE(logger()->metaObject()->className(), "Log4Qt::BinaryLogger");
}

void BinaryLoggerTest::cleanupTestCase()
{
    Log4Qt::Logger::logger(binLogger)->removeAllAppenders();
    unitTestLogger()->removeAllAppenders();
    logger()->removeAllAppenders();

    Log4Qt::Logger::rootLogger()->info(QStringLiteral("Unit test logger was shutdown."));
    Log4Qt::Logger::rootLogger()->removeAllAppenders();
    Log4Qt::Logger::rootLogger()->loggerRepository()->shutdown();
}

void BinaryLoggerTest::init()
{
    for (auto &appenderData : mAppenderData)
        appenderData.reset();
}

void BinaryLoggerTest::testBinaryToTextLayout()
{
    log("Hello world!");
    auto list = mAppender->clearList();
    QCOMPARE(list.size(), 1);
    auto result = list.at(0);
    QVERIFY(result.contains(QStringLiteral("12 bytes: 48 65 6c 6c 6f 20 77 6f 72 6c 64 21")));
}

void BinaryLoggerTest::testBinaryEventFilter()
{
    auto blogger = Log4Qt::Logger::rootLogger();
    Log4Qt::Filter *denyall = new Log4Qt::DenyAllFilter;
    denyall->activateOptions();

    auto *binfilter = new Log4Qt::BinaryEventFilter(blogger);

    binfilter->setAcceptBinaryEvents(true);
    binfilter->setNext(Log4Qt::FilterSharedPtr(denyall));
    binfilter->activateOptions();

    mAppender->addFilter(Log4Qt::FilterSharedPtr(binfilter));

    auto _ = createScopeExitGuard([this, binfilter] {mAppender->removeEventFilter(binfilter);});

    log("Hello world!");
    auto list = mAppender->clearList();
    QCOMPARE(list.size(), 1);
    QVERIFY(list.at(0).contains("12 bytes: 48 65 6c 6c 6f 20 77 6f 72 6c 64 21"));

    binfilter->setAcceptBinaryEvents(false);

    log("Hello world!");
    QVERIFY(mAppender->clearList().isEmpty());

}

void BinaryLoggerTest::testBinaryWriterAppender()
{
    auto blogger = Log4Qt::Logger::logger(binLogger);

    blogger->debug(QStringLiteral("Hello world!"));
    char expected[] = {0x18, 0x00, 0x00, 0x00, 0x48, 0x00, 0x65, 0x00,
                       0x6C, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x20, 0x00,
                       0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6C, 0x00,
                       0x64, 0x00, 0x21, 0x00
                      };
    QCOMPARE(flushLogger(blogger), QByteArray(expected, elementsInArray(expected)));

    Log4Qt::BinaryLoggingEvent event(blogger, Log4Qt::Level::INFO_INT, QByteArray("\0\1\2\3", 4));
    blogger->callAppenders(event);
    char expected1[] = {0x18, 0x00, 0x00, 0x00, 0x48, 0x00, 0x65, 0x00,
                        0x6C, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x20, 0x00,
                        0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6C, 0x00,
                        0x64, 0x00, 0x21, 0x00, 0x04, 0x00, 0x00, 0x00,
                        0x00, 0x01, 0x02, 0x03
                       };
    QCOMPARE(flushLogger(blogger), QByteArray(expected1, elementsInArray(expected1)));
}

void BinaryLoggerTest::testBinaryFileAppender()
{
    auto blogger = Log4Qt::Logger::logger(binLogger);

    QTemporaryFile file;
    QVERIFY(file.open());
    file.close();
    Log4Qt::BinaryFileAppender *bfa = new Log4Qt::BinaryFileAppender(file.fileName(), blogger);
    bfa->activateOptions();
    blogger->addAppender(bfa);

    auto _ = createScopeExitGuard([blogger, bfa] {blogger->removeAppender(bfa);});

    blogger->debug(QStringLiteral("Hello world!"));
    Log4Qt::BinaryLoggingEvent event(blogger, Log4Qt::Level::INFO_INT, QByteArray("\0\1\2\3", 4));
    blogger->callAppenders(event);

    char expected[] = {0x18, 0x00, 0x00, 0x00, 0x48, 0x00, 0x65, 0x00,
                       0x6c, 0x00, 0x6c, 0x00, 0x6f, 0x00, 0x20, 0x00,
                       0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x6c, 0x00,
                       0x64, 0x00, 0x21, 0x00, 0x04, 0x00, 0x00, 0x00,
                       0x00, 0x01, 0x02, 0x03
                      };

    bfa->close();
    QVERIFY(file.open());
    QByteArray data = file.readAll();
    QCOMPARE(data, QByteArray(expected, elementsInArray(expected)));
}

void BinaryLoggerTest::testBinaryLogStream()
{
    char expected[] = {0x1d, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x40,
                       0x0d, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c,
                       0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21,
                       0x00
                      };
    {
        Log4Qt::BinaryLogStream bls{Log4Qt::Logger::logger(binLogger), Log4Qt::Level::INFO_INT};
        bls << QByteArray(expected + 4, 4);
        bls << QByteArray(expected + 8, 8)
            << QByteArray(expected + 16, 4)
            << QByteArray(expected + 20, 13);
    }

    auto blogger = Log4Qt::Logger::logger(binLogger);
    QCOMPARE(flushLogger(blogger), QByteArray(expected, elementsInArray(expected)));
}

void BinaryLoggerTest::testBinaryLogger()
{
    auto ulogger = unitTestLogger();

    ulogger->debug(QByteArray("Hello world!"));

    char expected[] = {0x0c, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c,
                       0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21
                      };

    QCOMPARE(flushLogger(ulogger), QByteArray(expected, elementsInArray(expected)));
}

void BinaryLoggerTest::testBinaryClassLogger()
{
    auto clogger = logger();

    clogger->debug(QByteArray("Hello world!"));

    char expected[] = {0x0c, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c,
                       0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21
                      };

    QCOMPARE(flushLogger(clogger), QByteArray(expected, elementsInArray(expected)));
}

void BinaryLoggerTest::testBinaryLayout()
{
    auto clogger = logger();
    auto appender = new Log4Qt::BinaryWriterAppender(clogger);
    auto layout = new Log4Qt::BinaryLayout(appender);
    layout->setBinaryHeader("This is the header:");
    layout->setBinaryFooter(":This is the footer");
    appender->setLayout(Log4Qt::LayoutSharedPtr(layout));
    clogger->addAppender(appender);
    auto _ = createScopeExitGuard([clogger, appender] {clogger->removeAppender(appender);});
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    appender->setWriter(&ds);
    appender->activateOptions();

    clogger->debug("Hello world!");
    appender->close();

    char expected[] =
    {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x68, 0x65, 0x61, 0x64, 0x65, 0x72, 0x3a,
        0x0c, 0x00, 0x00, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21,
        0x3a, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x66, 0x6f, 0x6f, 0x74, 0x65, 0x72
    };
    QCOMPARE(QByteArray(expected, elementsInArray(expected)), data);
}

QTEST_GUILESS_MAIN(BinaryLoggerTest)

#include "binaryloggertest.moc"
