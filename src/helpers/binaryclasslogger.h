#ifndef LOG4QT_BINARYCLASSLOGGER_H
#define LOG4QT_BINARYCLASSLOGGER_H

#include "../log4qtshared.h"

#include <QtCore/QObject>

namespace Log4Qt
{

class Logger;
class BinaryLogger;

class LOG4QT_EXPORT BinaryClassLogger
{
public:
    BinaryLogger *logger(const QObject *pObject);
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYCLASSLOGGER_H
