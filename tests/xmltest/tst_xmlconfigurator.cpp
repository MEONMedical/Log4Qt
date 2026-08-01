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
#include <QTemporaryFile>
#include <QtTest>

#include "log4qt/consoleappender.h"
#include "log4qt/dailyrollingfileappender.h"
#include "log4qt/helpers/configuratorhelper.h"
#include "log4qt/helpers/properties.h"
#include "log4qt/xmlconfigurator.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/patternlayout.h"
#include "log4qt/ttcclayout.h"

using namespace Qt::StringLiterals;

using namespace Log4Qt;

class XmlConfiguratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();
    void testFlattenSimpleKeys();
    void testFlattenNestedElements();
    void testFlattenAttributes();
    void testFlattenBooleanStrings();
    void testFlattenFullConfig();
    void testConfigureFromFile();
    void testRealWorldConfig();
    void testConfigureMissingFile();
    void testConfigureMalformedXml();
    void testDocumentationExample();

private:
    void writeXmlFile(const QString &path, const QByteArray &content);
};

void XmlConfiguratorTest::cleanup()
{
    LogManager::resetConfiguration();
}

void XmlConfiguratorTest::writeXmlFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(content);
    file.close();
}

void XmlConfiguratorTest::testFlattenSimpleKeys()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <rootLogger>
        <level>ALL</level>
    </rootLogger>
</configuration>
)");

    QVERIFY(XmlConfigurator::configure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::ALL_INT);
}

void XmlConfiguratorTest::testFlattenNestedElements()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <appender>
        <console>
            <type>Console</type>
            <target>STDOUT_TARGET</target>
            <layout>
                <type>TTCCLayout</type>
                <dateFormat>ISO8601</dateFormat>
            </layout>
        </console>
    </appender>
    <rootLogger>
        <level>DEBUG</level>
        <appenderRef>
            <ref0>
                <ref>console</ref>
            </ref0>
        </appenderRef>
    </rootLogger>
</configuration>
)");

    QVERIFY(XmlConfigurator::configure(file));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::DEBUG_INT);

    // Verify the appender was created
    auto appenders = root->appenders();
    bool foundConsole = false;
    for (const auto &appender : appenders)
    {
        if (appender->name() == u"console"_s)
        {
            foundConsole = true;
            QVERIFY(qobject_cast<ConsoleAppender *>(appender.data()));
            QVERIFY(appender->layout());
            QVERIFY(qobject_cast<TTCCLayout *>(appender->layout().data()));
        }
    }
    QVERIFY(foundConsole);
}

void XmlConfiguratorTest::testFlattenAttributes()
{
    // Test that XML attributes on elements are flattened as child properties.
    // e.g. <console type="Console"> produces appender.console.type=Console
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <appender>
        <console type="Console" target="STDOUT_TARGET">
            <layout type="PatternLayout" conversionPattern="%-5p %c - %m%n"/>
        </console>
    </appender>
    <rootLogger level="WARN">
        <appenderRef>
            <ref0 ref="console"/>
        </appenderRef>
    </rootLogger>
</configuration>
)");

    QVERIFY(XmlConfigurator::configure(file));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::WARN_INT);
    auto appenders = root->appenders();
    QVERIFY(!appenders.isEmpty());
    QCOMPARE(appenders.first()->name(), u"console"_s);
    auto *consoleApp = qobject_cast<ConsoleAppender *>(appenders.first().data());
    QVERIFY(consoleApp);
    QCOMPARE(consoleApp->target(), u"STDOUT_TARGET"_s);
    auto *pattern = qobject_cast<PatternLayout *>(appenders.first()->layout().data());
    QVERIFY(pattern);
    QCOMPARE(pattern->conversionPattern(), u"%-5p %c - %m%n"_s);
}

void XmlConfiguratorTest::testFlattenBooleanStrings()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <appender>
        <console>
            <type>Console</type>
            <layout>
                <type>TTCCLayout</type>
                <contextPrinting>false</contextPrinting>
            </layout>
        </console>
    </appender>
    <rootLogger>
        <level>DEBUG</level>
        <appenderRef>
            <ref0>
                <ref>console</ref>
            </ref0>
        </appenderRef>
    </rootLogger>
</configuration>
)");

    QVERIFY(XmlConfigurator::configure(file));
    Logger *root = LogManager::rootLogger();
    auto appenders = root->appenders();
    QVERIFY(!appenders.isEmpty());
    auto *ttcc = qobject_cast<TTCCLayout *>(appenders.first()->layout().data());
    QVERIFY(ttcc);
    QCOMPARE(ttcc->contextPrinting(), false);
}

void XmlConfiguratorTest::testFlattenFullConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <appender>
        <console>
            <type>Console</type>
            <target>STDOUT_TARGET</target>
            <layout>
                <type>TTCCLayout</type>
                <dateFormat>ISO8601</dateFormat>
            </layout>
        </console>
    </appender>
    <rootLogger>
        <level>ALL</level>
        <appenderRef>
            <ref0>
                <ref>console</ref>
            </ref0>
        </appenderRef>
    </rootLogger>
    <logger>
        <MyApp>
            <name>MyApp</name>
            <level>ERROR</level>
            <additivity>false</additivity>
            <appenderRef>
                <ref0>
                    <ref>console</ref>
                </ref0>
            </appenderRef>
        </MyApp>
    </logger>
