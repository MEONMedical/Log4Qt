#include "helpers/binaryclasslogger.h"

#include <QtCore/QDebug>
#include <QtCore/QString>

#include "logmanager.h"
#include "binarylogger.h"

#include <QString>

namespace Log4Qt
{

BinaryLogger *BinaryClassLogger::logger(const QObject *pObject)
{
    Q_ASSERT_X(pObject, "BinaryClassLogger::logger()", "pObject must not be null");
    static Logger* mpLogger(LogManager::logger(QString(pObject->metaObject()->className()) + QStringLiteral("@@binary@@")));
    return qobject_cast<BinaryLogger *>(mpLogger);
}

} // namespace Log4Qt
