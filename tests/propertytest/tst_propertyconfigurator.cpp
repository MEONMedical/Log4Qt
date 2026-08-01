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

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "log4qt/abstractlayout.h"
#include "log4qt/consoleappender.h"
#include "log4qt/dailyrollingfileappender.h"
#include "log4qt/helpers/configuratorhelper.h"
#include "log4qt/helpers/optionconverter.h"
#include "log4qt/helpers/properties.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/patternlayout.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/simplelayout.h"
#include "log4qt/spi/headerfooterprovider.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/varia/levelmatchfilter.h"
#include "log4qt/varia/stringmatchfilter.h"
#include "log4qt/loggerrepository.h"

using namespace Qt::StringLiterals;

using namespace Log4Qt;

class PropertyConfiguratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();
    void testRootLoggerLevel();
    void testSingleAppender();
    void testAppenderWithLayout();
    void testAppenderWithPatternLayout();
    void testAppenderWithFilter();
    void testMultipleAppenders();
    void testNamedLogger();
    void testLoggerAdditivity();
    void testMultipleLoggers();
    void testVariableSubstitution();
    void testGlobalReset();
    void testGlobalStatus();
    void testGlobalThreshold();
    void testGlobalHandleQtMessages();
    void testMissingAppenderType();
    void testUnknownAppenderClass();
    void testMissingLayout();
    void testUnknownLayoutClass();
    void testConfigureFromFile();
    void testConfigureMissingFile();
    void testRealWorldConfig();
    // Legacy Log4j1-style format tests
    void testLegacyRootLogger();
    void testLegacyAppenderWithLayout();
    void testLegacyNamedLogger();
    void testLegacyCategoryAlias();
    void testLegacyGlobalSettings();
    void testLegacyMultipleAppenders();
    void testLegacyRealWorldConfig();
    void testLegacyVariableSubstitution();
    void testLegacyCrossReferenceSubstitution();
    void testCircularSubstitution();
    void testSubstitutionWithLiteralClosingBrace();
    void testEnumPropertyFromConfig();
    // HeaderFooterProvider configuration tests
    void testGlobalHeaderFooterProvider();
    void testPerLayoutHeaderFooterProvider();
    void testLegacyGlobalHeaderFooterProvider();
    void testPerLayoutProviderOverridesGlobal();

private:
    void writePropertiesFile(const QString &path, const QByteArray &content);
};

void PropertyConfiguratorTest::cleanup()
{
    AbstractLayout::setGlobalHeaderFooterProvider({});
    LogManager::resetConfiguration();
}

void PropertyConfiguratorTest::writePropertiesFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(content);
    file.close();
}

void PropertyConfiguratorTest::testRootLoggerLevel()
{
    Properties props;
    props.setProperty(u"rootLogger.level"_s, u"ALL"_s);

    QVERIFY(PropertyConfigurator::configure(props));
    QCOMPARE(LogManager::rootLogger()->level(), Level::ALL_INT);
}

void PropertyConfiguratorTest::testSingleAppender()
{
    // NullAppender doesn't require a layout
    Properties props;
    props.setProperty(u"appender.null0.type"_s, u"Null"_s);
    props.setProperty(u"rootLogger.level"_s, u"DEBUG"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"null0"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::DEBUG_INT);
    QCOMPARE(root->appenders().count(), 1);
    QCOMPARE(root->appenders().first()->name(), u"null0"_s);
}

void PropertyConfiguratorTest::testAppenderWithLayout()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"TTCCLayout"_s);
    props.setProperty(u"appender.console.layout.dateFormat"_s, u"ISO8601"_s);
    props.setProperty(u"rootLogger.level"_s, u"DEBUG"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::DEBUG_INT);
    auto appenders = root->appenders();
    QCOMPARE(appenders.count(), 1);

    auto *consoleApp = qobject_cast<ConsoleAppender *>(appenders.first().data());
    QVERIFY(consoleApp);
    QCOMPARE(consoleApp->name(), u"console"_s);

    auto *ttcc = qobject_cast<TTCCLayout *>(consoleApp->layout().data());
    QVERIFY(ttcc);
    QCOMPARE(ttcc->dateFormat(), u"ISO8601"_s);
}

