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

#include "consoleappender.h"

#include "helpers/optionconverter.h"
#include "layout.h"
#include "loggingevent.h"

#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Log4Qt
{

ConsoleAppender::ConsoleAppender(QObject *parent) :
    WriterAppender(parent),
    mTarget(STDOUT_TARGET),
    mtextStream(nullptr)
{
}


ConsoleAppender::ConsoleAppender(const LayoutSharedPtr &pLayout,
                                 QObject *parent) :
    WriterAppender(pLayout, parent),
    mTarget(STDOUT_TARGET),
    mtextStream(nullptr)
{
}


ConsoleAppender::ConsoleAppender(const LayoutSharedPtr &pLayout,
                                 const QString &target,
                                 QObject *parent) :
    WriterAppender(pLayout, parent),
    mTarget(STDOUT_TARGET),
    mtextStream(nullptr)
{
    setTarget(target);
}


ConsoleAppender::ConsoleAppender(const LayoutSharedPtr &pLayout,
                                 Target target,
                                 QObject *parent) :
    WriterAppender(pLayout, parent),
    mTarget(target),
    mtextStream(nullptr)
{
}


ConsoleAppender::~ConsoleAppender()
{
    closeInternal();
}

QString ConsoleAppender::target() const
{
    if (mTarget == STDOUT_TARGET)
        return QStringLiteral("STDOUT_TARGET");
    return QStringLiteral("STDERR_TARGET");
}

void ConsoleAppender::setTarget(const QString &target)
{
    bool ok;
    auto targetEnum = static_cast<Target>(OptionConverter::toTarget(target, &ok));
    if (ok)
        setTarget(targetEnum);
}

void ConsoleAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    closeStream();

    if (mTarget == STDOUT_TARGET)
        mtextStream = new QTextStream(stdout);
    else
        mtextStream = new QTextStream(stderr);
    setWriter(mtextStream);

    WriterAppender::activateOptions();
}

void ConsoleAppender::close()
{
    closeInternal();
    WriterAppender::close();
}

void ConsoleAppender::closeInternal()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    closeStream();
}

void ConsoleAppender::closeStream()
{
    setWriter(nullptr);
    delete mtextStream;
    mtextStream = nullptr;
}

void ConsoleAppender::append(const LoggingEvent &event)
{
#ifdef Q_OS_WIN
    if (!GetConsoleWindow())
    {
        // if console is blocked by debugger use OutputDebugString
        Q_ASSERT_X(layout(), "ConsoleAppender::append()", "Layout must not be null");

        QString message(layout()->format(event));

        OutputDebugString(message.toStdWString().c_str());
    }
    else
#endif
        WriterAppender::append(event);
}

} // namespace Log4Qt

#include "moc_consoleappender.cpp"
