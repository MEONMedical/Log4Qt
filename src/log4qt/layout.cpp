/******************************************************************************
 *
 * package:     Log4Qt
 * file:        layout.cpp
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

#include "layout.h"

#include "loggingevent.h"
#include "logmanager.h"

namespace Log4Qt
{

Layout::Layout(QObject *parent) :
    QObject(parent)
{}

Layout::~Layout() = default;

QString Layout::contentType() const
{
    return QStringLiteral("text/plain");
}

void Layout::activateOptions()
{
}

QString Layout::endOfLine()
{
    // There seams to be no function in Qt for this. MinGW enter '\r\n' automatically
    return QStringLiteral("\n");
}

} // namespace Log4Qt

#include "moc_layout.cpp"