void PropertyConfiguratorTest::testAppenderWithPatternLayout()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.target"_s, u"STDOUT_TARGET"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"PatternLayout"_s);
    props.setProperty(u"appender.console.layout.conversionPattern"_s, u"%-5p %c - %m%n"_s);
    props.setProperty(u"rootLogger.level"_s, u"TRACE"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::TRACE_INT);

    auto *consoleApp = qobject_cast<ConsoleAppender *>(root->appenders().first().data());
    QVERIFY(consoleApp);
    QCOMPARE(consoleApp->target(), u"STDOUT_TARGET"_s);

    auto *pattern = qobject_cast<PatternLayout *>(consoleApp->layout().data());
    QVERIFY(pattern);
    QCOMPARE(pattern->conversionPattern(), u"%-5p %c - %m%n"_s);
}

void PropertyConfiguratorTest::testAppenderWithFilter()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"SimpleLayout"_s);
    props.setProperty(u"appender.console.filter.f1.type"_s, u"LevelMatch"_s);
    props.setProperty(u"appender.console.filter.f1.levelToMatch"_s, u"ERROR"_s);
    props.setProperty(u"appender.console.filter.f1.acceptOnMatch"_s, u"true"_s);
    props.setProperty(u"rootLogger.level"_s, u"ALL"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    auto *consoleApp = qobject_cast<ConsoleAppender *>(root->appenders().first().data());
    QVERIFY(consoleApp);

    auto filter = consoleApp->filter();
    QVERIFY(filter);
    auto *levelMatch = qobject_cast<LevelMatchFilter *>(filter.data());
    QVERIFY(levelMatch);
    QCOMPARE(levelMatch->levelToMatch(), Level(Level::ERROR_INT));
    QCOMPARE(levelMatch->acceptOnMatch(), true);
}

void PropertyConfiguratorTest::testMultipleAppenders()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"SimpleLayout"_s);
    props.setProperty(u"appender.null0.type"_s, u"Null"_s);
    props.setProperty(u"rootLogger.level"_s, u"INFO"_s);
    props.setProperty(u"rootLogger.appenderRef.console.ref"_s, u"console"_s);
    props.setProperty(u"rootLogger.appenderRef.null0.ref"_s, u"null0"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::INFO_INT);
    QCOMPARE(root->appenders().count(), 2);
}

void PropertyConfiguratorTest::testNamedLogger()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"SimpleLayout"_s);
    props.setProperty(u"rootLogger.level"_s, u"INFO"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);
    props.setProperty(u"logger.MyApp.name"_s, u"MyApp"_s);
    props.setProperty(u"logger.MyApp.level"_s, u"ERROR"_s);
    props.setProperty(u"logger.MyApp.appenderRef.0.ref"_s, u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::INFO_INT);

    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->appenders().count(), 1);
}

void PropertyConfiguratorTest::testLoggerAdditivity()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"SimpleLayout"_s);
    props.setProperty(u"rootLogger.level"_s, u"ALL"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);
    props.setProperty(u"logger.MyApp.name"_s, u"MyApp"_s);
    props.setProperty(u"logger.MyApp.level"_s, u"WARN"_s);
    props.setProperty(u"logger.MyApp.additivity"_s, u"false"_s);
    props.setProperty(u"logger.MyApp.appenderRef.0.ref"_s, u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::WARN_INT);
    QCOMPARE(myApp->additivity(), false);
}

void PropertyConfiguratorTest::testMultipleLoggers()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"SimpleLayout"_s);
    props.setProperty(u"rootLogger.level"_s, u"ALL"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);
    props.setProperty(u"logger.App.name"_s, u"App"_s);
    props.setProperty(u"logger.App.level"_s, u"ERROR"_s);
    props.setProperty(u"logger.Lib.name"_s, u"Lib"_s);
    props.setProperty(u"logger.Lib.level"_s, u"WARN"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    QCOMPARE(LogManager::rootLogger()->level(), Level::ALL_INT);
    QCOMPARE(LogManager::logger(u"App"_s)->level(), Level::ERROR_INT);
    QCOMPARE(LogManager::logger(u"Lib"_s)->level(), Level::WARN_INT);
}

