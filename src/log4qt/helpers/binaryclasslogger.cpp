#include "helpers/binaryclasslogger.h"

#include "logmanager.h"
#include "binarylogger.h"

#include <QString>

namespace Log4Qt
{

BinaryClassLogger::BinaryClassLogger()
    : mLogger(nullptr)
{
}

BinaryLogger *BinaryClassLogger::logger(const QObject *object)
{
    Q_ASSERT_X(object, "BinaryClassLogger::logger()", "object must not be null");
    QString loggename(object->metaObject()->className());
    loggename += QStringLiteral("@@binary@@");
    if (!mLogger.loadAcquire())
        mLogger.testAndSetOrdered(nullptr, qobject_cast<BinaryLogger *>(LogManager::logger(loggename)));
    return mLogger.loadAcquire();
}

} // namespace Log4Qt
