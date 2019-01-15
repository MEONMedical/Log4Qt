#include "log4qt/dailyfileappender.h"

#include "log4qt/loggingevent.h"
#include "log4qt/simplelayout.h"

#include <QDate>
#include <QSharedPointer>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using Log4Qt::DailyFileAppender;

class DailyFileAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void testFileCreation_data();
    void testFileCreation();
    void testAppend();

private:
    QTemporaryDir *mLogDirectory;
    DailyFileAppender *mAppender;
};

void DailyFileAppenderTest::init()
{
    mLogDirectory = new QTemporaryDir;

    mAppender = new DailyFileAppender;
    mAppender->setLayout(Log4Qt::LayoutSharedPtr(new Log4Qt::SimpleLayout));
}

void DailyFileAppenderTest::cleanup()
{
    delete mAppender;
    delete mLogDirectory;  // destructor will remove temporary directory
}

namespace
{

class DateRetrieverMock final : public Log4Qt::IDateRetriever
{
public:
    QDate currentDate() const override
    {
        return mCurrentDate;
    }

    void setCurrentDate(const QDate currentDate)
    {
        mCurrentDate = currentDate;
    }

private:
    QDate mCurrentDate;
};

}

void DailyFileAppenderTest::testFileCreation_data()
{
    QTest::addColumn<QString>("appName");
    QTest::addColumn<QString>("datePattern");
    QTest::addColumn<QString>("fileName");

    QTest::newRow("default") << "app" << "_yyyy_MM_dd" << "app_2019_07_09.log";
    QTest::newRow("Austria") << "app" << "_dd.MM.yyyy" << "app_09.07.2019.log";
    QTest::newRow("service") << "srv" << "_yyyy_MM_dd" << "srv_2019_07_09.log";
}

void DailyFileAppenderTest::testFileCreation()
{
    const QSharedPointer<DateRetrieverMock> dateRetriever(new DateRetrieverMock);

    dateRetriever->setCurrentDate(QDate(2019, 7, 9));

    mAppender->setDateRetriever(dateRetriever);

    QFETCH(QString, appName);
    QFETCH(QString, datePattern);
    QFETCH(QString, fileName);

    mAppender->setDatePattern(datePattern);
    mAppender->setFile(mLogDirectory->filePath(appName + ".log"));

    mAppender->activateOptions();

    const QFileInfo fileInfo(mAppender->file());

    QVERIFY(fileInfo.exists());

    QCOMPARE(fileInfo.fileName(), fileName);
}

void DailyFileAppenderTest::testAppend()
{
    mAppender->setFile(mLogDirectory->filePath(QStringLiteral("app.log")));

    mAppender->activateOptions();

    const auto fileName = mAppender->file();

    QVERIFY(QFileInfo::exists(fileName));

    const QFile logFile(fileName);

    // nothing written yet
    QCOMPARE(logFile.size(), 0);

    mAppender->append(Log4Qt::LoggingEvent());

    QVERIFY(logFile.size() > 0);
}

QTEST_GUILESS_MAIN(DailyFileAppenderTest)

#include "dailyfileappendertest.moc"
