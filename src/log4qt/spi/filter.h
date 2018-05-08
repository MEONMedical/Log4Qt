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

#include <log4qt/log4qt.h>
#include <log4qt/log4qtsharedptr.h>

#include <QObject>

namespace Log4Qt
{

class LoggingEvent;
class Filter;

using FilterSharedPtr = Log4QtSharedPtr<Filter>;

/*!
 * \brief The class Filter is the base class for all filters.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class  LOG4QT_EXPORT Filter : public QObject
{
    Q_OBJECT

    /*!
     * The property holds the next filter of this filter.
     *
     * The default is 0 for no next filter.
     *
     * \sa next(), setNext()
     */
    Q_PROPERTY(FilterSharedPtr next READ next WRITE setNext)

public:
    enum Decision
    {
        ACCEPT,
        DENY,
        NEUTRAL
    };
    Q_ENUM(Decision)

public:
    Filter(QObject *parent = nullptr);
    virtual ~Filter();

    FilterSharedPtr next() const;
    void setNext(const FilterSharedPtr &filter);

    virtual void activateOptions();
    virtual Decision decide(const LoggingEvent &event) const = 0;

private:
    FilterSharedPtr mNext;
};


} // namespace Log4Qt


#endif // LOG4QT_FILTER_H
