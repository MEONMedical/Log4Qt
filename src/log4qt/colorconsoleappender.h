/******************************************************************************
 *
 * package:     log4qt
 * file:        colorconsoleappender.h
 * created:     March 2010
 * author:      Filonenko Michael
 *
 *
 * Copyright 2010 Filonenko Michael
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

#ifndef LOG4QT_COLORCONSOLEAPPENDER_H
#define LOG4QT_COLORCONSOLEAPPENDER_H

#include "consoleappender.h"

// if we are in WIN*
#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#endif

class QFile;
class QTextStream;

namespace Log4Qt
{

/*!
 * \brief The class ColorConsoleAppender appends to stdout or stderr with color formatting.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT ColorConsoleAppender : public ConsoleAppender
{
    Q_OBJECT

public:
    ColorConsoleAppender(QObject *parent = nullptr);
    ColorConsoleAppender(const LayoutSharedPtr &layout,
                         QObject *parent = nullptr);
    ColorConsoleAppender(const LayoutSharedPtr &layout,
                         const QString &target,
                         QObject *parent = nullptr);
    /*!
     * Creates a ConsoleAppender with the layout \a pLayout, the target
     * value specified by the \a target constant and the parent
     * \a parent.
     */
    ColorConsoleAppender(const LayoutSharedPtr &layout,
                         Target target,
                         QObject *parent = nullptr);

    ~ColorConsoleAppender() override;
    // if we are in WIN*
#ifdef Q_OS_WIN
    void activateOptions() override;
    void close() override;

protected:
    void append(const LoggingEvent &event) override;

private:
    HANDLE hConsole;
#endif
    void closeInternal();
};


} // namespace Log4Qt


#endif // #ifndef  LOG4QT_COLORCONSOLEAPPENDER_H
