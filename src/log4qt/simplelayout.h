/******************************************************************************
 *
 * package:     Log4Qt
 * file:        simplelayout.h
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

#ifndef LOG4QT_SIMPLELAYOUT_H
#define LOG4QT_SIMPLELAYOUT_H

#include "layout.h"

namespace Log4Qt
{

/*!
 * \brief The class SimpleLayout outputs the level and message of a logging
 *        event.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT  SimpleLayout : public Layout
{
    Q_OBJECT
    Q_PROPERTY(bool showLevel READ showLevel WRITE setShowLevel)

public:
    SimpleLayout(QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(SimpleLayout)
    bool mShowLevel;

public:
    bool showLevel() const;
    void setShowLevel(bool show);

    QString format(const LoggingEvent &event) override;

};

inline SimpleLayout::SimpleLayout(QObject *parent)
    : Layout(parent)
    , mShowLevel{true}
{
}

inline bool SimpleLayout::showLevel() const
{
    return mShowLevel;
}

inline void SimpleLayout::setShowLevel(bool show)
{
    mShowLevel = show;
}

} // namespace Log4Qt

#endif // LOG4QT_SIMPLELAYOUT_H
