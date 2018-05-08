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

#include "layout.h"
#include "log4qtsharedptr.h"
#include "spi/filter.h"
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
    Appender(QObject *parent = nullptr);
    virtual ~Appender() = default;

    virtual FilterSharedPtr filter() const = 0;
    virtual QString name() const = 0;
    virtual LayoutSharedPtr layout() const = 0;
    virtual bool requiresLayout() const = 0;
    virtual void setLayout(const LayoutSharedPtr &layout) = 0;
    virtual void setName(const QString &name) = 0;

    virtual void addFilter(const FilterSharedPtr &filter) = 0;
    virtual void clearFilters() = 0;
    virtual void close() = 0;
    virtual void doAppend(const LoggingEvent &event) = 0;

protected:
    /*!
     * Returns a pointer to a Logger named after of the object.
     *
     * \sa Logger::logger()
     */
    Logger *logger() const;

private:
    Q_DISABLE_COPY(Appender)
    mutable ClassLogger mLog4QtClassLogger;
};

using AppenderSharedPtr = Log4QtSharedPtr<Appender>;

} // namespace Log4Qt

#endif // LOG4QT_APPENDER_H
