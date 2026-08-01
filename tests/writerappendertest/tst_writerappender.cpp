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
#include <QBuffer>
#include <QTextStream>

#include <atomic>

#include "log4qt/writerappender.h"
#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/simplelayout.h"

using namespace Log4Qt;

LOG4QT_DECLARE_STATIC_LOGGER(test_logger, Test::WriterAppender)

// ---------------------------------------------------------------------------
// WriterRemovingAppender — tears down the writer from within preAppend()
// (Phase 4, outside the appender lock), simulating a concurrent close() /
// setWriter(nullptr) racing doAppend() between Phase 3 and Phase 5.
// ---------------------------------------------------------------------------
class WriterRemovingAppender : public WriterAppender
{
    Q_OBJECT
public:
    using WriterAppender::WriterAppender;

    void removeWriterOnNextAppend() { mRemoveWriter.store(true); }
protected:
    void preAppend(const LoggingEvent &event, const LayoutSharedPtr &layout) override
    {
        WriterAppender::preAppend(event, layout);
        if (mRemoveWriter.exchange(false))
            setWriter(nullptr);
    }
private:
    std::atomic<bool> mRemoveWriter{false};
};

class WriterAppenderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanup();

    void WriterAppender_basicAppend();
    void WriterAppender_writerRemovedBetweenPhasesIsSafe();
};

void WriterAppenderTest::cleanup()
{
    LogManager::resetConfiguration();
}

void WriterAppenderTest::WriterAppender_basicAppend()
{
    QBuffer buffer;
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QTextStream stream(&buffer);

    WriterAppender appender;
    appender.setName(QStringLiteral("Writer"));
    appender.setLayout(LayoutSharedPtr(new SimpleLayout));
    appender.setWriter(&stream);
    appender.activateOptions();
    QVERIFY(appender.isActive());

    appender.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                   QStringLiteral("first message")));
    appender.setWriter(nullptr); // detach before the local stream dies

    QVERIFY(buffer.data().contains("first message"));
}

// Regression test: a writer torn down while doAppend() had released the lock
// (Phase 4) used to be dereferenced as a null pointer in Phase 5, because
// only isActive() was re-checked. Phase 5 must re-validate the entry
// conditions and skip the event instead.
void WriterAppenderTest::WriterAppender_writerRemovedBetweenPhasesIsSafe()
{
    QBuffer buffer;
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QTextStream stream(&buffer);

    WriterRemovingAppender appender;
    appender.setName(QStringLiteral("Writer"));
    appender.setLayout(LayoutSharedPtr(new SimpleLayout));
    appender.setWriter(&stream);
    appender.activateOptions();

    appender.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                   QStringLiteral("before removal")));

    appender.removeWriterOnNextAppend();
    appender.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                   QStringLiteral("during removal")));

    // A further append on the writer-less appender must be skipped, too
    appender.doAppend(LoggingEvent(test_logger(), Level::INFO_INT,
                                   QStringLiteral("after removal")));

    QVERIFY(buffer.data().contains("before removal"));
    QVERIFY(!buffer.data().contains("during removal"));
    QVERIFY(!buffer.data().contains("after removal"));
}

QTEST_MAIN(WriterAppenderTest)
#include "tst_writerappender.moc"
