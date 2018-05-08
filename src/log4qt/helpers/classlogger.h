/******************************************************************************
 *
 * package:     Log4Qt
 * file:        classlogger.h
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

#ifndef LOG4QT_CLASSLOGGER_H
#define LOG4QT_CLASSLOGGER_H

#include <log4qt/log4qtshared.h>

#include <QAtomicPointer>

class QObject;

namespace Log4Qt
{
class Logger;

/*!
 * \brief The class ClassLogger provides logging for a QObject derived
 *        class.
 *
 * The class ClassLogger provides a logger for a specified QObject derived
 * object. It is used by \ref LOG4QT_DECLARE_QCLASS_LOGGER to implement the
 * member functions provided by the macro.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \sa LOG4QT_DECLARE_QCLASS_LOGGER
 */
class LOG4QT_EXPORT ClassLogger
{
public:
    /*!
     * Creates a ClassLogger object.
     */
    ClassLogger();

    /*!
     * Returns a pointer to a Logger named after the class of the object
     * \a pObject.
     *
     * On the first invocation the Logger is requested by a call to
     * LogManager::logger(const char *pName). The pointer is stored to be
     * returned on subsequent invocations.
     *
     * \sa LogManager::logger(const char *pName)
     */
    Logger *logger(const QObject *object);

private:
    mutable QAtomicPointer<Logger> mLogger;
};

} // namespace Log4Qt

#endif // LOG4QT_CLASSLOGGER_H
