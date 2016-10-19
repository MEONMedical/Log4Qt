#include "helpers/binaryclasslogger.h"

#include "logmanager.h"
#include "binarylogger.h"

#include <QString>

namespace Log4Qt
{

BinaryClassLogger::BinaryClassLogger()
    : mpLogger(0)
{
}

BinaryLogger *BinaryClassLogger::logger(const QObject *pObject)
{
    Q_ASSERT_X(pObject, "BinaryClassLogger::logger()", "pObject must not be null");
    QString loggerName(pObject->metaObject()->className());
    loggerName += QStringLiteral("@@binary@@");
    if (!mpLogger.loadAcquire())
        mpLogger.testAndSetOrdered(0, qobject_cast<BinaryLogger *>(LogManager::logger(loggerName)));
    return mpLogger.loadAcquire();
}

} // namespace Log4Qt
