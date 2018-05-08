/******************************************************************************
 *
 * package:     Log4Qt
 * file:        denyallfilter.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes      Feb 2009, Martin Heinrich
 *              - Fixed a compile error on VS 2008 by using Q_UNUSED(&event)
 *                instead of Q_UNUSED(event)
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

#ifndef LOG4QT_DENYALLFILTER_H
#define LOG4QT_DENYALLFILTER_H

#include <log4qt/spi/filter.h>

namespace Log4Qt
{

/*!
 * \brief The class DenyAllFilter drops all logging events
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class  LOG4QT_EXPORT DenyAllFilter : public Filter
{
    Q_OBJECT

public:
    DenyAllFilter(QObject *parent = nullptr);

    Decision decide(const LoggingEvent &event) const override
    {
        Q_UNUSED(&event);
        return Filter::DENY;
    }
};

} // namespace Log4Qt

#endif // LOG4QT_DENYALLFILTER_H
