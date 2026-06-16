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

#include "jsonlayout.h"

#include "loggingevent.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

JsonLayout::JsonLayout(QObject *parent)
    : AbstractStringLayout(parent)
{
}

QString JsonLayout::contentType() const
{
    return u"application/json; charset=UTF-8"_s;
}

bool JsonLayout::requiresLocation() const
{
    return mIncludeLocation;
}

QString JsonLayout::format(const LoggingEvent &event)
{
    QJsonObject obj;

    if (mIncludeTimestamp)
        obj[u"timestamp"_s] = event.timeStamp();

    if (mIncludeLevel)
        obj[u"level"_s] = event.level().toString();

    if (mIncludeLogger)
    {
        const QString loggerName = event.logger()
                                   ? event.logger()->name()
                                   : event.categoryName();
        obj[u"logger"_s] = loggerName;
    }

    if (mIncludeThread)
        obj[u"thread"_s] = event.threadName();

    if (mIncludeMessage)
        obj[u"message"_s] = event.message();

    if (mIncludeNdc)
    {
        const QString ndc = event.ndc();
        if (!ndc.isEmpty())
            obj[u"ndc"_s] = ndc;
    }

    if (mIncludeMdc)
    {
        const QHash<QString, QString> mdc = event.mdc();
        if (!mdc.isEmpty())
        {
            QJsonObject mdcObj;
            for (auto it = mdc.cbegin(); it != mdc.cend(); ++it)
                mdcObj[it.key()] = it.value();
            obj[u"mdc"_s] = mdcObj;
        }
    }

    if (mIncludeLocation)
    {
        const MessageContext ctx = event.context();
        if (ctx.file)
            obj[u"file"_s] = QString::fromLatin1(ctx.file);
        obj[u"line"_s] = ctx.line;
        if (ctx.function)
            obj[u"function"_s] = QString::fromLatin1(ctx.function);
    }

    const QJsonDocument doc(obj);
    return doc.toJson(mPrettyPrint ? QJsonDocument::Indented : QJsonDocument::Compact)
           + AbstractLayout::endOfLine();
}

} // namespace Log4Qt

#include "moc_jsonlayout.cpp"
