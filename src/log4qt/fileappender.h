/******************************************************************************
 *
 * package:
 * file:        fileappender.h
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

#ifndef LOG4QT_FILEAPPENDER_H
#define LOG4QT_FILEAPPENDER_H

#include "writerappender.h"

class QFile;
class QTextStream;

namespace Log4Qt
{

/*!
 * \brief The class FileAppender appends log events to a file.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed. See
 *       \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT  FileAppender : public WriterAppender
{
    Q_OBJECT

    /*!
     * The property holds, if the output is appended to the file.
     *
     * The default is false for not appending.
     *
     * \sa appendFile(), setAppendFile()
     */
    Q_PROPERTY(bool appendFile READ appendFile WRITE setAppendFile)

    /*!
     * The property holds, if the output is buffered.
     *
     * The default is true for buffering.
     *
     * \sa bufferedIo(), setBufferedIo()
     */
    Q_PROPERTY(bool bufferedIo READ bufferedIo WRITE setBufferedIo)

    /*!
     * The property holds the name of the file.
     *
     * \sa file(), setFile()
     */
    Q_PROPERTY(QString file READ file WRITE setFile)

public:
    explicit FileAppender(QObject *parent = nullptr);
    FileAppender(const LayoutSharedPtr &layout,
                 const QString &fileName,
                 QObject *parent = nullptr);
    FileAppender(const LayoutSharedPtr &layout,
                 const QString &fileName,
                 bool append,
                 QObject *parent = nullptr);
    FileAppender(const LayoutSharedPtr &layout,
                 const QString &fileName,
                 bool append,
                 bool buffered,
                 QObject *parent = nullptr);
    ~FileAppender() override;

private:
    Q_DISABLE_COPY(FileAppender)

public:
    bool appendFile() const;
    QString file() const;
    bool bufferedIo() const;
    void setAppendFile(bool append);
    void setBufferedIo(bool buffered);
    void setFile(const QString &fileName);

    void activateOptions() override;
    void close() override;

protected:
    /*!
     * Tests if all entry conditions for using append() in this class are met.
     *
     * If a conditions is not met, an error is logged and the function returns
     * false. Otherwise the result of WriterAppender::checkEntryConditions()
     * is returned.
     *
     * The checked conditions are:
     * - That a file is set and open (APPENDER_NO_OPEN_FILE_ERROR)
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by AppenderSkeleton::doAppend().
     *
     * \sa AppenderSkeleton::doAppend(), AppenderSkeleton::checkEntryConditions()
     */
    bool checkEntryConditions() const override;

    void closeFile();

    /*!
     * Checks for file I/O errrors. If an error is found it is logged and the
     * function returns true. Otherwise false is returned.
     */
    bool handleIoErrors() const override;

    /*!
     * Opens the file for the appender based on the specified file name and
     * mode. A text stream is created and passed on to the super class
     * WriterAppender.
     *
     * If the parent directory of the specified file does not exists,
     * it is created.
     */
    virtual void openFile();

    /*!
     * Removes the file \a file. If the operation is successful, true is
     * returned. Otherwise an APPENDER_REMOVE_FILE_ERROR error is logged
     * and false is returned.
     */
    bool removeFile(QFile &file) const;

    /*!
     * Renames the file \a file to \a fileName. If the operation is
     * successful, true is returned. Otherwise an
     * APPENDER_RENAMING_FILE_ERROR error is logged and false is returned.
     */
    bool renameFile(QFile &file,
                    const QString &fileName) const;

private:
    volatile bool mAppendFile;
    volatile bool mBufferedIo;
    QString mFileName;
    QFile *mFile;
    QTextStream *mTextStream;
    void closeInternal();
};

inline bool FileAppender::appendFile() const
{
    return mAppendFile;
}

inline QString FileAppender::file() const
{
    QMutexLocker locker(&mObjectGuard);
    return mFileName;
}

inline bool FileAppender::bufferedIo() const
{
    return mBufferedIo;
}

inline void FileAppender::setAppendFile(bool append)
{
    mAppendFile = append;
}

inline void FileAppender::setBufferedIo(bool buffered)
{
    mBufferedIo = buffered;
}

inline void FileAppender::setFile(const QString &fileName)
{
    QMutexLocker locker(&mObjectGuard);
    mFileName = fileName;
}


} // namespace Log4Qt

#endif // LOG4QT_FILEAPPENDER_H
