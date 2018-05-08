/******************************************************************************
 *
 * package:     Log4Qt
 * file:        rollingfileappender.h
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

#ifndef LOG4QT_ROLINGFILEAPPENDER_H
#define LOG4QT_ROLINGFILEAPPENDER_H

#include "fileappender.h"

namespace Log4Qt
{

/*!
 * \brief The class RollingFileAppender extends FileAppender to backup
 *        the log files when they reach a certain size.
 *        On application restart the existing log files are rolled
 *        if appendFile is set to false to avoid data loss.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class  LOG4QT_EXPORT RollingFileAppender : public FileAppender
{
    Q_OBJECT

    /*!
     * The property holds the maximum backup count used by the appender.
     *
     * The default is 1.
     *
     * \sa maxBackupIndex(), setMaxBackupIndex()
     */
    Q_PROPERTY(int maxBackupIndex READ maxBackupIndex WRITE setMaxBackupIndex)

    /*!
     * The property holds the maximum file size used by the appender.
     *
     * The default is 10 MB (10 * 1024 * 1024).
     *
     * \sa maximumFileSize(), setMaximumFileSize()
     */
    Q_PROPERTY(qint64 maximumFileSize READ maximumFileSize WRITE setMaximumFileSize)

    /*!
     * The property sets the maximum file size from a string value.
     *
     * \sa setMaxFileSize(), maximumFileSize()
     */
    Q_PROPERTY(QString maxFileSize READ maxFileSize WRITE setMaxFileSize)

public:
    RollingFileAppender(QObject *parent = nullptr);
    RollingFileAppender(const LayoutSharedPtr &layout,
                        const QString &fileName,
                        QObject *parent = nullptr);
    RollingFileAppender(const LayoutSharedPtr &layout,
                        const QString &fileName,
                        bool append,
                        QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(RollingFileAppender)

public:
    int maxBackupIndex() const;
    qint64 maximumFileSize() const;
    QString maxFileSize() const;
    void setMaxBackupIndex(int maxBackupIndex);
    void setMaximumFileSize(qint64 maximumFileSize);
    void setMaxFileSize(const QString &maxFileSize);

protected:
    void append(const LoggingEvent &event) override;
    void openFile() override;

private:
    void rollOver();

private:
    int mMaxBackupIndex;
    qint64 mMaximumFileSize;
};

inline int RollingFileAppender::maxBackupIndex() const
{
    QMutexLocker locker(&mObjectGuard);
    return mMaxBackupIndex;
}

inline qint64 RollingFileAppender::maximumFileSize() const
{
    QMutexLocker locker(&mObjectGuard);
    return mMaximumFileSize;
}

inline QString RollingFileAppender::maxFileSize() const
{
    QMutexLocker locker(&mObjectGuard);
    return QString::number(mMaximumFileSize);
}

inline void RollingFileAppender::setMaxBackupIndex(int maxBackupIndex)
{
    QMutexLocker locker(&mObjectGuard);
    mMaxBackupIndex = maxBackupIndex;
}

inline void RollingFileAppender::setMaximumFileSize(qint64 maximumFileSize)
{
    QMutexLocker locker(&mObjectGuard);
    mMaximumFileSize = maximumFileSize;
}

} // namespace Log4Qt

#endif // LOG4QT_ROLINGFILEAPPENDER_H
