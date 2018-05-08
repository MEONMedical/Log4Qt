/******************************************************************************
 *
 * package:     Log4Qt
 * file:        debugappender.h
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

#ifndef LOG4QT_DEBUGAPPENDER_H
#define LOG4QT_DEBUGAPPENDER_H

#include <log4qt/appenderskeleton.h>

namespace Log4Qt
{

/*!
 * \brief The class DebugAppender appends logging events to the platform
 *        specific debug output.
 *
 * A DebugAppender appends to the Debugger on Windows and to stderr on all
 * other systems.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class  LOG4QT_EXPORT DebugAppender : public AppenderSkeleton
{
    Q_OBJECT

public:
    /*!
     * Creates a DebugAppender.
     */
    DebugAppender(QObject *parent = nullptr);

    /*!
     * Creates a DebugAppender with the specified layout \a pLayout
     */
    DebugAppender(const LayoutSharedPtr &layout,
                  QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(DebugAppender)

public:
    /*!
     * The DebugAppended requires a layout. The function returns true.
     *
     * \sa setLayout()
     */
    bool requiresLayout() const override;

protected:
    /*!
     * Appends the specified logging event \a event to the debug output.
     * The output is formatted using the appender's layout.
     *
     * The method is called by the AppenderSkeleton::doAppend() after it
     * the entry conditions have been tested and it has been found that the
     * logging event needs to be appended.
     *
     * \sa setLayout(), AppenderSkeleton::doAppend(), checkEntryConditions()
     */
    void append(const LoggingEvent &event) override;

};

inline DebugAppender::DebugAppender(QObject *parent) :
    AppenderSkeleton(parent)
{}


} // namespace Log4Qt

#endif // LOG4QT_DEBUGAPPENDER_H
