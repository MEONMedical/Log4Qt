/******************************************************************************
 *
 * package:
 * file:        consoleappender.cpp
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

#include "consoleappender.h"

#include "helpers/optionconverter.h"
#include "layout.h"
#include "loggingevent.h"

#include <QTextStream>

namespace Log4Qt
{

ConsoleAppender::ConsoleAppender(QObject *pParent) :
    WriterAppender(pParent),
    mTarget(STDOUT_TARGET),
    mpTextStream(Q_NULLPTR)
{
}


ConsoleAppender::ConsoleAppender(LayoutSharedPtr pLayout,
                                 QObject *pParent) :
    WriterAppender(pLayout, pParent),
    mTarget(STDOUT_TARGET),
    mpTextStream(Q_NULLPTR)
{
}


ConsoleAppender::ConsoleAppender(LayoutSharedPtr pLayout,
                                 const QString &rTarget,
                                 QObject *pParent) :
    WriterAppender(pLayout, pParent),
    mTarget(STDOUT_TARGET),
    mpTextStream(Q_NULLPTR)
{
    setTarget(rTarget);
}


ConsoleAppender::ConsoleAppender(LayoutSharedPtr pLayout,
                                 Target target,
                                 QObject *pParent) :
    WriterAppender(pLayout, pParent),
    mTarget(target),
    mpTextStream(Q_NULLPTR)
{
}


ConsoleAppender::~ConsoleAppender()
{
    close();

}

QString ConsoleAppender::target() const
{
    if (mTarget == STDOUT_TARGET)
        return QLatin1String("STDOUT_TARGET");
    else
        return QLatin1String("STDERR_TARGET");
}

void ConsoleAppender::setTarget(const QString &rTarget)
{
    bool ok;
    Target target = static_cast<Target>(OptionConverter::toTarget(rTarget, &ok));
    if (ok)
        setTarget(target);
}


void ConsoleAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    closeStream();

    if (mTarget == STDOUT_TARGET)
        mpTextStream = new QTextStream(stdout);
    else
        mpTextStream = new QTextStream(stderr);
    setWriter(mpTextStream);

    WriterAppender::activateOptions();
}


void ConsoleAppender::close()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    WriterAppender::close();
    closeStream();
}

void ConsoleAppender::closeStream()
{
    setWriter(Q_NULLPTR);
    delete mpTextStream;
    mpTextStream = Q_NULLPTR;
}

} // namespace Log4Qt
