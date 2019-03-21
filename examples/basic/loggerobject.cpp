#include "loggerobject.h"

#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(category1, "test.category1")

LoggerObject::LoggerObject(QObject *parent) : QObject(parent),
    mCounter(0)
{
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LoggerObject::onTimeout);
    timer->start(10);
}

void LoggerObject::onTimeout()
{
    logger()->debug() << "Debug output";
    logger()->error() << "Error output";
    logger()->debug(QStringLiteral("test"));

    qCCritical(category1, "a debug message");

    l4qError(QStringLiteral("an error"));
    l4qDebug(QStringLiteral("debug info"));

    l4qError() << "an error via stream";
    l4qError(QStringLiteral("an error with param %1"), 10);
    mCounter++;
    if (mCounter >= 10)
        emit exit(0);
}

#include "moc_loggerobject.cpp"
