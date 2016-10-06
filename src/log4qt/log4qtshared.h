#ifndef LOG4QT_SHARED_H
#define LOG4QT_SHARED_H

#include <QtGlobal>

// Define LOG4QT_STATIC in you applikation if you want to link against the
// static version of Log4Qt

#ifdef LOG4QT_STATIC
#   define LOG4QT_EXPORT
#else
#  if defined(LOG4QT_LIBRARY)
#    define LOG4QT_EXPORT Q_DECL_EXPORT
#  else
#    define LOG4QT_EXPORT Q_DECL_IMPORT
#  endif
#endif

#endif // LOG4QT_SHARED_H
