/******************************************************************************
 *
 * package:         log4qt
 * file:        colorconsoleappender.cpp
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

#include "colorconsoleappender.h"

#include "loggingevent.h"
#include "layout.h"

#include <QTextStream>

#define NIX_BACK_BLACK      40
#define NIX_BACK_RED            41
#define NIX_BACK_GREEN      42
#define NIX_BACK_YELLOW     43
#define NIX_BACK_BLUE           44
#define NIX_BACK_MAGNETTA   45
#define NIX_BACK_CYAN           46
#define NIX_BACK_GRAY           47

#define NIX_FORE_BLACK      30
#define NIX_FORE_RED            31
#define NIX_FORE_GREEN      32
#define NIX_FORE_YELLOW     33
#define NIX_FORE_BLUE           34
#define NIX_FORE_MAGNETTA   35
#define NIX_FORE_CYAN           36
#define NIX_FORE_GRAY           37

#define NIX_FORE_BOLD           1

#define NIX_DEFAULT             0

#ifdef Q_OS_WIN
#define WIN_BACK_BLACK                      0
#define WIN_BACK_RED                            BACKGROUND_RED
#define WIN_BACK_LIGHT_RED              BACKGROUND_RED | BACKGROUND_INTENSITY
#define WIN_BACK_GREEN                      BACKGROUND_GREEN
#define WIN_BACK_LIGHT_GREEN            BACKGROUND_GREEN | BACKGROUND_INTENSITY
#define WIN_BACK_YELLOW                     BACKGROUND_GREEN | BACKGROUND_RED
#define WIN_BACK_LIGHT_YELLOW           BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY
#define WIN_BACK_BLUE                           BACKGROUND_BLUE
#define WIN_BACK_LIGHT_BLUE             BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define WIN_BACK_MAGNETTA                   BACKGROUND_RED | BACKGROUND_BLUE
#define WIN_BACK_LIGHT_MAGNETTA     BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY
#define WIN_BACK_CYAN                           BACKGROUND_BLUE | BACKGROUND_GREEN
#define WIN_BACK_LIGHT_CYAN             BACKGROUND_BLUE | BACKGROUND_GREEN
#define WIN_BACK_GRAY                           BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED
#define WIN_BACK_WHITE                      BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY

#define WIN_FORE_BLACK                      0
#define WIN_FORE_RED                            FOREGROUND_RED
#define WIN_FORE_LIGHT_RED              FOREGROUND_RED | FOREGROUND_INTENSITY
#define WIN_FORE_GREEN                      FOREGROUND_GREEN
#define WIN_FORE_LIGHT_GREEN            FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define WIN_FORE_YELLOW                     FOREGROUND_GREEN | FOREGROUND_RED
#define WIN_FORE_LIGHT_YELLOW           FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define WIN_FORE_BLUE                           FOREGROUND_BLUE
#define WIN_FORE_LIGHT_BLUE             FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define WIN_FORE_MAGNETTA                   FOREGROUND_RED | FOREGROUND_BLUE
#define WIN_FORE_LIGHT_MAGNETTA     FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define WIN_FORE_CYAN                           FOREGROUND_BLUE | FOREGROUND_GREEN
#define WIN_FORE_LIGHT_CYAN             FOREGROUND_BLUE | FOREGROUND_GREEN
#define WIN_FORE_GRAY                           FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
#define WIN_FORE_WHITE                      FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY

#define WIN_FORE_BOLD                           FOREGROUND_INTENSITY

#define WIN_DEFAULT                             FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

static void colorOutputString(HANDLE hConsole, const QString &output)
{
    QString message = output;

    // save colors
    CONSOLE_SCREEN_BUFFER_INFO cbi;
    GetConsoleScreenBufferInfo(hConsole, &cbi);

    wchar_t *wideMessage;

    QStringList colorizedMessage = message.split('\033');

    int actualSize;
    DWORD out;

    WORD color = 0;
    WORD newColor = 0;
    QString parsedWordString;
    QStringList escParams;
    int indexOfM;
    // display first part of message
    if (!colorizedMessage.at(0).isEmpty())
    {
        wideMessage = new wchar_t [colorizedMessage.at(0).size()];
        actualSize = colorizedMessage.at(0).toWCharArray(wideMessage);
        WriteConsoleW(hConsole, wideMessage, actualSize, &out, nullptr);
        delete [] wideMessage;
        colorizedMessage.removeAt(0);
    }
    for (QString it : colorizedMessage)
    {
        // color setted
        if (it.startsWith('['))
        {
            indexOfM = it.indexOf('m');
            // not esc-sequence
            if (indexOfM != -1)
            {
                parsedWordString = it.mid(1, indexOfM - 1);

                escParams = parsedWordString.split(';');
                for (const auto &param : qAsConst(escParams))
                {
                    color = param.toUInt();
                    switch (color)
                    {
                    case NIX_DEFAULT:
                        newColor = WIN_DEFAULT;
                        break;
                    case NIX_FORE_BOLD:
                        newColor |= WIN_FORE_BOLD;
                        break;
                    case NIX_BACK_BLACK :
                        newColor = (newColor & 0x0f) | WIN_BACK_BLACK;
                        break;
                    case NIX_BACK_RED :
                        newColor = (newColor & 0x0f) | WIN_BACK_RED;
                        break;
                    case NIX_BACK_GREEN :
                        newColor = (newColor & 0x0f) | WIN_BACK_GREEN;
                        break;
                    case NIX_BACK_YELLOW :
                        newColor = (newColor & 0x0f) | WIN_BACK_YELLOW;
                        break;
                    case NIX_BACK_BLUE :
                        newColor = (newColor & 0x0f) | WIN_BACK_BLUE;
                        break;
                    case NIX_BACK_MAGNETTA :
                        newColor = (newColor & 0x0f) | WIN_BACK_MAGNETTA;
                        break;
                    case NIX_BACK_CYAN :
                        newColor = (newColor & 0x0f) | WIN_BACK_CYAN;
                        break;
                    case NIX_BACK_GRAY :
                        newColor = (newColor & 0x0f) | WIN_BACK_GRAY;
                        break;
                    case NIX_FORE_BLACK :
                        newColor = (newColor & 0xF8) | WIN_FORE_BLACK;
                        break;
                    case NIX_FORE_RED :
                        newColor = (newColor & 0xF8) | WIN_FORE_RED;
                        break;
                    case NIX_FORE_GREEN :
                        newColor = (newColor & 0xF8) | WIN_FORE_GREEN;
                        break;
                    case NIX_FORE_YELLOW :
                        newColor = (newColor & 0xF8) | WIN_FORE_YELLOW;
                        break;
                    case NIX_FORE_BLUE :
                        newColor = (newColor & 0xF8) | WIN_FORE_BLUE;
                        break;
                    case NIX_FORE_MAGNETTA :
                        newColor = (newColor & 0xF8) | WIN_FORE_MAGNETTA;
                        break;
                    case NIX_FORE_CYAN :
                        newColor = (newColor & 0xF8) | WIN_FORE_CYAN;
                        break;
                    case NIX_FORE_GRAY :
                        newColor = (newColor & 0xF8) | WIN_FORE_GRAY;
                        break;
                    default:
                        break;
                    }
                }
                it = it.mid(indexOfM + 1);

                SetConsoleTextAttribute(hConsole, newColor);
            }
        }

        wideMessage = new wchar_t [it.size()];
        actualSize = it.toWCharArray(wideMessage);
        WriteConsoleW(hConsole, wideMessage, actualSize, &out, nullptr);
        delete [] wideMessage;
    }
    // load old colors
    SetConsoleTextAttribute(hConsole, cbi.wAttributes);
}
#endif

namespace Log4Qt
{

ColorConsoleAppender::ColorConsoleAppender(QObject *parent) :
    ConsoleAppender(parent),
    hConsole(nullptr)
{
}

ColorConsoleAppender::ColorConsoleAppender(const LayoutSharedPtr &layout, QObject *parent) :
    ConsoleAppender(layout, parent),
    hConsole(nullptr)
{
}

ColorConsoleAppender::ColorConsoleAppender(const LayoutSharedPtr &layout,
        const QString &target, QObject *parent) :
    ConsoleAppender(layout, target, parent),
    hConsole(nullptr)
{
}

ColorConsoleAppender::ColorConsoleAppender(const LayoutSharedPtr &layout, Target target,
        QObject *parent) :
    ConsoleAppender(layout, target, parent),
    hConsole(nullptr)
{
}

ColorConsoleAppender::~ColorConsoleAppender()
{
    closeInternal();
}

#ifdef Q_OS_WIN
void ColorConsoleAppender::append(const LoggingEvent &event)
{
    QString message = layout()->format(event);

    colorOutputString(hConsole, message);

    if (handleIoErrors())
        return;

    if (immediateFlush())
    {
        writer()->flush();
        if (handleIoErrors())
            return;
    }
}

void ColorConsoleAppender::activateOptions()
{
    ConsoleAppender::activateOptions();

    if (target() == QStringLiteral("STDOUT_TARGET"))
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    else
        hConsole = GetStdHandle(STD_ERROR_HANDLE);
}

void ColorConsoleAppender::close()
{
    closeInternal();
    ConsoleAppender::close();
}
#endif

void ColorConsoleAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

#ifdef Q_OS_WIN
    CloseHandle(hConsole);
#endif
}

} // namespace Log4Qt

#include "moc_colorconsoleappender.cpp"

