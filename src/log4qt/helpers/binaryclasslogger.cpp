#include "helpers/binaryclasslogger.h"

#include "logmanager.h"
#include "binarylogger.h"

#include <QString>

namespace Log4Qt
{

BinaryClassLogger::BinaryClassLogger()
    : mpLogger(nullptr)
{
}

BinaryLogger *BinaryClassLogger::logger(const QObject *pObject)
{
    Q_ASSERT_X(pObject, "BinaryClassLogger::logger()", "pObject must not be null");
    QString loggerName(pObject->metaObject()->className());
    loggerName += QStringLiteral("@@binary@@");
    if (!mpLogger.loadAcquire())
        mpLogger.testAndSetOrdered(nullptr, qobject_cast<BinaryLogger *>(LogManager::logger(loggerName)));
    return mpLogger.loadAcquire();
}

} // namespace Log4Qt
