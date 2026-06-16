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
#include "log4qt/jsonconfigurator.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/patternlayout.h"
#include "log4qt/ttcclayout.h"

using namespace Qt::StringLiterals;

using namespace Log4Qt;

class JsonConfiguratorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();
    void testFlattenSimpleKeys();
    void testFlattenNestedObjects();
    void testFlattenBooleans();
    void testFlattenNumbers();
    void testFlattenNull();
    void testFlattenFullConfig();
    void testConfigureFromFile();
    void testRealWorldConfig();
    void testConfigureMissingFile();
    void testConfigureMalformedJson();
    void testConfigureNonObjectRoot();

private:
    void writeJsonFile(const QString &path, const QByteArray &content);
};

void JsonConfiguratorTest::cleanup()
{
    LogManager::resetConfiguration();
}

void JsonConfiguratorTest::writeJsonFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(content);
    file.close();
}

void JsonConfiguratorTest::testFlattenSimpleKeys()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "rootLogger": {
            "level": "ALL"
        }
    })");

    QVERIFY(JsonConfigurator::configure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::ALL_INT);
}

void JsonConfiguratorTest::testFlattenNestedObjects()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "appender": {
            "console": {
                "type": "Console",
                "target": "STDOUT_TARGET",
                "layout": {
                    "type": "TTCCLayout",
                    "dateFormat": "ISO8601"
                }
            }
        },
        "rootLogger": {
            "level": "DEBUG",
            "appenderRef": {
                "0": { "ref": "console" }
            }
        }
    })");

    QVERIFY(JsonConfigurator::configure(file));

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

void JsonConfiguratorTest::testFlattenBooleans()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "appender": {
            "console": {
                "type": "Console",
                "layout": {
                    "type": "TTCCLayout",
                    "contextPrinting": false
                }
            }
        },
        "rootLogger": {
            "level": "DEBUG",
            "appenderRef": {
                "0": { "ref": "console" }
            }
        }
    })");

    QVERIFY(JsonConfigurator::configure(file));
    Logger *root = LogManager::rootLogger();
    auto appenders = root->appenders();
    QVERIFY(!appenders.isEmpty());
    auto *ttcc = qobject_cast<TTCCLayout *>(appenders.first()->layout().data());
    QVERIFY(ttcc);
    QCOMPARE(ttcc->contextPrinting(), false);
}

void JsonConfiguratorTest::testFlattenNumbers()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "rootLogger": {
            "level": "INFO"
        }
    })");

    QVERIFY(JsonConfigurator::configure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::INFO_INT);
}

void JsonConfiguratorTest::testFlattenNull()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "rootLogger": {
            "level": "ERROR"
        },
        "nullprop": null
    })");

    QVERIFY(JsonConfigurator::configure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::ERROR_INT);
}

void JsonConfiguratorTest::testFlattenFullConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "appender": {
            "console": {
                "type": "Console",
                "target": "STDOUT_TARGET",
                "layout": {
                    "type": "TTCCLayout",
                    "dateFormat": "ISO8601"
                }
            }
        },
        "rootLogger": {
            "level": "ALL",
            "appenderRef": {
                "0": { "ref": "console" }
            }
        },
        "logger": {
            "MyApp": {
                "name": "MyApp",
                "level": "ERROR",
                "additivity": "false",
                "appenderRef": {
                    "0": { "ref": "console" }
                }
            }
        }
    })");

    QVERIFY(JsonConfigurator::configure(file));

    Logger *root = LogManager::rootLogger();
    QCOMPARE(root->level(), Level::ALL_INT);

    Logger *myApp = LogManager::logger(u"MyApp"_s);
    QCOMPARE(myApp->level(), Level::ERROR_INT);
    QCOMPARE(myApp->additivity(), false);
}

void JsonConfiguratorTest::testConfigureFromFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/test.json";
    writeJsonFile(file, R"({
        "appender": {
            "a1": {
                "type": "Console",
                "layout": {
                    "type": "SimpleLayout"
                }
            }
        },
        "rootLogger": {
            "level": "TRACE",
            "appenderRef": {
                "0": { "ref": "a1" }
            }
        }
    })");

    QVERIFY(JsonConfigurator::doConfigure(file));
    QCOMPARE(LogManager::rootLogger()->level(), Level::TRACE_INT);
}

void JsonConfiguratorTest::testRealWorldConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString logDir = dir.path() + "/logging";
    QVERIFY(QDir().mkpath(logDir));

    const QString file = dir.path() + "/test.json";
    const QByteArray json = QString(R"({
        "logpath": "%1",
        "reset": "true",
        "threshold": "NULL",
        "handleQtMessages": "true",
        "appender": {
            "console": {
                "type": "Console",
                "target": "STDOUT_TARGET",
                "threshold": "OFF",
                "layout": {
                    "type": "TTCCLayout",
                    "dateFormat": "dd.MM.yyyy hh:mm:ss.zzz",
                    "contextPrinting": true
                }
            },
            "daily": {
                "type": "DailyFile",
                "file": "${logpath}/idlmapp.log",
                "appendFile": "true",
                "datePattern": "_yyyy_MM_dd",
                "layout": {
                    "type": "TTCCLayout",
                    "dateFormat": "dd.MM.yyyy hh:mm:ss.zzz",
                    "contextPrinting": "true"
                }
            }
        },
        "rootLogger": {
            "level": "ALL",
            "appenderRef": {
                "console": { "ref": "console" },
                "daily": { "ref": "daily" }
            }
        }
    })").arg(logDir).toUtf8();

    writeJsonFile(file, json);
    QVERIFY(JsonConfigurator::configure(file));

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

void JsonConfiguratorTest::testConfigureMissingFile()
{
    QVERIFY(!JsonConfigurator::configure(u"/nonexistent/path/to/file.json"_s));
}

void JsonConfiguratorTest::testConfigureMalformedJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/bad.json";
    writeJsonFile(file, "{ not valid json }}}");

    QVERIFY(!JsonConfigurator::configure(file));
}

void JsonConfiguratorTest::testConfigureNonObjectRoot()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString file = dir.path() + "/array.json";
    writeJsonFile(file, "[1, 2, 3]");

    QVERIFY(!JsonConfigurator::configure(file));
}

QTEST_GUILESS_MAIN(JsonConfiguratorTest)
#include "tst_jsonconfigurator.moc"
