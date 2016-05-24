#ifndef LOGGEROBJECTPRIO_H
#define LOGGEROBJECTPRIO_H

#include "logger.h"

#include <QObject>

class LoggerObjectPrio : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    explicit LoggerObjectPrio(QObject *parent = 0);

private slots:
    void onTimeout();

};

#endif // LOGGEROBJECTPRIO_H
