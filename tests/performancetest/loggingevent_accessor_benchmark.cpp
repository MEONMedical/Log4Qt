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

/*
 * Benchmark for investigation target I-004.
 *
 * LoggingEvent's string accessors (message(), ndc(), threadName(),
 * loggername(), ...) return QString *by value*. Each call is a copy-on-write
 * QString copy: one atomic refcount increment when the temporary is created
 * and one decrement when it dies. PatternFormatter touches ~5-10 of these per
 * event, so the open question is whether switching the accessors to
 * `const QString &` (an ABI break) is worth it.
 *
 * The benchmark answers two things:
 *   1. byValueAccessors vs byConstRefAccessors — the raw per-event cost of the
 *      refcount churn in isolation (upper bound on what the ABI break could
 *      save).
 *   2. fullPatternFormat — the same accessor work as a fraction of a real
 *      PatternLayout::format() call, which is the context that actually
 *      decides whether the saving matters.
 */

#include <QtTest>
#include <QString>
#include <QHash>

#include "log4qt/loggingevent.h"
#include "log4qt/logger.h"
#include "log4qt/level.h"
#include "log4qt/patternlayout.h"

using namespace Log4Qt;

namespace
{
// Minimal stand-in that mirrors LoggingEvent's string fields but exposes them
// through `const QString &` accessors — models the by-reference alternative
// (no COW refcount churn) as a lower bound.
struct RefEvent
{
    QString mMessage;
    QString mNdc;
    QString mThreadName;
    QString mLoggerName;

    [[nodiscard]] const QString &message() const { return mMessage; }
    [[nodiscard]] const QString &ndc() const { return mNdc; }
    [[nodiscard]] const QString &threadName() const { return mThreadName; }
    [[nodiscard]] const QString &loggername() const { return mLoggerName; }
};
}

class LoggingEventAccessorBenchmark : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // (1) Isolated accessor cost: by-value (current) vs const-ref (proposed).
    void byValueAccessors();
    void byConstRefAccessors();

    // (2) Context: the same per-event work inside a real layout format.
    void fullPatternFormat();

private:
    Logger *mLogger = nullptr;
    std::unique_ptr<LoggingEvent> mEvent;
    RefEvent mRefEvent;
    std::unique_ptr<PatternLayout> mLayout;
};

void LoggingEventAccessorBenchmark::initTestCase()
{
    mLogger = Logger::logger(QStringLiteral("benchmark.module.component.Worker"));

    const QString message = QStringLiteral(
        "Processing request 12345 for user alice from 10.0.0.7 took 42ms");
    const QString ndc = QStringLiteral("request-context");
    QHash<QString, QString> properties;
    properties.insert(QStringLiteral("requestId"), QStringLiteral("12345"));
    properties.insert(QStringLiteral("user"), QStringLiteral("alice"));
    const QString threadName = QStringLiteral("worker-thread-3");
    const qint64 timeStamp = Q_INT64_C(1700000000000);

    mEvent = std::make_unique<LoggingEvent>(mLogger, Level::INFO_INT, message,
                                            ndc, properties, threadName, timeStamp);

    mRefEvent.mMessage = message;
    mRefEvent.mNdc = ndc;
    mRefEvent.mThreadName = threadName;
    mRefEvent.mLoggerName = mLogger->name();

    mLayout = std::make_unique<PatternLayout>(QStringLiteral("%d [%t] %-5p %c - %m%n"));
    mLayout->activateOptions();
}

void LoggingEventAccessorBenchmark::byValueAccessors()
{
    const LoggingEvent &e = *mEvent;
    qsizetype sink = 0;
    QBENCHMARK {
        // Mirror a PatternFormatter pass: each converter pulls one field.
        sink += e.message().size();
        sink += e.ndc().size();
        sink += e.threadName().size();
        sink += e.loggername().size();
        sink += e.message().size();   // %m and %l both touch message
    }
    QVERIFY(sink >= 0);
}

void LoggingEventAccessorBenchmark::byConstRefAccessors()
{
    const RefEvent &e = mRefEvent;
    qsizetype sink = 0;
    QBENCHMARK {
        sink += e.message().size();
        sink += e.ndc().size();
        sink += e.threadName().size();
        sink += e.loggername().size();
        sink += e.message().size();
    }
    QVERIFY(sink >= 0);
}

void LoggingEventAccessorBenchmark::fullPatternFormat()
{
    const LoggingEvent &e = *mEvent;
    qsizetype sink = 0;
    QBENCHMARK {
        sink += mLayout->format(e).size();
    }
    QVERIFY(sink >= 0);
}

QTEST_MAIN(LoggingEventAccessorBenchmark)

#include "loggingevent_accessor_benchmark.moc"
