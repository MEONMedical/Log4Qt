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

#include <QTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include "log4qt/telnetappender.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/simplelayout.h"

using namespace Log4Qt;

// Exposes the protected clients() list so tests can observe registration
// and removal of client connections.
class TestTelnetAppender : public TelnetAppender
{
public:
    using TelnetAppender::TelnetAppender;
    using TelnetAppender::clients;
};

class TelnetAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();

    void TelnetAppender_clientReceivesEvents();
    void TelnetAppender_synchronousDisconnectDuringAppend();

private:
    static int findFreePort();
};

void TelnetAppenderTest::cleanup()
{
    LogManager::resetConfiguration();
}

int TelnetAppenderTest::findFreePort()
{
    QTcpServer probe;
    if (!probe.listen(QHostAddress::LocalHost, 0))
        return -1;
    const int port = probe.serverPort();
    probe.close();
    return port;
}

void TelnetAppenderTest::TelnetAppender_clientReceivesEvents()
{
    const int port = findFreePort();
    QVERIFY(port > 0);

    TestTelnetAppender appender;
    appender.setName(QStringLiteral("Telnet"));
    appender.setLayout(LayoutSharedPtr(new SimpleLayout));
    appender.setAddress(QHostAddress(QHostAddress::LocalHost));
    appender.setPort(port);
    appender.setWelcomeMessage(QStringLiteral("WELCOME\r\n"));
    appender.activateOptions();
    QVERIFY(appender.isActive());

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, port);
    QVERIFY(client.waitForConnected(3000));
    QTRY_COMPARE(appender.clients().size(), 1);

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("TELNET_TEST_MESSAGE")));

    // Both ends live in this thread: spin the event loop (QTRY_) so the
    // server side actually flushes its buffered writes.
    QByteArray received;
    QTRY_VERIFY(([&received, &client] {
        received += client.readAll();
        return received.contains("WELCOME")
            && received.contains("TELNET_TEST_MESSAGE");
    }()));

    appender.close();
}

// Regression test: when a client's connection is broken, a same-thread
// append() runs the write lambda synchronously; the failed flush aborts the
// socket, which emits disconnected() synchronously, and onClientDisconnected()
// re-enters through the recursive mutex and removes the socket from the
// client list. append() used to range-iterate the member list itself, so the
// mid-iteration removals invalidated its iterators (UB). With two broken
// sockets ahead of a healthy one, the stale iteration skipped one removal
// and delivered the event to the healthy client twice.
void TelnetAppenderTest::TelnetAppender_synchronousDisconnectDuringAppend()
{
    const int port = findFreePort();
    QVERIFY(port > 0);

    TestTelnetAppender appender;
    appender.setName(QStringLiteral("Telnet"));
    appender.setLayout(LayoutSharedPtr(new SimpleLayout));
    appender.setAddress(QHostAddress(QHostAddress::LocalHost));
    appender.setPort(port);
    appender.setImmediateFlush(true);
    appender.activateOptions();
    QVERIFY(appender.isActive());

    // Two clients that will break their connections, then a healthy one.
    // Connect strictly in order so the appender's client list is ordered
    // [broken1, broken2, healthy].
    QTcpSocket broken1, broken2, healthy;
    broken1.connectToHost(QHostAddress::LocalHost, port);
    QVERIFY(broken1.waitForConnected(3000));
    QTRY_COMPARE(appender.clients().size(), 1);
    broken2.connectToHost(QHostAddress::LocalHost, port);
    QVERIFY(broken2.waitForConnected(3000));
    QTRY_COMPARE(appender.clients().size(), 2);
    healthy.connectToHost(QHostAddress::LocalHost, port);
    QVERIFY(healthy.waitForConnected(3000));
    QTRY_COMPARE(appender.clients().size(), 3);

    // Send data the broken clients never read: aborting with unread data
    // makes the kernel reset the connection (RST) instead of closing it
    // orderly, so the server's next flush to those sockets fails
    // synchronously inside append().
    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("PRIME_EVENT")));
    QThread::msleep(100);
    broken1.abort();
    broken2.abort();

    // Give the RSTs a moment to arrive WITHOUT processing events, so the
    // appender still has the broken sockets in its list when append() runs.
    QThread::msleep(200);
    QCOMPARE(appender.clients().size(), 3);

    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("MARKER_EVENT")));

    // Both broken sockets must be dropped from the client list —
    // synchronously or after event processing.
    QTRY_COMPARE(appender.clients().size(), 1);

    // A subsequent event must still reach the remaining client
    appender.doAppend(LoggingEvent(LogManager::rootLogger(), Level::INFO_INT,
                                   QStringLiteral("SECOND_EVENT")));

    QByteArray received;
    while (healthy.waitForReadyRead(1000))
        received += healthy.readAll();

    // Each event must arrive exactly once — the stale-slot iteration of the
    // unfixed code delivered MARKER_EVENT twice.
    QCOMPARE(received.count("PRIME_EVENT"), 1);
    QCOMPARE(received.count("MARKER_EVENT"), 1);
    QCOMPARE(received.count("SECOND_EVENT"), 1);

    appender.close();
}

QTEST_MAIN(TelnetAppenderTest)
#include "tst_telnetappender.moc"
