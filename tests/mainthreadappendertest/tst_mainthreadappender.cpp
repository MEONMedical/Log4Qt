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
#include <QThread>

#include "log4qt/mainthreadappender.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/varia/listappender.h"

using namespace Log4Qt;

LOG4QT_DECLARE_STATIC_LOGGER(test_logger, Test::MainThreadAppender)

// ---------------------------------------------------------------------------
// ThreadCapturingListAppender — records the thread each event was appended on
// ---------------------------------------------------------------------------
class ThreadCapturingListAppender : public ListAppender
{
    Q_OBJECT
public:
    explicit ThreadCapturingListAppender(QObject *parent = nullptr)
        : ListAppender(parent) {}

    QList<QThread *> appendThreads() const
    {
        QMutexLocker locker(&mObjectGuard);
        return mAppendThreads;
    }
protected:
    void append(const LoggingEvent &event) override
    {
        mAppendThreads.append(QThread::currentThread());
        ListAppender::append(event);
    }
private:
    QList<QThread *> mAppendThreads;
};

class MainThreadAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();

    void MainThreadAppender_sameThreadEventsReachAppender();
    void MainThreadAppender_sameThreadLoggingViaLogger();
    void MainThreadAppender_crossThreadEventsDeliveredOnMainThread();
};

void MainThreadAppenderTest::cleanup()
{
    LogManager::resetConfiguration();
}

// Regression test: events logged on the application (main) thread used to be
// silently dropped, because the same-thread path called the attached
// appender's doAppend() while the thread-local recursion guard was already
// raised by the MainThreadAppender's own doAppend().
void MainThreadAppenderTest::MainThreadAppender_sameThreadEventsReachAppender()
{
    MainThreadAppender mta;
    mta.setName(QStringLiteral("MainThread"));

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    mta.addAppender(AppenderSharedPtr(list));
    mta.activateOptions();

    QCOMPARE(QThread::currentThread(), QCoreApplication::instance()->thread());

    mta.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                              QStringLiteral("from main thread")));

    QCOMPARE(list->list().size(), 1);
    QCOMPARE(list->list().at(0).message(), QStringLiteral("from main thread"));
}

void MainThreadAppenderTest::MainThreadAppender_sameThreadLoggingViaLogger()
{
    auto *mta = new MainThreadAppender;
    mta->setName(QStringLiteral("MainThread"));

    auto *list = new ListAppender;
    list->setName(QStringLiteral("List"));
    mta->addAppender(AppenderSharedPtr(list));
    mta->activateOptions();

    Logger *logger = LogManager::logger(QStringLiteral("Test::MainThreadAppender::ViaLogger"));
    logger->setLevel(Level::DEBUG_INT);
    logger->addAppender(AppenderSharedPtr(mta));

    logger->info(QStringLiteral("via logger"));

    QCOMPARE(list->list().size(), 1);
    QCOMPARE(list->list().at(0).message(), QStringLiteral("via logger"));

    logger->removeAllAppenders();
}

void MainThreadAppenderTest::MainThreadAppender_crossThreadEventsDeliveredOnMainThread()
{
    MainThreadAppender mta;
    mta.setName(QStringLiteral("MainThread"));

    auto *list = new ThreadCapturingListAppender;
    list->setName(QStringLiteral("List"));
    mta.addAppender(AppenderSharedPtr(list));
    mta.activateOptions();

    QThread worker;
    QObject context;
    context.moveToThread(&worker);
    worker.start();

    QMetaObject::invokeMethod(&context, [&mta] {
        mta.doAppend(LoggingEvent(test_logger(), Level::WARN_INT,
                                  QStringLiteral("from worker thread")));
    }, Qt::BlockingQueuedConnection);

    // Delivery happens via the main thread's event loop
    QTRY_COMPARE(list->list().size(), 1);
    QCOMPARE(list->list().at(0).message(), QStringLiteral("from worker thread"));
    QCOMPARE(list->appendThreads().at(0), QCoreApplication::instance()->thread());

    worker.quit();
    worker.wait();
}

QTEST_MAIN(MainThreadAppenderTest)
#include "tst_mainthreadappender.moc"
