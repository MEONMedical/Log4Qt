#include <QtCore/QtGlobal>

#if defined(LOG4QT_EXPORTS)
#  define LOG4QT_EXPORT Q_DECL_EXPORT
#elif defined (LOG4QT_IMPORTS)
#  define LOG4QT_EXPORT Q_DECL_IMPORT
#else
#  define LOG4QT_EXPORT   /**/
#endif