void PropertyConfiguratorTest::testVariableSubstitution()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    Properties props;
    props.setProperty(u"logpath"_s, logDir);
    props.setProperty(u"appender.daily.type"_s, u"DailyFile"_s);
    props.setProperty(u"appender.daily.file"_s, u"${logpath}/myapp.log"_s);
    props.setProperty(u"appender.daily.appendFile"_s, u"true"_s);
    props.setProperty(u"appender.daily.datePattern"_s, u"_yyyy_MM_dd"_s);
    props.setProperty(u"appender.daily.layout.type"_s, u"TTCCLayout"_s);
    props.setProperty(u"rootLogger.level"_s, u"ALL"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"daily"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    auto *dailyApp = qobject_cast<DailyRollingFileAppender *>(root->appenders().first().data());
    QVERIFY(dailyApp);
    QVERIFY(dailyApp->file().contains(u"myapp"_s));
    QCOMPARE(dailyApp->appendFile(), true);
    QCOMPARE(dailyApp->datePattern(), u"_yyyy_MM_dd"_s);
}

void PropertyConfiguratorTest::testGlobalReset()
{
    // Add an appender first, then reset should remove it
    Logger *root = LogManager::rootLogger();
    root->addAppender(AppenderSharedPtr(new ConsoleAppender));
    QCOMPARE(root->appenders().count(), 1);

    Properties props;
    props.setProperty(u"reset"_s, u"true"_s);

    QVERIFY(PropertyConfigurator::configure(props));
    // After reset, root logger has no appenders (we didn't configure any)
    QCOMPARE(root->appenders().count(), 0);
}

void PropertyConfiguratorTest::testGlobalStatus()
{
    Logger *logLogger = LogManager::logLogger();
    logLogger->setLevel(Level::INFO_INT);

    Properties props;
    props.setProperty(u"status"_s, u"TRACE"_s);

    QVERIFY(PropertyConfigurator::configure(props));
    QCOMPARE(logLogger->level(), Level(Level::TRACE_INT));
}

void PropertyConfiguratorTest::testGlobalThreshold()
{
    Properties props;
    props.setProperty(u"threshold"_s, u"WARN"_s);

    QVERIFY(PropertyConfigurator::configure(props));
    QCOMPARE(LogManager::loggerRepository()->threshold(), Level(Level::WARN_INT));
}

void PropertyConfiguratorTest::testGlobalHandleQtMessages()
{
    LogManager::setHandleQtMessages(false);

    Properties props;
    props.setProperty(u"handleQtMessages"_s, u"true"_s);

    QVERIFY(PropertyConfigurator::configure(props));
    QCOMPARE(LogManager::handleQtMessages(), true);
}

void PropertyConfiguratorTest::testMissingAppenderType()
{
    // appender alias defined but no type key
    Properties props;
    props.setProperty(u"appender.bad.name"_s, u"bad"_s);

    QVERIFY(!PropertyConfigurator::configure(props));
    QCOMPARE(ConfiguratorHelper::configureError().count(), 1);
}

void PropertyConfiguratorTest::testUnknownAppenderClass()
{
    Properties props;
    props.setProperty(u"appender.bad.type"_s, u"NonExistentAppender"_s);

    QVERIFY(!PropertyConfigurator::configure(props));
    QCOMPARE(ConfiguratorHelper::configureError().count(), 1);
}

void PropertyConfiguratorTest::testMissingLayout()
{
    // ConsoleAppender requires a layout, but none is specified
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);

    QVERIFY(!PropertyConfigurator::configure(props));
    QCOMPARE(ConfiguratorHelper::configureError().count(), 1);
}

void PropertyConfiguratorTest::testUnknownLayoutClass()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"NonExistentLayout"_s);

    QVERIFY(!PropertyConfigurator::configure(props));
    QCOMPARE(ConfiguratorHelper::configureError().count(), 1);
}

void PropertyConfiguratorTest::testConfigureFromFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.properties";
    writePropertiesFile(file,
        "appender.console.type=Console\n"
        "appender.console.layout.type=SimpleLayout\n"
        "rootLogger.level=TRACE\n"
        "rootLogger.appenderRef.0.ref=console\n"
    );

    QVERIFY(PropertyConfigurator::configure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::TRACE_INT);
    QCOMPARE(LogManager::rootLogger()->appenders().count(), 1);

    auto *consoleApp = qobject_cast<ConsoleAppender *>(
        LogManager::rootLogger()->appenders().first().data());
    QVERIFY(consoleApp);
    QVERIFY(qobject_cast<SimpleLayout *>(consoleApp->layout().data()));
}

