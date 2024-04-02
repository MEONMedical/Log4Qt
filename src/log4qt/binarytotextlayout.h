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

#ifndef LOG4QT_BINARYTOTEXTLAYOUT_H
#define LOG4QT_BINARYTOTEXTLAYOUT_H

#include "layout.h"

namespace Log4Qt
{

//! Used to format binary messages for a textual appenders
class LOG4QT_EXPORT BinaryToTextLayout : public Layout
{
    Q_OBJECT
    Q_PROPERTY(LayoutSharedPtr subLayout READ subLayout WRITE setSubLayout)
public:
    explicit BinaryToTextLayout(const LayoutSharedPtr &subLayout = LayoutSharedPtr(), QObject *parent = nullptr);

    virtual QString format(const LoggingEvent &event) override;

    LayoutSharedPtr subLayout() const {return mSubLayout;}
    void setSubLayout(const LayoutSharedPtr &layout) {mSubLayout = layout;}

private:
    Q_DISABLE_COPY_MOVE(BinaryToTextLayout)
    LayoutSharedPtr mSubLayout;
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYTOTEXTLAYOUT_H
