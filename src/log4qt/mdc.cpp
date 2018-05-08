/******************************************************************************
 *
 * package:     Log4Qt
 * file:        mdc.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes      Feb 2009, Martin Heinrich
 *              - Fixed unreferenced formal parameter warning by using
 *                Q_UNUSED in operator<<.
 *
 *
 * Copyright 2007 - 2009 Martin Heinrich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************/

#include "mdc.h"

#include <QMutex>
#include <QThread>
#include "helpers/initialisationhelper.h"
#include "logger.h"


namespace Log4Qt
{

QString MDC::get(const QString &key)
{
    if (!instance()->mHash.hasLocalData())
        return QString();

    return instance()->mHash.localData()->value(key);
}

QHash<QString, QString> MDC::context()
{
    if (!instance()->mHash.hasLocalData())
        return QHash<QString, QString>();

    return *instance()->mHash.localData();
}

LOG4QT_IMPLEMENT_INSTANCE(MDC)

QHash<QString, QString> *MDC::localData()
{
    if (!instance()->mHash.hasLocalData())
        instance()->mHash.setLocalData(new QHash<QString, QString>);
    return instance()->mHash.localData();
}
} // namespace Log4Qt
