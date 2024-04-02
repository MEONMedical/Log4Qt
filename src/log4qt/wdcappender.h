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

#ifndef LOG4QT_WDCAPPENDER_H
#define LOG4QT_WDCAPPENDER_H

#include "appenderskeleton.h"

namespace Log4Qt
{

class LOG4QT_EXPORT WDCAppender : public AppenderSkeleton
{
    Q_OBJECT
public:
    WDCAppender(QObject *parent = nullptr);
    WDCAppender(const LayoutSharedPtr &layout,
                QObject *parent = nullptr);

    virtual bool requiresLayout() const override;

protected:
    virtual void append(const LoggingEvent &event) override;

private:
    Q_DISABLE_COPY_MOVE(WDCAppender)
};

} // namespace Log4Qt

#endif //  LOG4QT_WDCAPPENDER_H
