/******************************************************************************
 *
 * package:     Log4Qt
 * file:        performancetest.cpp
 * created:     February 2026
 * author:      Performance Test
 *
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

#include "performancetest.h"
#include "log4qt/helpers/datetime.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/fileappender.h"
#include "log4qt/randomaccessfileappender.h"
#include "log4qt/patternlayout.h"
#include "log4qt/simplelayout.h"
#include "log4qt/varia/nullappender.h"
#include "log4qt/varia/levelmatchfilter.h"
#include "log4qt/helpers/datetime.h"
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QElapsedTimer>
#include <QDebug>

using namespace Qt::StringLiterals;

void PerformanceTest::initTestCase()
{
    // Create temporary directory for test files
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    mTestDir = tempDir.path();
    qDebug() << "Test directory:" << mTestDir;
}

void PerformanceTest::cleanupTestCase()
{
    // Clean up test directory
    QDir dir(mTestDir);
    if (dir.exists())
    {
        dir.removeRecursively();
    }
}

void PerformanceTest::init()
{
    // Reset logging before each test
    Log4Qt::LogManager::resetConfiguration();
}

void PerformanceTest::cleanup()
{
    // Reset logging after each test
    Log4Qt::LogManager::resetConfiguration();
}

void PerformanceTest::testSimpleLoggingPerformance_data()
{
    QTest::addColumn<int>("iterations");
    QTest::addColumn<QString>("level");
    
    QTest::newRow("1000 DEBUG messages") << 1000 << "DEBUG";
    QTest::newRow("10000 DEBUG messages") << 10000 << "DEBUG";
    QTest::newRow("100000 DEBUG messages") << 100000 << "DEBUG";
    QTest::newRow("1000 INFO messages") << 1000 << "INFO";
    QTest::newRow("10000 INFO messages") << 10000 << "INFO";
}

void PerformanceTest::testSimpleLoggingPerformance()
{
    QFETCH(int, iterations);
    QFETCH(QString, level);
    
    // Setup logger with null appender (no I/O overhead)
    auto logger = Log4Qt::Logger::rootLogger();
    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    nullAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));

    if (level == "DEBUG")
        logger->setLevel(Log4Qt::Level::DEBUG_INT);
    else if (level == "INFO")
        logger->setLevel(Log4Qt::Level::INFO_INT);
    
    // Benchmark logging
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            if (level == "DEBUG")
                logger->debug("Test message %1", i);
            else
                logger->info("Test message %1", i);
        }
    }
    
    logger->removeAllAppenders();
}

void PerformanceTest::testFileAppenderPerformance_data()
{
    QTest::addColumn<int>("iterations");
    QTest::addColumn<bool>("immediateFlush");
    QTest::addColumn<QString>("layoutType");
    
    QTest::newRow("1000 msgs, immediate flush, simple layout") << 1000 << true << "simple";
    QTest::newRow("1000 msgs, buffered, simple layout") << 1000 << false << "simple";
    QTest::newRow("10000 msgs, immediate flush, simple layout") << 10000 << true << "simple";
    QTest::newRow("10000 msgs, buffered, simple layout") << 10000 << false << "simple";
    QTest::newRow("1000 msgs, immediate flush, pattern layout") << 1000 << true << "pattern";
    QTest::newRow("1000 msgs, buffered, pattern layout") << 1000 << false << "pattern";
}

void PerformanceTest::testFileAppenderPerformance()
{
    QFETCH(int, iterations);
    QFETCH(bool, immediateFlush);
    QFETCH(QString, layoutType);
    
    QString testFile = mTestDir + "/performance_test.log";
    
    // Setup logger with file appender
    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::DEBUG_INT);
    
    auto fileAppender = new Log4Qt::FileAppender();
    fileAppender->setName("FileAppender");
    fileAppender->setFile(testFile);
    fileAppender->setImmediateFlush(immediateFlush);
    
    if (layoutType == "simple")
    {
        auto layout = new Log4Qt::SimpleLayout();
        fileAppender->setLayout(Log4Qt::LayoutSharedPtr(layout));
    }
    else if (layoutType == "pattern")
    {
        auto layout = new Log4Qt::PatternLayout();
        layout->setConversionPattern("%d{ISO8601} [%t] %-5p %c - %m%n");
        fileAppender->setLayout(Log4Qt::LayoutSharedPtr(layout));
    }

    fileAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(fileAppender));
    
    // Benchmark logging
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            logger->info("Performance test message number %1", i);
        }
    }
    
    logger->removeAllAppenders();
    
    // Clean up test file
    QFile::remove(testFile);
}

// Worker class for multi-threaded logging test
class LoggingWorker : public QRunnable
{
public:
    LoggingWorker(Log4Qt::Logger *logger, int iterations, int threadId)
        : mLogger(logger), mIterations(iterations), mThreadId(threadId)
    {
    }
    
    void run() override
    {
        for (int i = 0; i < mIterations; ++i)
        {
            mLogger->info("Thread %1 - Message %2", mThreadId, i);
        }
    }
    
private:
    Log4Qt::Logger *mLogger;
    int mIterations;
    int mThreadId;
};

void PerformanceTest::testMultiThreadedLoggingPerformance_data()
{
    QTest::addColumn<int>("threadCount");
    QTest::addColumn<int>("messagesPerThread");
    
    QTest::newRow("2 threads, 1000 msgs each") << 2 << 1000;
    QTest::newRow("4 threads, 1000 msgs each") << 4 << 1000;
    QTest::newRow("8 threads, 1000 msgs each") << 8 << 1000;
    QTest::newRow("4 threads, 5000 msgs each") << 4 << 5000;
}

void PerformanceTest::testMultiThreadedLoggingPerformance()
{
    QFETCH(int, threadCount);
    QFETCH(int, messagesPerThread);
    
    QString testFile = mTestDir + "/multithread_test.log";
    
    // Setup logger with file appender
    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::INFO_INT);
    
    auto fileAppender = new Log4Qt::FileAppender();
    fileAppender->setName("FileAppender");
    fileAppender->setFile(testFile);
    fileAppender->setImmediateFlush(false); // Use buffering for better performance
    
    auto layout = new Log4Qt::SimpleLayout();
    fileAppender->setLayout(Log4Qt::LayoutSharedPtr(layout));
    fileAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(fileAppender));
    
    // Benchmark multi-threaded logging
    QBENCHMARK
    {
        QThreadPool pool;
        pool.setMaxThreadCount(threadCount);
        
        for (int i = 0; i < threadCount; ++i)
        {
            auto worker = new LoggingWorker(logger, messagesPerThread, i);
            worker->setAutoDelete(true);
            pool.start(worker);
        }
        
        pool.waitForDone();
    }
    
    logger->removeAllAppenders();
    
    // Clean up test file
    QFile::remove(testFile);
}

void PerformanceTest::testFormattingPerformance_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("iterations");
    
    QTest::newRow("Simple pattern, 10000 msgs") << "%m%n" << 10000;
    QTest::newRow("Basic pattern, 10000 msgs") << "%d %p %c - %m%n" << 10000;
    QTest::newRow("Complex pattern, 10000 msgs") << "%d{ISO8601} [%t] %-5p %c{2} - %m%n" << 10000;
    QTest::newRow("Very complex pattern, 10000 msgs") << "%d{yyyy-MM-dd HH:mm:ss.zzz} [%t] %-5p %c - %F:%L - %m%n" << 10000;
}

void PerformanceTest::testFormattingPerformance()
{
    QFETCH(QString, pattern);
    QFETCH(int, iterations);
    
    // Setup logger with null appender but with pattern layout
    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::DEBUG_INT);
    
    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    
    auto layout = new Log4Qt::PatternLayout();
    layout->setConversionPattern(pattern);
    nullAppender->setLayout(Log4Qt::LayoutSharedPtr(layout));
    nullAppender->activateOptions();

    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));
    
    // Benchmark formatting
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            logger->debug("Test message with parameter: %1", i);
        }
    }
    
    logger->removeAllAppenders();
}

void PerformanceTest::testFilteringPerformance_data()
{
    QTest::addColumn<int>("filterCount");
    QTest::addColumn<int>("iterations");
    
    QTest::newRow("No filters, 10000 msgs") << 0 << 10000;
    QTest::newRow("1 filter, 10000 msgs") << 1 << 10000;
    QTest::newRow("3 filters, 10000 msgs") << 3 << 10000;
    QTest::newRow("5 filters, 10000 msgs") << 5 << 10000;
}

void PerformanceTest::testFilteringPerformance()
{
    QFETCH(int, filterCount);
    QFETCH(int, iterations);
    
    // Setup logger with null appender and filters
    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::DEBUG_INT);
    
    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    
    // Add filters
    for (int i = 0; i < filterCount; ++i)
    {
        auto filter = new Log4Qt::LevelMatchFilter();
        filter->setLevelToMatch(Log4Qt::Level::INFO_INT);
        filter->setAcceptOnMatch(true);
        nullAppender->addFilter(Log4Qt::FilterSharedPtr(filter));
    }

    nullAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));
    
    // Benchmark with filtering
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            logger->info("Test message %1", i);
        }
    }
    
    logger->removeAllAppenders();
}

void PerformanceTest::testTimestampCacheWindowPerformance_data()
{
    QTest::addColumn<int>("cacheWindowMs");
    QTest::addColumn<int>("messageCount");
    
    // Test different cache windows with various message counts
    QTest::newRow("0ms (no cache), 1000 msgs") << 0 << 1000;
    QTest::newRow("1ms (default), 1000 msgs") << 1 << 1000;
    QTest::newRow("10ms, 1000 msgs") << 10 << 1000;
    QTest::newRow("100ms, 1000 msgs") << 100 << 1000;
    
    QTest::newRow("0ms (no cache), 10000 msgs") << 0 << 10000;
    QTest::newRow("1ms (default), 10000 msgs") << 1 << 10000;
    QTest::newRow("10ms, 10000 msgs") << 10 << 10000;
    QTest::newRow("100ms, 10000 msgs") << 100 << 10000;
    
    QTest::newRow("0ms (no cache), 100000 msgs") << 0 << 100000;
    QTest::newRow("1ms (default), 100000 msgs") << 1 << 100000;
    QTest::newRow("10ms, 100000 msgs") << 10 << 100000;
    QTest::newRow("100ms, 100000 msgs") << 100 << 100000;
}

void PerformanceTest::testTimestampCacheWindowPerformance()
{
    QFETCH(int, cacheWindowMs);
    QFETCH(int, messageCount);
    
    // Configure timestamp cache window
    Log4Qt::DateTime::setCacheWindow(cacheWindowMs);
    
    // Create a null appender (discards all output - pure LoggingEvent creation benchmark)
    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::DEBUG_INT);
    logger->removeAllAppenders();
    
    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    nullAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));

    // Benchmark pure logging event creation with different cache windows
    QBENCHMARK
    {
        for (int i = 0; i < messageCount; ++i)
        {
            logger->debug("Timestamp benchmark message %1", i);
        }
    }
    
    logger->removeAllAppenders();
    
    // Reset to default
    Log4Qt::DateTime::setCacheWindow(1);
}

void PerformanceTest::testLoggerTemplateDisabled_data()
{
    QTest::addColumn<QString>("level");
    QTest::addColumn<int>("iterations");

    // Test with logging disabled to verify template optimization (isEnabledFor check before formatting)
    QTest::newRow("DEBUG disabled, 1000000 iter") << "DEBUG" << 1000000;
    QTest::newRow("TRACE disabled, 1000000 iter") << "TRACE" << 1000000;

    // Test with logging enabled for comparison (INFO is enabled)
    QTest::newRow("INFO enabled, 100000 iter") << "INFO" << 100000;
}

void PerformanceTest::testLoggerTemplateDisabled()
{
    QFETCH(QString, level);
    QFETCH(int, iterations);

    // Setup logger
    auto logger = Log4Qt::Logger::rootLogger();
    logger->removeAllAppenders();
    
    // Set logger level to INFO (so DEBUG and TRACE are disabled)
    logger->setLevel(Log4Qt::Level::INFO_INT);

    // Add a NullAppender just in case (though it shouldn't be reached if optimization works)
    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    nullAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));

    // Benchmark logging calls that should be short-circuited (check isEnabledFor)
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            if (level == "DEBUG")
                logger->debug("Complex message requiring formatting: %1 %2 %3", i, i*2, "string arg");
            else if (level == "TRACE")
                logger->trace("Complex message requiring formatting: %1 %2 %3", i, i*2, "string arg");
            else if (level == "INFO")
                logger->info("Complex message requiring formatting: %1 %2 %3", i, i*2, "string arg");
        }
    }

    logger->removeAllAppenders();
}

void PerformanceTest::testPatternFormatterOptimization_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("iterations");

    // Test fast path (simple patterns) vs standard path vs complex path
    QTest::newRow("Fast path (%m), 100000 iter") << "%m" << 100000;
    QTest::newRow("Standard path (%d %p - %m), 100000 iter") << "%d %p - %m" << 100000;
    QTest::newRow("Complex path (%d{ISO8601} [%t] %-5p %c - %m%n), 100000 iter") << "%d{ISO8601} [%t] %-5p %c - %m%n" << 100000;
}

void PerformanceTest::testPatternFormatterOptimization()
{
    QFETCH(QString, pattern);
    QFETCH(int, iterations);

    // Setup logger
    auto logger = Log4Qt::Logger::rootLogger();
    logger->removeAllAppenders();
    logger->setLevel(Log4Qt::Level::DEBUG_INT);

    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    
    auto layout = new Log4Qt::PatternLayout();
    layout->setConversionPattern(pattern);
    layout->activateOptions();

    nullAppender->setLayout(Log4Qt::LayoutSharedPtr(layout));
    nullAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));

    // Benchmark the formatting speed with optimized PatternFormatter
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            logger->debug("Optimized formatter test message %1", i);
        }
    }

    logger->removeAllAppenders();
}

void PerformanceTest::testLogStreamLazyInit_data()
{
    QTest::addColumn<QString>("level");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("iterations");

    QTest::newRow("DEBUG enabled, 100000 msgs") << "DEBUG" << true << 100000;
    QTest::newRow("DEBUG disabled, 100000 msgs") << "DEBUG" << false << 100000;
    QTest::newRow("INFO enabled, 100000 msgs") << "INFO" << true << 100000;
    QTest::newRow("INFO disabled, 100000 msgs") << "INFO" << false << 100000;
}

void PerformanceTest::testLogStreamLazyInit()
{
    QFETCH(QString, level);
    QFETCH(bool, enabled);
    QFETCH(int, iterations);

    // Setup logger
    auto logger = Log4Qt::Logger::rootLogger();
    logger->removeAllAppenders();

    // Use NullAppender if enabled, otherwise no appender needed (but we add one to be safe)
    auto nullAppender = new Log4Qt::NullAppender();
    nullAppender->setName("NullAppender");
    nullAppender->activateOptions();
    logger->addAppender(Log4Qt::AppenderSharedPtr(nullAppender));

    // Set logger level to control enabled/disabled state
    if (enabled) {
        // If we want it enabled, set logger level to match or be lower (more verbose)
        if (level == "DEBUG") logger->setLevel(Log4Qt::Level::DEBUG_INT);
        else if (level == "INFO") logger->setLevel(Log4Qt::Level::INFO_INT);
    } else {
        // If we want it disabled, set logger level to be higher (less verbose)
        if (level == "DEBUG") logger->setLevel(Log4Qt::Level::INFO_INT);
        else if (level == "INFO") logger->setLevel(Log4Qt::Level::WARN_INT);
    }

    // Benchmark the stream operator usage
    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            if (level == "DEBUG")
                logger->debug() << "LogStream benchmark message " << i;
            else
                logger->info() << "LogStream benchmark message " << i;
        }
    }

    logger->removeAllAppenders();
}

void PerformanceTest::testAppenderComparison_data()
{
    QTest::addColumn<int>("iterations");
    QTest::addColumn<QString>("appenderType");

    QTest::newRow("FileAppender 1000 msgs")               << 1000   << "file";
    QTest::newRow("RandomAccessFileAppender 1000 msgs")   << 1000   << "raf";
    QTest::newRow("FileAppender 10000 msgs")              << 10000  << "file";
    QTest::newRow("RandomAccessFileAppender 10000 msgs")  << 10000  << "raf";
    QTest::newRow("FileAppender 100000 msgs")             << 100000 << "file";
    QTest::newRow("RandomAccessFileAppender 100000 msgs") << 100000 << "raf";
}

void PerformanceTest::testAppenderComparison()
{
    QFETCH(int, iterations);
    QFETCH(QString, appenderType);

    const QString testFile = mTestDir + QString("/comparison_%1_%2.log").arg(appenderType).arg(iterations);

    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::INFO_INT);

    auto layout = new Log4Qt::PatternLayout();
    layout->setConversionPattern("%d{ISO8601} [%t] %-5p %c - %m%n");
    layout->activateOptions();

    Log4Qt::Appender *appenderPtr = nullptr;

    Log4Qt::LayoutSharedPtr layoutPtr(layout);
    if (appenderType == "file")
    {
        auto appender = new Log4Qt::FileAppender();
        appender->setName("FileAppender");
        appender->setFile(testFile);
        appender->setImmediateFlush(false);
        appender->setLayout(layoutPtr);
        appender->activateOptions();
        appenderPtr = appender;
        logger->addAppender(Log4Qt::AppenderSharedPtr(appender));
    }
    else
    {
        auto appender = new Log4Qt::RandomAccessFileAppender();
        appender->setName("RAFAppender");
        appender->setFile(testFile);
        appender->setImmediateFlush(false);
        appender->setLayout(layoutPtr);
        appender->activateOptions();
        appenderPtr = appender;
        logger->addAppender(Log4Qt::AppenderSharedPtr(appender));
    }

    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
            logger->info("Comparison benchmark message number %1", i);
    }

    // Explicitly close before removeAllAppenders() to guarantee all buffered
    // data is flushed to disk (removeAllAppenders does not call close).
    if (appenderPtr)
        appenderPtr->close();
    logger->removeAllAppenders();

    // Correctness: verify the log file was written with the expected number of lines
    QFile result(testFile);
    QVERIFY2(result.exists(), "Log file must exist after benchmark");
    QVERIFY2(result.open(QIODevice::ReadOnly), "Log file must be readable");
    const QByteArray content = result.readAll();
    result.close();

    const int lineCount = content.count('\n');
    QVERIFY2(lineCount >= iterations,
             qPrintable(QString("Expected at least %1 lines, got %2").arg(iterations).arg(lineCount)));

    QFile::remove(testFile);
}

void PerformanceTest::testAppenderComparisonMultiThreaded_data()
{
    QTest::addColumn<int>("threadCount");
    QTest::addColumn<int>("messagesPerThread");
    QTest::addColumn<QString>("appenderType");

    // Fewer messages per thread than the single-threaded test: the total work
    // (threadCount * messagesPerThread) is what matters, and we want the
    // benchmark to complete in a reasonable time in CI.
    QTest::newRow("FileAppender             2 threads x 5000")  << 2 << 5000  << "file";
    QTest::newRow("RandomAccessFileAppender 2 threads x 5000")  << 2 << 5000  << "raf";
    QTest::newRow("FileAppender             4 threads x 5000")  << 4 << 5000  << "file";
    QTest::newRow("RandomAccessFileAppender 4 threads x 5000")  << 4 << 5000  << "raf";
    QTest::newRow("FileAppender             8 threads x 5000")  << 8 << 5000  << "file";
    QTest::newRow("RandomAccessFileAppender 8 threads x 5000")  << 8 << 5000  << "raf";
}

void PerformanceTest::testAppenderComparisonMultiThreaded()
{
    QFETCH(int, threadCount);
    QFETCH(int, messagesPerThread);
    QFETCH(QString, appenderType);

    const QString testFile = mTestDir
        + QString("/mt_comparison_%1_%2x%3.log")
              .arg(appenderType).arg(threadCount).arg(messagesPerThread);

    auto logger = Log4Qt::Logger::rootLogger();
    logger->setLevel(Log4Qt::Level::INFO_INT);

    // Use the same expensive pattern as the single-threaded test so that the
    // split-lock benefit (concurrent formatting) is clearly visible.
    auto layout = new Log4Qt::PatternLayout();
    layout->setConversionPattern("%d{ISO8601} [%t] %-5p %c - %m%n");
    layout->activateOptions();

    Log4Qt::Appender *appenderPtr = nullptr;

    Log4Qt::LayoutSharedPtr layoutPtr(layout);
    if (appenderType == "file")
    {
        auto appender = new Log4Qt::FileAppender();
        appender->setName("FileAppender");
        appender->setFile(testFile);
        appender->setImmediateFlush(false);
        appender->setLayout(layoutPtr);
        appender->activateOptions();
        appenderPtr = appender;
        logger->addAppender(Log4Qt::AppenderSharedPtr(appender));
    }
    else
    {
        auto appender = new Log4Qt::RandomAccessFileAppender();
        appender->setName("RAFAppender");
        appender->setFile(testFile);
        appender->setImmediateFlush(false);
        appender->setLayout(layoutPtr);
        appender->activateOptions();
        appenderPtr = appender;
        logger->addAppender(Log4Qt::AppenderSharedPtr(appender));
    }

    QBENCHMARK
    {
        QThreadPool pool;
        pool.setMaxThreadCount(threadCount);

        for (int i = 0; i < threadCount; ++i)
        {
            auto worker = new LoggingWorker(logger, messagesPerThread, i);
            worker->setAutoDelete(true);
            pool.start(worker);
        }

        pool.waitForDone();
    }

    if (appenderPtr)
        appenderPtr->close();
    logger->removeAllAppenders();

    // Correctness: every message from every thread must appear in the file.
    // QBENCHMARK may run the block N times, so we check >= one full round.
    QFile result(testFile);
    QVERIFY2(result.exists(), "Log file must exist after benchmark");
    QVERIFY2(result.open(QIODevice::ReadOnly), "Log file must be readable");
    const QByteArray content = result.readAll();
    result.close();

    const int expectedMinLines = threadCount * messagesPerThread;
    const int lineCount = content.count('\n');
    QVERIFY2(lineCount >= expectedMinLines,
             qPrintable(QString("Expected at least %1 lines (%2 threads x %3 msgs), got %4")
                            .arg(expectedMinLines).arg(threadCount)
                            .arg(messagesPerThread).arg(lineCount)));

    QFile::remove(testFile);
}

QTEST_MAIN(PerformanceTest)

struct OldLoggingEvent
{
    Log4Qt::Level mLevel;
    const Log4Qt::Logger *mLogger;
    QString mMessage;
    QString mNdc;
    QHash<QString, QString> mProperties;
    qint64 mSequenceNumber;
    QString mThreadName;
    qint64 mTimeStamp;
    Log4Qt::MessageContext mContext;
    QString mCategoryName;

    OldLoggingEvent(const Log4Qt::Logger *logger, Log4Qt::Level level, const QString &message)
        : mLevel(level), mLogger(logger), mMessage(message), mSequenceNumber(123), mTimeStamp(456)
    {
        mThreadName = QStringLiteral("MainThread");
        mCategoryName = QStringLiteral("TestCategory");
        mProperties.insert(QStringLiteral("key1"), QStringLiteral("value1"));
        mProperties.insert(QStringLiteral("key2"), QStringLiteral("value2"));
    }
};

void PerformanceTest::testLoggingEventPerformance_data()
{
    QTest::addColumn<int>("iterations");
    QTest::addColumn<int>("payloadSize");
    
    QTest::newRow("1000000 copies, small message") << 1000000 << 32;
}

void PerformanceTest::testLoggingEventPerformance()
{
    QFETCH(int, iterations);
    QFETCH(int, payloadSize);
    
    QString message(payloadSize, 'x');
    Log4Qt::LoggingEvent event(Log4Qt::Logger::rootLogger(), Log4Qt::Level::INFO_INT, message);
    
    QBENCHMARK {
        for (int i = 0; i < iterations; ++i) {
            Log4Qt::LoggingEvent copy = event;
            Q_UNUSED(copy);
        }
    }
}

void PerformanceTest::testLoggingEventComparison_data()
{
    QTest::addColumn<bool>("useNew");
    
    QTest::newRow("Old Implementation (Direct members)") << false;
    QTest::newRow("New Implementation (Implicit sharing)") << true;
}

void PerformanceTest::testLoggingEventComparison()
{
    QFETCH(bool, useNew);
    const int iterations = 1000000;
    QString message(1024, 'x');

    if (useNew) {
        Log4Qt::LoggingEvent event(Log4Qt::Logger::rootLogger(), Log4Qt::Level::INFO_INT, message);
        QBENCHMARK {
            for (int i = 0; i < iterations; ++i) {
                Log4Qt::LoggingEvent copy = event;
                Q_UNUSED(copy);
            }
        }
    } else {
        OldLoggingEvent event(Log4Qt::Logger::rootLogger(), Log4Qt::Level::INFO_INT, message);
        QBENCHMARK {
            for (int i = 0; i < iterations; ++i) {
                OldLoggingEvent copy = event;
                Q_UNUSED(copy);
            }
        }
    }
}

void PerformanceTest::testISO8601FormattingPerformance_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<int>("iterations");

    QTest::newRow("ISO8601, same ms (cache hit), 100000")    << "ISO8601"        << 100000;
    QTest::newRow("ISO8601, unique ms (cache miss), 100000") << "ISO8601_unique" << 100000;
    QTest::newRow("ABSOLUTE, same ms (cache hit), 100000")   << "ABSOLUTE"       << 100000;
    QTest::newRow("custom format, 10000")                    << "hh:mm:ss"       << 10000;
}

void PerformanceTest::testISO8601FormattingPerformance()
{
    QFETCH(QString, format);
    QFETCH(int, iterations);

    const bool uniqueMs = format.endsWith(u"_unique"_s);
    const QString actualFormat = uniqueMs ? format.chopped(7) : format;

    // Use DateTime::formatMsecs() directly — this is the hot path called by
    // DatePatternConverter::convert() and avoids QDateTime construction overhead
    // so the benchmark isolates the formatting cost alone.
    const qint64 baseMs = QDateTime::currentMSecsSinceEpoch();

    QBENCHMARK
    {
        for (int i = 0; i < iterations; ++i)
        {
            const qint64 ms = uniqueMs ? (baseMs + i) : baseMs;
            const QString result = Log4Qt::DateTime::formatMsecs(ms, actualFormat);
            Q_UNUSED(result)
        }
    }
}

#include "moc_performancetest.cpp"
