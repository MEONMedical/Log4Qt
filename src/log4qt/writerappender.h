/******************************************************************************
 *
 * package:     Log4Qt
 * file:        writerappender.h
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

#ifndef LOG4QT_WRITERAPPENDER_H
#define LOG4QT_WRITERAPPENDER_H

#include "appenderskeleton.h"

class QTextCodec;
class QTextStream;

namespace Log4Qt
{

/*!
 * \brief The class WriterAppender appends log events to a QTextStream.
 *
 * \note All the functions declared in this class are thread-safe.
 * &nbsp;
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT  WriterAppender : public AppenderSkeleton
{
    Q_OBJECT

    /*!
     * The property holds the codec the appender uses.
     *
     * The default is null to use the codec the writer has set.
     *
     * \sa encoding(), setEncoding()
     */
    Q_PROPERTY(QTextCodec *encoding READ encoding WRITE setEncoding)

    /*!
     * The property holds the writer the appender uses.
     *
     * \sa writer(), setWriter()
     */
    Q_PROPERTY(QTextStream *writer READ writer WRITE setWriter)

    /*!
     * The property holds, if the writer flushes after all write operations.
     *
     * The default is true for flushing.
     *
     * \sa immediateFlush(), setImmediateFlush()
     */
    Q_PROPERTY(bool immediateFlush READ immediateFlush WRITE setImmediateFlush)

public:
    WriterAppender(QObject *parent = nullptr);
    WriterAppender(const LayoutSharedPtr &layout,
                   QObject *parent = nullptr);
    WriterAppender(const LayoutSharedPtr &layout,
                   QTextStream *textStream,
                   QObject *parent = nullptr);
    ~WriterAppender() override;

private:
    Q_DISABLE_COPY(WriterAppender)

public:
    bool requiresLayout() const override;
    QTextCodec *encoding() const;
    bool immediateFlush() const;
    QTextStream *writer() const;

    /*!
     * Sets the codec used by the writer to \a pTextCoded.
     *
     * If a codec is set with setEncoding, it will overwrite the codec set
     * in the text stream. A subsequent call with \a pTextCoded equals null
     * will resets the codec to the default QTextCodec::codecForLocale().
     *
     * \sa encoding(), QTextSream::setCodec(), QTextCodec::codecForLocale()
     */
    void setEncoding(QTextCodec *encoding);
    void setImmediateFlush(bool immediateFlush);
    void setWriter(QTextStream *textStream);

    void activateOptions() override;
    void close() override;

protected:
    void append(const LoggingEvent &event) override;

    /*!
     * Tests if all entry conditions for using append() in this class are
     * met.
     *
     * If a conditions is not met, an error is logged and the function
     * returns false. Otherwise the result of
     * AppenderSkeleton::checkEntryConditions() is returned.
     *
     * The checked conditions are:
     * - A writer has been set (APPENDER_USE_MISSING_WRITER_ERROR)
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by AppenderSkeleton::doAppend().
     *
     * \sa AppenderSkeleton::doAppend(),
     *     AppenderSkeleton::checkEntryConditions()
     */
    bool checkEntryConditions() const override;

    void closeWriter();

    virtual bool handleIoErrors() const;
    void writeFooter() const;
    void writeHeader() const;

private:
    QTextCodec *mEncoding;
    QTextStream *mWriter;
    volatile bool mImmediateFlush;
    void closeInternal();
};

inline QTextCodec *WriterAppender::encoding() const
{
    QMutexLocker locker(&mObjectGuard);
    return mEncoding;
}

inline bool WriterAppender::immediateFlush() const
{
    return mImmediateFlush;
}

inline QTextStream *WriterAppender::writer() const
{
    return mWriter;
}

inline void WriterAppender::setImmediateFlush(bool immediateFlush)
{
    mImmediateFlush = immediateFlush;
}


} // namespace Log4Qt

#endif // LOG4QT_WRITERAPPENDER_H
