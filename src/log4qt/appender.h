/******************************************************************************
 *
 * package:
 * file:        appender.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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
 ********************************************************************************/

#ifndef LOG4QT_APPENDER_H
#define LOG4QT_APPENDER_H

#include "spi/filter.h"
#include "layout.h"
#include "helpers/classlogger.h"

namespace Log4Qt
{

class LoggingEvent;

/*!
 * \brief The class Appender is the base class for all Appenders.
 *
 * To allow the whole hirarchy to be an ascendant of QObject Appender is
 * not an interface.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT Appender : public QObject
{
    Q_OBJECT

    /*!
     * The property holds the Layout used by the Appender.
     *
     * \sa layout(), setLayout()
     */
    Q_PROPERTY(LayoutSharedPtr layout READ layout WRITE setLayout)

    /*!
     * The property holds the name of the Appender.
     *
     * \sa name(), setName()
     */
    Q_PROPERTY(QString name READ name WRITE setName)

    /*!
     * The property holds if the Appender requires a Layout or not.
     *
     * \sa requiresLayout(), setRequiresLayout()
     */
    Q_PROPERTY(bool requiresLayout READ requiresLayout)

public:
    Appender(QObject *pParent = Q_NULLPTR);
    virtual ~Appender();

    virtual FilterSharedPtr filter() const = 0;
    virtual QString name() const = 0;
    virtual LayoutSharedPtr layout() const = 0;
    virtual bool requiresLayout() const = 0;
    virtual void setLayout(LayoutSharedPtr pLayout) = 0;
    virtual void setName(const QString &rName) = 0;

    virtual void addFilter(FilterSharedPtr pFilter) = 0;
    virtual void clearFilters() = 0;
    virtual void close() = 0;
    virtual void doAppend(const LoggingEvent &rEvent) = 0;

protected:
#ifndef QT_NO_DEBUG_STREAM
    /*!
     * Writes all object member variables to the given debug stream
     * \a rDebug and returns the stream.
     *
     * The member function is used by
     * QDebug operator<<(QDebug debug, const Filter &rFilter) to
     * generate class specific output.
     *
     * \sa QDebug operator<<(QDebug debug, const Filter &rFilter)
     */
    virtual QDebug debug(QDebug &rDebug) const = 0;

    // Needs to be friend to access internal data
    friend QDebug operator<<(QDebug debug,
                             const Appender &rAppender);
#endif // QT_NO_DEBUG_STREAM
    /*!
     * Returns a pointer to a Logger named after of the object.
     *
     * \sa Logger::logger(const char *pName)
     */
    Logger *logger() const;

private:
    Q_DISABLE_COPY(Appender)
    mutable ClassLogger mLog4QtClassLogger;

};

class LOG4QT_EXPORT AppenderSharedPtr : public QSharedPointer<Appender>
{
public:
    AppenderSharedPtr(Appender *ptr)
        : QSharedPointer<Appender>(ptr, &Appender::deleteLater)
    {}

    AppenderSharedPtr() : QSharedPointer<Appender>()
    {}

    AppenderSharedPtr(const QSharedPointer<Appender> &other) :
        QSharedPointer<Appender>(other)
    {}

    AppenderSharedPtr(const QWeakPointer<Appender> &other) :
        QSharedPointer<Appender>(other)
    {}
};

} // namespace Log4Qt

#endif // LOG4QT_APPENDER_H
