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

#include <QHash>
#include <QPair>
#include <QRegExp>

// if we are in WIN*
#if defined(__WIN32__) || defined(WIN) || defined(WIN32) || defined(Q_OS_WIN32)
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

    ColorConsoleAppender(QObject *pParent = Q_NULLPTR);
    ColorConsoleAppender(LayoutSharedPtr pLayout,
                         QObject *pParent = Q_NULLPTR);
    ColorConsoleAppender(LayoutSharedPtr pLayout,
                         const QString &rTarget,
                         QObject *pParent = Q_NULLPTR);

    /*!
     * Creates a ConsoleAppender with the layout \a pLayout, the target
     * value specified by the \a target constant and the parent
     * \a pParent.
     */
    ColorConsoleAppender(LayoutSharedPtr pLayout,
                         Target target,
                         QObject *pParent = Q_NULLPTR);
    // if we are in WIN*
#if defined(__WIN32__) || defined(WIN) || defined(WIN32) || defined(Q_OS_WIN32)

    virtual void activateOptions() Q_DECL_OVERRIDE;

    virtual void close() Q_DECL_OVERRIDE;
protected:
    virtual void append(const LoggingEvent &rEvent) Q_DECL_OVERRIDE;
private:
    HANDLE hConsole;
#endif

};


} // namespace Log4Qt


#endif // #ifndef  LOG4QT_COLORCONSOLEAPPENDER_H
