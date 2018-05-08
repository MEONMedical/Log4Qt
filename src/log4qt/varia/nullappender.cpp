/******************************************************************************
 *
 * package:     Log4Qt
 * file:        nullappender.cpp
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

#include "varia/nullappender.h"

#include "layout.h"
#include "loggingevent.h"



namespace Log4Qt
{

NullAppender::NullAppender(QObject *parent) :
    AppenderSkeleton(false, parent)
{
}

NullAppender::~NullAppender()
{
    close();
}

void NullAppender::append(const LoggingEvent &event)
{
    Q_UNUSED(event);
}

} // namespace Log4Qt

#include "moc_nullappender.cpp"
