/******************************************************************************
 *
 * package:     Log4Qt
 * file:        filter.cpp
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

#include "spi/filter.h"

#include <QDebug>

namespace Log4Qt
{

Filter::Filter(QObject *pParent) :
    QObject(pParent)
{}

Filter::~Filter()
{}

FilterSharedPtr Filter::next() const
{
    return mpNext;
}

void Filter::setNext(FilterSharedPtr pFilter)
{
    mpNext = pFilter;
}

void Filter::activateOptions()
{}

QDebug operator<<(QDebug debug, const Filter &rFilter)
{
    return rFilter.debug(debug);
}

FilterSharedPtr::FilterSharedPtr(Filter *ptr)
    : QSharedPointer<Filter>(ptr, &Filter::deleteLater)
{}

FilterSharedPtr::FilterSharedPtr() : QSharedPointer<Filter>()
{}

FilterSharedPtr::FilterSharedPtr(const QSharedPointer<Filter> &other) :
    QSharedPointer<Filter>(other)
{}

FilterSharedPtr::FilterSharedPtr(const QWeakPointer<Filter> &other) :
    QSharedPointer<Filter>(other)
{}

} // namespace Log4Qt
