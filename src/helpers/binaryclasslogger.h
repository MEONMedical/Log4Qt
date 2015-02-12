#ifndef LOG4QT_BINARYCLASSLOGGER_H
#define LOG4QT_BINARYCLASSLOGGER_H

#include "../log4qtshared.h"

#include <QtCore/QObject>
#if QT_VERSION >= QT_VERSION_CHECK(4, 4, 0)
#	include <QtCore/QAtomicPointer>
#	ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE
#		warning "QAtomicPointer test and set is not native. The class Log4Qt::BinaryClassLogger is not thread-safe."
#	endif
#endif

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
