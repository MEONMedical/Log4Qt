/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
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

#ifndef LOG4QT_CONSOLEAPPENDER_H
#define LOG4QT_CONSOLEAPPENDER_H

#include "writerappender.h"

#include <memory>

class QTextStream;

namespace Log4Qt
{

/*!
 * \brief The class ConsoleAppender appends to stdout or stderr.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT ConsoleAppender : public WriterAppender
{
    Q_OBJECT

    /*!
     * The property holds the target used by the appender.
     *
     * The default is StdOut for the standard output.
     *
     * \sa Target, target(), setTarget()
     */
    Q_PROPERTY(QString target READ target WRITE setTarget)

public:
    /*!
     * The enum defines the possible output targets
     *
     * \sa target(), setTarget()
     */
    enum Target : int
    {
        /*! The output target is standard out. */
        StdOut,
        /*! The output target is standard error. */
        StdErr,
    };
    Q_ENUM(Target)


    ConsoleAppender(QObject *parent = nullptr);
    ConsoleAppender(const LayoutSharedPtr &pLayout,
                    QObject *parent = nullptr);
    ConsoleAppender(const LayoutSharedPtr &pLayout,
                    const QString &target,
                    QObject *parent = nullptr);

    /*!
     * Creates a ConsoleAppender with the layout \a pLayout, the target
     * value specified by the \a target constant and the parent
     * \a parent.
     */
    ConsoleAppender(const LayoutSharedPtr &pLayout,
                    Target target,
                    QObject *parent = nullptr);

    ~ConsoleAppender() override;
private:
    Q_DISABLE_COPY_MOVE(ConsoleAppender)

public:
    QString target() const;
    void setTarget(const QString &target);

    /*!
     * Sets the target to the value specified by the \a target constant.
     */
    void setTarget(Target target) { mTarget = target; }

    virtual void activateOptions() override;
    virtual void close() override;

protected:
    void closeStream();
    void append(const LoggingEvent &event) override;

private:
    std::atomic<Target> mTarget;
    std::unique_ptr<QTextStream> mtextStream;
#ifdef Q_OS_WIN
    bool mUseOutputDebugString = false;
#endif
    void closeInternal();
};

} // namespace Log4Qt

#endif // _CONSOLEAPPENDER_H