void PropertyConfiguratorTest::testConfigureMissingFile()
{
    QVERIFY(!PropertyConfigurator::configure(u"/nonexistent/path/to/file.properties"_s));
}

void PropertyConfiguratorTest::testRealWorldConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    const QString file = dir.path() + "/test.properties";
    const QByteArray content = QString(
        "logpath=%1\n"
        "reset=true\n"
        "threshold=NULL\n"
        "handleQtMessages=true\n"
        "\n"
        "appender.console.type=Console\n"
        "appender.console.target=STDOUT_TARGET\n"
        "appender.console.threshold=OFF\n"
        "appender.console.layout.type=TTCCLayout\n"
        "appender.console.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz\n"
        "appender.console.layout.contextPrinting=true\n"
        "\n"
        "appender.daily.type=DailyFile\n"
        "appender.daily.file=${logpath}/myapp.log\n"
        "appender.daily.appendFile=true\n"
        "appender.daily.datePattern=_yyyy_MM_dd\n"
        "appender.daily.layout.type=TTCCLayout\n"
        "appender.daily.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz\n"
        "appender.daily.layout.contextPrinting=true\n"
        "\n"
        "rootLogger.level=ALL\n"
        "rootLogger.appenderRef.console.ref=console\n"
        "rootLogger.appenderRef.daily.ref=daily\n"
        "\n"
        "logger.MyApp.name=MyApp\n"
        "logger.MyApp.level=ERROR\n"
        "logger.MyApp.additivity=false\n"
        "logger.MyApp.appenderRef.0.ref=console\n"
    ).arg(logDir).toUtf8();

    writePropertiesFile(file, content);
    QVERIFY(PropertyConfigurator::configure(file));

    // Root logger
    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::ALL_INT);

    // Verify both appenders exist
    auto appenders = root->appenders();
    ConsoleAppender *consoleApp = nullptr;
    DailyRollingFileAppender *dailyApp = nullptr;
    for (const auto &a : appenders)
    {
        if (a->name() == u"console"_s)
            consoleApp = qobject_cast<ConsoleAppender *>(a.data());
        else if (a->name() == u"daily"_s)
            dailyApp = qobject_cast<DailyRollingFileAppender *>(a.data());
    }

    // Console appender
    QVERIFY(consoleApp);
    QCOMPARE(consoleApp->target(), u"STDOUT_TARGET"_s);
    QCOMPARE(consoleApp->threshold(), Level::OFF_INT);
    auto *consoleTtcc = qobject_cast<TTCCLayout *>(consoleApp->layout().data());
    QVERIFY(consoleTtcc);
    QCOMPARE(consoleTtcc->dateFormat(), u"dd.MM.yyyy hh:mm:ss.zzz"_s);
    QCOMPARE(consoleTtcc->contextPrinting(), true);

    // Daily file appender
    QVERIFY(dailyApp);
    QVERIFY(dailyApp->file().contains(u"myapp"_s));
    QCOMPARE(dailyApp->appendFile(), true);
    QCOMPARE(dailyApp->datePattern(), u"_yyyy_MM_dd"_s);
    auto *dailyTtcc = qobject_cast<TTCCLayout *>(dailyApp->layout().data());
    QVERIFY(dailyTtcc);
    QCOMPARE(dailyTtcc->dateFormat(), u"dd.MM.yyyy hh:mm:ss.zzz"_s);
    QCOMPARE(dailyTtcc->contextPrinting(), true);

    // Named logger
    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->additivity(), false);
    QCOMPARE(myApp->appenders().count(), 1);
}

// --- Legacy Log4j1-style format tests ---

void PropertyConfiguratorTest::testLegacyRootLogger()
{
    Properties props;
    props.setProperty(u"log4j.appender.A1"_s, u"org.apache.log4j.ConsoleAppender"_s);
    props.setProperty(u"log4j.appender.A1.layout"_s, u"org.apache.log4j.SimpleLayout"_s);
    props.setProperty(u"log4j.rootLogger"_s, u"DEBUG, A1"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::DEBUG_INT);
    QCOMPARE(root->appenders().count(), 1);
    QCOMPARE(root->appenders().first()->name(), u"A1"_s);
}

