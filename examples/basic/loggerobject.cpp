#include "loggerobject.h"

#include <QTimer>

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

    mCounter++;
    if (mCounter >= 10)
        emit exit(0);
}
