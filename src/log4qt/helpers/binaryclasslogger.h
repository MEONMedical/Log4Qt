#ifndef LOG4QT_BINARYCLASSLOGGER_H
#define LOG4QT_BINARYCLASSLOGGER_H

#include <log4qt/log4qtshared.h>

#include <QAtomicPointer>

class QObject;
namespace Log4Qt
{

class Logger;
class BinaryLogger;

class LOG4QT_EXPORT BinaryClassLogger
{
public:
    BinaryClassLogger();
    BinaryLogger *logger(const QObject *object);

private:
    mutable QAtomicPointer<BinaryLogger> mLogger;
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYCLASSLOGGER_H
