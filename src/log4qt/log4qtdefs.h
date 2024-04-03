#ifndef LOG4QTDEFS_H
#define LOG4QTDEFS_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtClassHelperMacros>
#endif

// use Q_DISABLE_COPY for Qt version prior to 5.13.0
#ifndef Q_DISABLE_COPY_MOVE
#define Q_DISABLE_COPY_MOVE(Class) \
Q_DISABLE_COPY(Class)
#endif

#endif // LOG4QTDEFS_H
