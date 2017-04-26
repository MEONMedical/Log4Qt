#include "loggerobject.h"

#include <QTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(category1, "test.category1")

LoggerObject::LoggerObject(QObject *parent) : QObject(parent),
    mCounter(0)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LoggerObject::onTimeout);
    timer->start(10);
}

void LoggerObject::onTimeout()
{
    logger()->debug() << "Debug output";
    logger()->error() << "Error output";
    logger()->debug("test");

    qCritical(category1, "a debug message");
    qCritical("Hello");

    mCounter++;
    if (mCounter >= 10)
        emit exit(0);
}
