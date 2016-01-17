/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logobject.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:		Sep 2008, Martin Heinrich:
 * 				- Replaced usage of q_atomic_increment and q_atomic_decrement
 * 				  with QAtomicInt.
 *              Feb 2009, Martin Heinrich
 *              - Fixed a problem where the pParent parameter of the constructor
 *                was not passed on to the QObject constructor
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
 ******************************************************************************/

#ifndef LOG4QT_LOGOBJECT_H
#define LOG4QT_LOGOBJECT_H

#include "../log4qtshared.h"
#include "classlogger.h"

#include <QtCore/QObject>
#include <QtCore/QAtomicInt>

#ifndef Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE
#warning "QAtomicInt reference counting is not native. The class Log4Qt::LogObject is not thread-safe."
#endif

namespace Log4Qt
{

class Logger;

/*!
 * \brief The class LogObject is the common base class for many classes
 *        in the package.
 *
 * The class inherits QObject to allow its subclass to be accessed using
 * the Qt property system.
 *
 * LogObject objects provide a reference counter. A reference to the
 * object is established by calling retain() and freed by calling
 * release(). The object will delete itself when the reference counter
 * is decremented to 0.
 *
 * A class specific logger can be accessed over logger().
 *
 * The class also implements generic streaming to QDebug. Streaming an
 * object to QDebug will invoke debug() to create class specific output.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \sa \ref Ownership "Object ownership",
 *     LOG4QT_DECLARE_QCLASS_LOGGER
 */
class LOG4QT_EXPORT LogObject : public QObject
{
    Q_OBJECT

public:
    /*!
     * Creates a LogObject which is a child of \a pObject.
     */
    LogObject(QObject *pObject = 0);

    /*!
     * Destroys the LogObject.
     */
    virtual ~LogObject();

private:
    Q_DISABLE_COPY(LogObject)

public:
    /*!
     * Returns the value of the reference counter.
     */
    int referenceCount() const;

    /*!
     * Decrements the reference count of the object. If the reference count
     * count reaches zero and the object does not have a parent the object
     * is deleted.
     */
    void release();

    /*!
     * Increments the reference count of the object.
     */
    void retain();

protected:
#ifndef QT_NO_DEBUG_STREAM
    /*!
     * Writes all object member variables to the given debug stream
     * \a rDebug and returns the stream.
     *
     * The member function is used by
     * QDebug operator<<(QDebug debug, const LogObject &rLogObject) to
     * generate class specific output.
     *
     * \sa QDebug operator<<(QDebug debug, const LogObject &rLogObject)
     */
    virtual QDebug debug(QDebug &rDebug) const = 0;

    // Needs to be friend to access internal data
    friend QDebug operator<<(QDebug debug,
                             const LogObject &rLogObject);
#endif // QT_NO_DEBUG_STREAM

    /*!
     * Returns a pointer to a Logger named after of the object.
     *
     * \sa Logger::logger(const char *pName)
     */
    Logger* logger() const;

private:
    mutable QAtomicInt mReferenceCount;
    mutable ClassLogger mLog4QtClassLogger;
};

#ifndef QT_NO_DEBUG_STREAM
/*!
 * \relates LogObject
 *
 * Writes all object member variables to the given debug stream \a debug
 * and returns the stream.
 *
 * To handle sub-classing the function uses the virtual member function
 * debug(). This allows each class to generate its own output.
 *
 * \sa QDebug, debug()
 */
QDebug operator<<(QDebug debug,
                  const LogObject &rLogObject);
#endif

inline LogObject::LogObject(QObject *pParent) :
    QObject(pParent),
    mReferenceCount()
{}

inline LogObject::~LogObject()
{}

inline int LogObject::referenceCount() const
{
    return mReferenceCount.loadAcquire();
}

inline void LogObject::release()
{
    if (!mReferenceCount.deref())
        delete(this);
}

inline void LogObject::retain()
{
    mReferenceCount.ref();
}

inline Logger *LogObject::logger() const
{
    return mLog4QtClassLogger.logger(this);
}

} // namespace Log4Qt

#endif // LOG4QT_LOGOBJECT_H