void PropertyConfiguratorTest::testLegacyAppenderWithLayout()
{
    Properties props;
    props.setProperty(u"log4j.appender.A1"_s, u"org.apache.log4j.ConsoleAppender"_s);
    props.setProperty(u"log4j.appender.A1.target"_s, u"STDOUT_TARGET"_s);
    props.setProperty(u"log4j.appender.A1.layout"_s, u"org.apache.log4j.TTCCLayout"_s);
    props.setProperty(u"log4j.appender.A1.layout.dateFormat"_s, u"ISO8601"_s);
    props.setProperty(u"log4j.rootLogger"_s, u"ALL, A1"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    auto *consoleApp = qobject_cast<ConsoleAppender *>(root->appenders().first().data());
    QVERIFY(consoleApp);
    QCOMPARE(consoleApp->target(), u"STDOUT_TARGET"_s);

    auto *ttcc = qobject_cast<TTCCLayout *>(consoleApp->layout().data());
    QVERIFY(ttcc);
    QCOMPARE(ttcc->dateFormat(), u"ISO8601"_s);
}

void PropertyConfiguratorTest::testLegacyNamedLogger()
{
    Properties props;
    props.setProperty(u"log4j.appender.A1"_s, u"org.apache.log4j.ConsoleAppender"_s);
    props.setProperty(u"log4j.appender.A1.layout"_s, u"org.apache.log4j.SimpleLayout"_s);
    props.setProperty(u"log4j.rootLogger"_s, u"ALL, A1"_s);
    props.setProperty(u"log4j.logger.com.myapp"_s, u"ERROR, A1"_s);
    props.setProperty(u"log4j.additivity.com.myapp"_s, u"false"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *myApp = LogManager::logger(u"com.myapp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->additivity(), false);
    QCOMPARE(myApp->appenders().count(), 1);
}

void PropertyConfiguratorTest::testLegacyCategoryAlias()
{
    Properties props;
    props.setProperty(u"log4j.appender.A1"_s, u"org.apache.log4j.ConsoleAppender"_s);
    props.setProperty(u"log4j.appender.A1.layout"_s, u"org.apache.log4j.SimpleLayout"_s);
    props.setProperty(u"log4j.rootCategory"_s, u"INFO, A1"_s);
    props.setProperty(u"log4j.category.com.myapp"_s, u"WARN, A1"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    QCOMPARE(LogManager::rootLogger()->level(), Level::INFO_INT);
    Logger *myApp = LogManager::logger(u"com.myapp"_s);
    QCOMPARE(myApp->level(), Level::WARN_INT);
}

void PropertyConfiguratorTest::testLegacyGlobalSettings()
{
    Logger *logLogger = LogManager::logLogger();
    logLogger->setLevel(Level::INFO_INT);

    Properties props;
    props.setProperty(u"log4j.Debug"_s, u"TRACE"_s);
    props.setProperty(u"log4j.threshold"_s, u"WARN"_s);

    QVERIFY(PropertyConfigurator::configure(props));
    QCOMPARE(logLogger->level(), Level(Level::TRACE_INT));
    QCOMPARE(LogManager::loggerRepository()->threshold(), Level(Level::WARN_INT));
}

void PropertyConfiguratorTest::testLegacyMultipleAppenders()
{
    Properties props;
    props.setProperty(u"log4j.appender.console"_s, u"org.apache.log4j.ConsoleAppender"_s);
    props.setProperty(u"log4j.appender.console.layout"_s, u"org.apache.log4j.SimpleLayout"_s);
    props.setProperty(u"log4j.appender.null0"_s, u"Log4Qt::NullAppender"_s);
    props.setProperty(u"log4j.rootLogger"_s, u"ALL, console, null0"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::ALL_INT);
    QCOMPARE(root->appenders().count(), 2);
}

void PropertyConfiguratorTest::testLegacyRealWorldConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    const QString file = dir.path() + "/test.properties";
    const QByteArray content = QString(
        "logpath=%1\n"
        "log4j.reset=true\n"
        "log4j.threshold=NULL\n"
        "log4j.handleQtMessages=true\n"
        "\n"
        "log4j.appender.console=org.apache.log4j.ConsoleAppender\n"
        "log4j.appender.console.target=STDOUT_TARGET\n"
        "log4j.appender.console.threshold=OFF\n"
        "log4j.appender.console.layout=org.apache.log4j.TTCCLayout\n"
        "log4j.appender.console.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz\n"
        "log4j.appender.console.layout.contextPrinting=true\n"
        "\n"
        "log4j.appender.daily=Log4Qt::DailyRollingFileAppender\n"
        "log4j.appender.daily.file=${logpath}/myapp.log\n"
        "log4j.appender.daily.appendFile=true\n"
        "log4j.appender.daily.datePattern=_yyyy_MM_dd\n"
        "log4j.appender.daily.layout=org.apache.log4j.TTCCLayout\n"
        "log4j.appender.daily.layout.dateFormat=dd.MM.yyyy hh:mm:ss.zzz\n"
        "log4j.appender.daily.layout.contextPrinting=true\n"
        "\n"
        "log4j.rootLogger=ALL, console, daily\n"
        "\n"
        "log4j.logger.MyApp=ERROR, console\n"
        "log4j.additivity.MyApp=false\n"
    ).arg(logDir).toUtf8();

    writePropertiesFile(file, content);
    QVERIFY(PropertyConfigurator::configure(file));

    // Root logger
    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::ALL_INT);

    // Verify both appenders exist
    auto appenders = root->appenders();
    ConsoleAppender *consoleApp = nullptr;
    DailyRollingFileAppender *dailyApp = nullptr;
    for (const auto &a : appenders)
    {
        if (a->name() == u"console"_s)
            consoleApp = qobject_cast<ConsoleAppender *>(a.data());
        else if (a->name() == u"daily"_s)
            dailyApp = qobject_cast<DailyRollingFileAppender *>(a.data());
    }

    // Console appender
    QVERIFY(consoleApp);
    QCOMPARE(consoleApp->target(), u"STDOUT_TARGET"_s);
    QCOMPARE(consoleApp->threshold(), Level::OFF_INT);
    auto *consoleTtcc = qobject_cast<TTCCLayout *>(consoleApp->layout().data());
    QVERIFY(consoleTtcc);
    QCOMPARE(consoleTtcc->dateFormat(), u"dd.MM.yyyy hh:mm:ss.zzz"_s);

    // Daily file appender
    QVERIFY(dailyApp);
    QVERIFY(dailyApp->file().contains(u"myapp"_s));
    QCOMPARE(dailyApp->appendFile(), true);

    // Named logger
    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->additivity(), false);
}

void PropertyConfiguratorTest::testLegacyVariableSubstitution()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    // Variable defined without log4j. prefix, used in log4j. keys
    Properties props;
    props.setProperty(u"logpath"_s, logDir);
    props.setProperty(u"log4j.appender.daily"_s, u"Log4Qt::DailyRollingFileAppender"_s);
    props.setProperty(u"log4j.appender.daily.file"_s, u"${logpath}/app.log"_s);
    props.setProperty(u"log4j.appender.daily.appendFile"_s, u"true"_s);
    props.setProperty(u"log4j.appender.daily.datePattern"_s, u"_yyyy_MM_dd"_s);
    props.setProperty(u"log4j.appender.daily.layout"_s, u"org.apache.log4j.TTCCLayout"_s);
    props.setProperty(u"log4j.rootLogger"_s, u"ALL, daily"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    auto *dailyApp = qobject_cast<DailyRollingFileAppender *>(root->appenders().first().data());
    QVERIFY(dailyApp);
    QVERIFY(dailyApp->file().contains(u"app"_s));
    QVERIFY(dailyApp->file().contains(logDir));
}

void PropertyConfiguratorTest::testLegacyCrossReferenceSubstitution()
{
    // Test that ${log4j.*} references in values resolve correctly after translation,
    // e.g. reusing one appender's layout class in another appender.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    Properties props;
    props.setProperty(u"logpath"_s, logDir);
    props.setProperty(u"log4j.appender.console"_s, u"org.apache.log4j.ConsoleAppender"_s);
    props.setProperty(u"log4j.appender.console.layout"_s, u"org.apache.log4j.TTCCLayout"_s);
    props.setProperty(u"log4j.appender.console.layout.dateFormat"_s, u"ISO8601"_s);
    props.setProperty(u"log4j.appender.daily"_s, u"Log4Qt::DailyRollingFileAppender"_s);
    props.setProperty(u"log4j.appender.daily.file"_s, u"${logpath}/app.log"_s);
    props.setProperty(u"log4j.appender.daily.appendFile"_s, u"true"_s);
    props.setProperty(u"log4j.appender.daily.datePattern"_s, u"_yyyy_MM_dd"_s);
    props.setProperty(u"log4j.appender.daily.layout"_s, u"${log4j.appender.console.layout}"_s);
    props.setProperty(u"log4j.appender.daily.layout.dateFormat"_s, u"${log4j.appender.console.layout.dateFormat}"_s);
    props.setProperty(u"log4j.rootLogger"_s, u"ALL, console, daily"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->appenders().count(), 2);

    DailyRollingFileAppender *dailyApp = nullptr;
    for (const auto &a : root->appenders())
    {
        if (a->name() == u"daily"_s)
            dailyApp = qobject_cast<DailyRollingFileAppender *>(a.data());
    }
    QVERIFY(dailyApp);
    auto *ttcc = qobject_cast<TTCCLayout *>(dailyApp->layout().data());
    QVERIFY(ttcc);
    QCOMPARE(ttcc->dateFormat(), u"ISO8601"_s);
}

// Regression test: a circular ${...} reference — a plausible configuration
// typo like 'logpath=${logpath}/logs' — used to recurse without bound and
// crash the process with a stack overflow during configure().
void PropertyConfiguratorTest::testCircularSubstitution()
{
    Properties props;

    // Direct self-reference
    props.setProperty(u"self"_s, u"${self}/logs"_s);
    QCOMPARE(OptionConverter::findAndSubst(props, u"self"_s), u"/logs"_s);

    // Mutual cycle
    props.setProperty(u"ping"_s, u"${pong}"_s);
    props.setProperty(u"pong"_s, u"${ping}"_s);
    QCOMPARE(OptionConverter::findAndSubst(props, u"ping"_s), u""_s);

    // Legitimate nested substitution keeps working, and the same key may
    // appear more than once as long as there is no cycle
    props.setProperty(u"base"_s, u"/var"_s);
    props.setProperty(u"logdir"_s, u"${base}/log"_s);
    props.setProperty(u"file"_s, u"${logdir}/app.log (${base})"_s);
    QCOMPARE(OptionConverter::findAndSubst(props, u"file"_s),
             u"/var/log/app.log (/var)"_s);
}

// Regression test: the search for the closing bracket started at the scan
// position instead of at the '${' that was found, so a literal '}' earlier
// in the value was picked up as the closing bracket — producing a bogus key
// lookup, duplicated text and a corrupted value.
void PropertyConfiguratorTest::testSubstitutionWithLiteralClosingBrace()
{
    Properties props;
    props.setProperty(u"key"_s, u"VALUE"_s);

    props.setProperty(u"braceBefore"_s, u"ab}c${key}"_s);
    QCOMPARE(OptionConverter::findAndSubst(props, u"braceBefore"_s),
             u"ab}cVALUE"_s);

    props.setProperty(u"braceOnly"_s, u"plain}text"_s);
    QCOMPARE(OptionConverter::findAndSubst(props, u"braceOnly"_s),
             u"plain}text"_s);
}

// Regression test: Factory::doSetObjectProperty() had no conversion for enum
// property types, so StringMatchFilter's caseSensitivity property — added
// specifically to be configurable — failed with ConfiguratorUnknownTypeError
// from every configuration format.
void PropertyConfiguratorTest::testEnumPropertyFromConfig()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s, u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s, u"SimpleLayout"_s);
    props.setProperty(u"appender.console.filter.f1.type"_s, u"StringMatch"_s);
    props.setProperty(u"appender.console.filter.f1.stringToMatch"_s, u"needle"_s);
    props.setProperty(u"appender.console.filter.f1.caseSensitivity"_s,
                      u"CaseInsensitive"_s);
    props.setProperty(u"rootLogger.level"_s, u"ALL"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s, u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    Logger *root = LogManager::rootLogger();
    auto *consoleApp = qobject_cast<ConsoleAppender *>(root->appenders().first().data());
    QVERIFY(consoleApp);

    auto *stringMatch =
        qobject_cast<StringMatchFilter *>(consoleApp->filter().data());
    QVERIFY(stringMatch);
    QCOMPARE(stringMatch->stringToMatch(), u"needle"_s);
    QCOMPARE(stringMatch->caseSensitivity(), Qt::CaseInsensitive);
}

