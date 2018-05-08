/******************************************************************************
 *
 * package:     Log4Qt
 * file:        datetime.cpp
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

#include "helpers/datetime.h"

#include "helpers/initialisationhelper.h"

namespace Log4Qt
{

QString DateTime::toString(const QString &format) const
{
    if (format.isEmpty())
        return QString();
    if (!isValid())
        return QString();

    if (format == QStringLiteral("NONE"))
        return QString();
    if (format == QStringLiteral("RELATIVE"))
        return QString::number(toMSecsSinceEpoch() - InitialisationHelper::startTime());
    if (format == QStringLiteral("ISO8601"))
        return formatDateTime(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
    if (format == QStringLiteral("ABSOLUTE"))
        return formatDateTime(QStringLiteral("HH:mm:ss.zzz"));
    if (format == QStringLiteral("DATE"))
        return formatDateTime(QStringLiteral("dd MM yyyy HH:mm:ss.zzz"));

    return formatDateTime(format);
}

QString DateTime::formatDateTime(const QString &format) const
{
    return QDateTime::toString(format);
}

} // namespace Log4Qt
