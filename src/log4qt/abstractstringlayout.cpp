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

#include "abstractstringlayout.h"

#include "loggingevent.h"

using namespace Qt::StringLiterals;

namespace Log4Qt
{

AbstractStringLayout::AbstractStringLayout(QObject *parent)
    : AbstractLayout(parent)
    , mCharset(u"UTF-8"_s)
{
}

QString AbstractStringLayout::contentType() const
{
    return u"text/plain; charset=%1"_s.arg(mCharset);
}

void AbstractStringLayout::formatTo(const LoggingEvent &event, QByteArray &dest)
{
    dest += format(event).toUtf8();
}

QByteArray &AbstractStringLayout::threadLocalBuffer()
{
    thread_local QByteArray buf;
    return buf;
}

} // namespace Log4Qt

#include "moc_abstractstringlayout.cpp"