// --- HeaderFooterProvider configuration ---

void PropertyConfiguratorTest::testGlobalHeaderFooterProvider()
{
    Properties props;
    props.setProperty(u"headerFooterProvider.type"_s,          u"PatternHeaderFooterProvider"_s);
    props.setProperty(u"headerFooterProvider.headerPattern"_s, u"Session start"_s);
    props.setProperty(u"headerFooterProvider.footerPattern"_s, u"Session end"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    auto provider = AbstractLayout::globalHeaderFooterProvider();
    QVERIFY(provider);
    QCOMPARE(provider->header(), u"Session start"_s);
    QCOMPARE(provider->footer(), u"Session end"_s);
}

void PropertyConfiguratorTest::testPerLayoutHeaderFooterProvider()
{
    Properties props;
    props.setProperty(u"appender.console.type"_s,                                          u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s,                                   u"PatternLayout"_s);
    props.setProperty(u"appender.console.layout.headerFooterProvider.type"_s,              u"PatternHeaderFooterProvider"_s);
    props.setProperty(u"appender.console.layout.headerFooterProvider.headerPattern"_s,     u"File opened"_s);
    props.setProperty(u"rootLogger.level"_s,                                               u"INFO"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s,                                   u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    auto *consoleApp = qobject_cast<ConsoleAppender *>(
        LogManager::rootLogger()->appenders().first().data());
    QVERIFY(consoleApp);

    auto *layout = qobject_cast<PatternLayout *>(consoleApp->layout().data());
    QVERIFY(layout);

    auto provider = layout->headerFooterProvider();
    QVERIFY(provider);
    QCOMPARE(provider->header(), u"File opened"_s);
    // Global provider is not set — only the per-layout one
    QVERIFY(!AbstractLayout::globalHeaderFooterProvider());
}

void PropertyConfiguratorTest::testLegacyGlobalHeaderFooterProvider()
{
    Properties props;
    props.setProperty(u"log4j.headerFooterProvider.type"_s,          u"PatternHeaderFooterProvider"_s);
    props.setProperty(u"log4j.headerFooterProvider.headerPattern"_s, u"Legacy header"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    auto provider = AbstractLayout::globalHeaderFooterProvider();
    QVERIFY(provider);
    QCOMPARE(provider->header(), u"Legacy header"_s);
}

void PropertyConfiguratorTest::testPerLayoutProviderOverridesGlobal()
{
    Properties props;
    props.setProperty(u"headerFooterProvider.type"_s,                                      u"PatternHeaderFooterProvider"_s);
    props.setProperty(u"headerFooterProvider.headerPattern"_s,                             u"Global header"_s);
    props.setProperty(u"appender.console.type"_s,                                          u"Console"_s);
    props.setProperty(u"appender.console.layout.type"_s,                                   u"PatternLayout"_s);
    props.setProperty(u"appender.console.layout.headerFooterProvider.type"_s,              u"PatternHeaderFooterProvider"_s);
    props.setProperty(u"appender.console.layout.headerFooterProvider.headerPattern"_s,     u"Per-layout header"_s);
    props.setProperty(u"rootLogger.level"_s,                                               u"INFO"_s);
    props.setProperty(u"rootLogger.appenderRef.0.ref"_s,                                   u"console"_s);

    QVERIFY(PropertyConfigurator::configure(props));

    // Global provider is set
    auto globalProvider = AbstractLayout::globalHeaderFooterProvider();
    QVERIFY(globalProvider);
    QCOMPARE(globalProvider->header(), u"Global header"_s);

    // Per-layout provider wins over global
    auto *consoleApp = qobject_cast<ConsoleAppender *>(
        LogManager::rootLogger()->appenders().first().data());
    QVERIFY(consoleApp);
    auto *layout = qobject_cast<PatternLayout *>(consoleApp->layout().data());
    QVERIFY(layout);
    QCOMPARE(layout->header(), u"Per-layout header"_s);
}

QTEST_GUILESS_MAIN(PropertyConfiguratorTest)
#include "tst_propertyconfigurator.moc"
