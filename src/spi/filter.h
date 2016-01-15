/******************************************************************************
 *
 * package:     Log4Qt
 * file:        filter.h
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
 ******************************************************************************/

#ifndef LOG4QT_FILTER_H
#define LOG4QT_FILTER_H

#include "helpers/logobject.h"

#include "helpers/logobjectptr.h"
#include "../log4qt.h"

namespace Log4Qt
{

class LoggingEvent;

/*!
 * \brief The class Filter is the base class for all filters.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class  LOG4QT_EXPORT Filter : public LogObject
{
    Q_OBJECT

    /*!
     * The property holds the next filter of this filter.
     *
     * The default is 0 for no next filter.
     *
     * \sa next(), setNext()
     */
    Q_PROPERTY(Filter* next READ next WRITE setNext)

public:
    enum Decision
    {
        ACCEPT,
        DENY,
        NEUTRAL
    };
    Q_ENUMS(Decision)

public:
    Filter(QObject *pParent = nullptr);
    virtual ~Filter();

    Filter* next() const
    { return mpNext; }

    void setNext(Filter *pFilter);

    virtual void activateOptions();
    virtual Decision decide(const LoggingEvent &rEvent) const = 0;

private:
    LogObjectPtr<Filter> mpNext;
};


} // namespace Log4Qt


// Q_DECLARE_TYPEINFO(Log4Qt::Filter, Q_COMPLEX_TYPE); // Use default
Q_DECLARE_TYPEINFO(Log4Qt::LogObjectPtr<Log4Qt::Filter>, Q_MOVABLE_TYPE);


#endif // LOG4QT_FILTER_H
