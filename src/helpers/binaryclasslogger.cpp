#include "helpers/binaryclasslogger.h"

#include <QtCore/QDebug>
#include <QtCore/QString>

#include "logmanager.h"
#include "binarylogger.h"

namespace Log4Qt
{

BinaryClassLogger::BinaryClassLogger()
    : mpLogger(0)
{
}

BinaryLogger *BinaryClassLogger::logger(const QObject *pObject)
{
    Q_ASSERT_X(pObject, "BinaryClassLogger::logger()", "parent must not be null");
    QString loggerName(pObject->metaObject()->className());
    loggerName+= QStringLiteral("@@binary@@");
    if (!mpLogger.loadAcquire())
        mpLogger.testAndSetOrdered(0, qobject_cast<BinaryLogger *>(LogManager::logger(loggerName)));
    return mpLogger.loadAcquire();
}

} // namespace Log4Qt
