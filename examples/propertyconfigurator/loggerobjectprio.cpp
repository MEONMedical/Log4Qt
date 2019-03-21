#include "loggerobjectprio.h"

#include <QTimer>

LoggerObjectPrio::LoggerObjectPrio(QObject *parent) : QObject(parent)
{
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LoggerObjectPrio::onTimeout);
    timer->start(1);
}

void LoggerObjectPrio::onTimeout()
{
    logger()->debug() << "Debug output";
    logger()->error() << "Error output";
}

#include "moc_loggerobjectprio.cpp"
