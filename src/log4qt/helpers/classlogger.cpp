/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
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
    mLogger(nullptr)
{
}

Logger *ClassLogger::logger(const QObject *object)
{
    Q_ASSERT_X(object, "ClassLogger::logger()", "pObject must not be null");
    if (mLogger.load(std::memory_order_acquire) == nullptr)
    {
        Logger *expected = nullptr;
        Logger *resolved = LogManager::logger(QString::fromLatin1(object->metaObject()->className()));
        mLogger.compare_exchange_strong(expected, resolved,
                                        std::memory_order_acq_rel, std::memory_order_acquire);
    }
    return mLogger.load(std::memory_order_acquire);
}

} // namespace Log4Qt
