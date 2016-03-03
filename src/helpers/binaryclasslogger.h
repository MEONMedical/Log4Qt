#ifndef LOG4QT_BINARYCLASSLOGGER_H
#define LOG4QT_BINARYCLASSLOGGER_H

#include "log4qtshared.h"

#include <QObject>
#include <QAtomicPointer>

namespace Log4Qt
{

class Logger;
class BinaryLogger;

class LOG4QT_EXPORT BinaryClassLogger
{
public:
    BinaryClassLogger();
    BinaryLogger *logger(const QObject *pObject);

private:
    mutable QAtomicPointer<BinaryLogger> mpLogger;
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYCLASSLOGGER_H
