#include "loggerobjectprio.h"

#include <QTimer>

LoggerObjectPrio::LoggerObjectPrio(QObject *parent) : QObject(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LoggerObjectPrio::onTimeout);
    timer->start(1);
}

void LoggerObjectPrio::onTimeout()
{
    logger()->debug() << "Debug output";
    logger()->error() << "Error output";
}
