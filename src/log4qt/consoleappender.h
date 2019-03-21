/******************************************************************************
 *
 * package:
 * file:        consoleappender.h
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

#ifndef LOG4QT_CONSOLEAPPENDER_H
#define LOG4QT_CONSOLEAPPENDER_H

#include "writerappender.h"

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
class LOG4QT_EXPORT  ConsoleAppender : public WriterAppender
{
    Q_OBJECT

    /*!
     * The property holds the target used by the appender.
     *
     * The default is STDOUT_TARGET for the standard output.
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
    enum Target
    {
        /*! The output target is standard out. */
        STDOUT_TARGET,
        /*! The output target is standard error. */
        STDERR_TARGET
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
    Q_DISABLE_COPY(ConsoleAppender)

public:
    QString target() const;
    void setTarget(const QString &target);

    /*!
     * Sets the target to the value specified by the \a target constant.
     */
    void setTarget(Target target);

    virtual void activateOptions() override;
    virtual void close() override;

protected:
    void closeStream();

private:
    volatile Target mTarget;
    QTextStream *mtextStream;
    void closeInternal();
};

inline void ConsoleAppender::setTarget(Target target)
{
    mTarget = target;
}

} // namespace Log4Qt

#endif // _CONSOLEAPPENDER_H
