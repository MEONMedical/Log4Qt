/******************************************************************************
 *
 * package:     Log4Qt
 * file:        classlogger.cpp
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:     Sep 2008, Martin Heinrich:
 *              - Replaced usage of q_atomic_test_and_set_ptr with
 *                QAtomicPointer
 *
 *
 * Copyright 2007 - 2008 Martin Heinrich
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
 ******************************************************************************/

#include "logmanager.h"
#include "helpers/classlogger.h"

namespace Log4Qt
{

ClassLogger::ClassLogger() :
    mpLogger(Q_NULLPTR)
{
}

Logger *ClassLogger::logger(const QObject *pObject)
{
    Q_ASSERT_X(pObject, "ClassLogger::logger()", "pObject must not be null");
    if (!static_cast<Logger *>(mpLogger.loadAcquire()))
        mpLogger.testAndSetOrdered(0,
                                   LogManager::logger(QLatin1String(pObject->metaObject()->className())));
    return const_cast<Logger *>(static_cast<Logger *>(mpLogger.loadAcquire()));
}

} // namespace Log4Qt
