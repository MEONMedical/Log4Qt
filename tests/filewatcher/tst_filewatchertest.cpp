#include <QString>
#include <QtTest>
#include <QFile>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTextStream>

#include "log4qt/helpers/configuratorhelper.h"

class FilewatcherTest : public QObject
{
    Q_OBJECT

public:
    FilewatcherTest();

    void testFileWatcheSaveFileToTempDeleteOrigAndRename();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testConfiguratorHelperSaveFileToTempDeleteOrigAndRename();

private:
    void createTestFile(const QString &filename) const;
    void modifyTestFile(const QString &filename) const;
    void saveFileToTempDeleteOrigAndRenameToOrig(const QString &filename) const;
};

FilewatcherTest::FilewatcherTest()
{
}

void FilewatcherTest::createTestFile(const QString &filename) const
{
    QFile testFile(filename);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    testFile.write(QByteArray("Hello create\n"));
    testFile.close();
}

void FilewatcherTest::modifyTestFile(const QString &filename) const
{
    // resolution of the modification time is system dependent
    // So we have to wait before modifying
    QTest::qWait(2000);
    QFile testFile(filename);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    testFile.write(QByteArray("Hello modify\n"));
    testFile.close();
}

void FilewatcherTest::saveFileToTempDeleteOrigAndRenameToOrig(const QString &filename) const
{
    QSaveFile testFile(filename);
    QVERIFY(testFile.open(QIODevice::WriteOnly));
    testFile.write(QByteArray("Hello modify QSaveFile\n"));
    testFile.commit();
}

void FilewatcherTest::initTestCase()
{
}

void FilewatcherTest::cleanupTestCase()
{
}

void FilewatcherTest::testFileWatcheSaveFileToTempDeleteOrigAndRename()
{
    QTemporaryDir tempDir;
    QString testFilePath = tempDir.path() + "test.txt";
    createTestFile(testFilePath);

    QScopedPointer<QFileSystemWatcher> fileWatcher(new QFileSystemWatcher());
    fileWatcher->addPath(testFilePath);
    QSignalSpy fileChangeSpy(fileWatcher.data(), &QFileSystemWatcher::fileChanged);

    modifyTestFile(testFilePath);
    QVERIFY(QFile::exists(testFilePath));
    QTRY_VERIFY(fileChangeSpy.count() == 1); // modify
    fileChangeSpy.clear();

    saveFileToTempDeleteOrigAndRenameToOrig(testFilePath);
    QVERIFY(QFile::exists(testFilePath));
    QEXPECT_FAIL("", "Delete and rename fails: QFileSystemWatcher behavior", Continue);
    QTRY_VERIFY(fileChangeSpy.count() == 1);   // delete
    fileChangeSpy.clear();

    fileWatcher->addPath(testFilePath);  //read file path
    modifyTestFile(testFilePath);
    QVERIFY(QFile::exists(testFilePath));
    QTRY_VERIFY(fileChangeSpy.count() == 1);  // expected from modify
    QFile::remove(testFilePath);
}

bool configure(const QString &configFileName)
{
    Q_UNUSED(configFileName)
    return true;
}

void FilewatcherTest::testConfiguratorHelperSaveFileToTempDeleteOrigAndRename()
{
    QTemporaryDir tempDir;
    QString testFilePath = tempDir.path() + "log4qt.properties";
    createTestFile(testFilePath);
    QSignalSpy configurationFileChangeSpy(Log4Qt::ConfiguratorHelper::instance(), &Log4Qt::ConfiguratorHelper::configurationFileChanged);

    Log4Qt::ConfiguratorHelper::instance()->setConfigurationFile(testFilePath, configure);

    modifyTestFile(testFilePath);
    QVERIFY(QFile::exists(testFilePath));
    QTRY_VERIFY(configurationFileChangeSpy.count() == 1); // modify
    configurationFileChangeSpy.clear();

    saveFileToTempDeleteOrigAndRenameToOrig(testFilePath);
    QVERIFY(QFile::exists(testFilePath));
    QTRY_VERIFY(configurationFileChangeSpy.count() == 1);   // delete
    configurationFileChangeSpy.clear();

    modifyTestFile(testFilePath);
    QVERIFY(QFile::exists(testFilePath));
    QTRY_VERIFY(configurationFileChangeSpy.count() == 1);  // expected from modify
    QFile::remove(testFilePath);
}

QTEST_GUILESS_MAIN(FilewatcherTest)

#include "tst_filewatchertest.moc"
