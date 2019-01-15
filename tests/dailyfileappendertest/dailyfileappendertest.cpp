#include "log4qt/dailyfileappender.h"

#include "log4qt/simplelayout.h"

#include <QTemporaryDir>
#include <QtTest/QtTest>

using Log4Qt::DailyFileAppender;

class DailyFileAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void testFileCreation();

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

void DailyFileAppenderTest::testFileCreation()
{
    mAppender->setDatePattern(QStringLiteral("_yyyy_MM_dd"));
    mAppender->setFile(mLogDirectory->filePath(QStringLiteral("app.log")));

    mAppender->activateOptions();

    QVERIFY(QFileInfo::exists(mAppender->file()));
}

QTEST_GUILESS_MAIN(DailyFileAppenderTest)

#include "dailyfileappendertest.moc"
