#ifndef LOGGEROBJECTPRIO_H
#define LOGGEROBJECTPRIO_H

#include "log4qt/logger.h"

#include <QObject>

class LoggerObjectPrio : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    explicit LoggerObjectPrio(QObject *parent = nullptr);

private slots:
    void onTimeout();

};

#endif // LOGGEROBJECTPRIO_H
