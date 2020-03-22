/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#include "binarytotextlayout.h"
#include "binaryloggingevent.h"
#include "simplelayout.h"

#include <QStringBuilder>

namespace Log4Qt
{

BinaryToTextLayout::BinaryToTextLayout(const LayoutSharedPtr &subLayout, QObject *parent)
    : Layout(parent)
    , mSubLayout(subLayout)
{
}

QString BinaryToTextLayout::format(const LoggingEvent &event)
{
    if (!mSubLayout.isNull())
    {
        if (const auto *binaryEvent = dynamic_cast<const BinaryLoggingEvent *>(&event))
        {
            QString hexData = binaryEvent->binaryMessage().toHex();
            QString spacedHexData;

            for (int i = 0; i < hexData.length(); i += 2)
                spacedHexData.append(hexData.mid(i, 2) % " ");

            // replace binary marker in output with hexdump
            return mSubLayout->format(event).replace(Log4Qt::BinaryLoggingEvent::binaryMarker(), QStringLiteral("%1 bytes: %2").arg(binaryEvent->binaryMessage().size()).arg(spacedHexData));
        }
    }
    return QString();
}

} // namespace Log4Qt

#include "moc_binarytotextlayout.cpp"