</configuration>
)");

    QVERIFY(XmlConfigurator::configure(file));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::ALL_INT);

    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->additivity(), false);
}

void XmlConfiguratorTest::testConfigureFromFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <appender>
        <a1>
            <type>Console</type>
            <layout>
                <type>SimpleLayout</type>
            </layout>
        </a1>
    </appender>
    <rootLogger>
        <level>TRACE</level>
        <appenderRef>
            <ref0>
                <ref>a1</ref>
            </ref0>
        </appenderRef>
    </rootLogger>
</configuration>
)");

    QVERIFY(XmlConfigurator::doConfigure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::TRACE_INT);
}

void XmlConfiguratorTest::testRealWorldConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    const QString file = dir.path() + "/test.xml";
    const QByteArray xml = QString(R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <logpath>%1</logpath>
    <reset>true</reset>
    <threshold>NULL</threshold>
    <handleQtMessages>true</handleQtMessages>
    <appender>
        <console>
            <type>Console</type>
            <target>STDOUT_TARGET</target>
            <threshold>OFF</threshold>
            <layout>
                <type>TTCCLayout</type>
                <dateFormat>dd.MM.yyyy hh:mm:ss.zzz</dateFormat>
                <contextPrinting>true</contextPrinting>
            </layout>
        </console>
        <daily>
            <type>DailyFile</type>
            <file>${logpath}/idlmapp.log</file>
            <appendFile>true</appendFile>
            <datePattern>_yyyy_MM_dd</datePattern>
            <layout>
                <type>TTCCLayout</type>
                <dateFormat>dd.MM.yyyy hh:mm:ss.zzz</dateFormat>
                <contextPrinting>true</contextPrinting>
            </layout>
        </daily>
    </appender>
    <rootLogger>
        <level>ALL</level>
        <appenderRef>
            <console>
                <ref>console</ref>
            </console>
            <daily>
                <ref>daily</ref>
            </daily>
        </appenderRef>
    </rootLogger>
</configuration>
)").arg(logDir).toUtf8();

    writeXmlFile(file, xml);
    QVERIFY(XmlConfigurator::configure(file));

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
    QVERIFY(dailyApp->file().contains(u"idlmapp"_s));
    QCOMPARE(dailyApp->appendFile(), true);
    QCOMPARE(dailyApp->datePattern(), u"_yyyy_MM_dd"_s);
    auto *dailyTtcc = qobject_cast<TTCCLayout *>(dailyApp->layout().data());
    QVERIFY(dailyTtcc);
    QCOMPARE(dailyTtcc->dateFormat(), u"dd.MM.yyyy hh:mm:ss.zzz"_s);
    QCOMPARE(dailyTtcc->contextPrinting(), true);
}

void XmlConfiguratorTest::testConfigureMissingFile()
{
    QVERIFY(!XmlConfigurator::configure(u"/nonexistent/path/to/file.xml"_s));
}

void XmlConfiguratorTest::testConfigureMalformedXml()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/bad.xml";
    writeXmlFile(file, "<not><valid><xml");

    QVERIFY(!XmlConfigurator::configure(file));
}

// Keeps the example in the xmlconfigurator.h class documentation honest:
// the previous doc showed an unsupported Log4j2-style dialect that silently
// configured nothing. This is the exact XML from the documentation.
void XmlConfiguratorTest::testDocumentationExample()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/doc.xml";
    writeXmlFile(file, R"(<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <appender>
        <console>
            <type>Console</type>
            <layout conversionPattern="%-5p %c - %m%n">
                <type>PatternLayout</type>
            </layout>
        </console>
    </appender>
    <rootLogger level="ALL">
        <appenderRef>
            <ref0 ref="console"/>
        </appenderRef>
    </rootLogger>
    <logger>
        <MyApp name="MyApp" level="ERROR" additivity="false">
            <appenderRef>
                <ref0 ref="console"/>
            </appenderRef>
        </MyApp>
    </logger>
</configuration>
)");

    QVERIFY(XmlConfigurator::configure(file));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::ALL_INT);
    QCOMPARE(root->appenders().count(), 1);
    auto *consoleApp = qobject_cast<ConsoleAppender *>(root->appenders().first().data());
    QVERIFY(consoleApp);
    auto *pattern = qobject_cast<PatternLayout *>(consoleApp->layout().data());
    QVERIFY(pattern);
    QCOMPARE(pattern->conversionPattern(), u"%-5p %c - %m%n"_s);

    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->additivity(), false);
    QCOMPARE(myApp->appenders().count(), 1);
}

QTEST_GUILESS_MAIN(XmlConfiguratorTest)
#include "tst_xmlconfigurator.moc"
