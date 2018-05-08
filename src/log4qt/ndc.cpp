/******************************************************************************
 *
 * package:     Log4Qt
 * file:        ndc.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes      Feb 2009, Martin Heinrich
 *              - Fixed VS 2008 unreferenced formal parameter warning by using
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

#include "ndc.h"

#include "helpers/initialisationhelper.h"
#include "logger.h"

#include <QMutex>
#include <QThread>

namespace Log4Qt
{

LOG4QT_DECLARE_STATIC_LOGGER(logger, Log4Qt: NDC)

void NDC::clear()
{
    if (!instance()->mStack.hasLocalData())
        return;

    instance()->mStack.localData()->clear();
}


int NDC::depth()
{
    if (!instance()->mStack.hasLocalData())
        return 0;

    return instance()->mStack.localData()->count();
}


LOG4QT_IMPLEMENT_INSTANCE(NDC)


QString NDC::pop()
{
    if (!instance()->mStack.hasLocalData() || instance()->mStack.localData()->isEmpty())
    {
        logger()->warn(QStringLiteral("Requesting pop from empty NDC stack"));
        return QString();
    }

    return instance()->mStack.localData()->pop();
}


void NDC::push(const QString &message)
{
    if (!instance()->mStack.hasLocalData())
        instance()->mStack.setLocalData(new QStack<QString>);

    instance()->mStack.localData()->push(message);
}


void NDC::setMaxDepth(int maxDepth)
{
    if (!instance()->mStack.hasLocalData() ||
            instance()->mStack.localData()->size() <= maxDepth)
        return;

    instance()->mStack.localData()->resize(maxDepth);
}


QString NDC::peek()
{
    if (!instance()->mStack.hasLocalData() ||
            instance()->mStack.localData()->isEmpty())
        return QString();

    return instance()->mStack.localData()->top();
}

} // namespace Log4Qt
