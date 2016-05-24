#ifndef LOGGEROBJECT_H
#define LOGGEROBJECT_H

#include "logger.h"

#include <QObject>

class LoggerObject : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    explicit LoggerObject(QObject *parent = 0);

signals:
    void exit(int code);

private slots:
    void onTimeout();

private:
    int mCounter;
};

#endif // LOGGEROBJECT_H
