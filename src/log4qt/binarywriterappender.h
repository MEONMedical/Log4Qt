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

#ifndef LOG4QT_BINARYWRITTERAPPENDER_H
#define LOG4QT_BINARYWRITTERAPPENDER_H

#include "appenderskeleton.h"

class QDataStream;
namespace Log4Qt
{

class BinaryLayout;

class LOG4QT_EXPORT BinaryWriterAppender : public AppenderSkeleton
{
    Q_OBJECT
    Q_PROPERTY(QDataStream *writer READ writer WRITE setWriter)
public:
    BinaryWriterAppender(QObject *parent = nullptr);
    BinaryWriterAppender(QDataStream *dataStream, QObject *parent = nullptr);
    ~BinaryWriterAppender() override;

    bool requiresLayout() const override;
    QDataStream *writer() const;

    void setWriter(QDataStream *dataStream);

    void activateOptions() override;
    void close() override;

protected:
    void append(const LoggingEvent &event) override;
    bool checkEntryConditions() const override;

    void closeWriter();

    virtual bool handleIoErrors() const;
    void writeFooter() const;
    void writeHeader() const;

private:
    Q_DISABLE_COPY_MOVE(BinaryWriterAppender)
    void writeRawData(const QByteArray &data) const;
    void closeInternal();
    BinaryLayout *binaryLayout() const;

    QDataStream *mWriter;
};

inline QDataStream *BinaryWriterAppender::writer() const
{
    return mWriter;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYWRITTERAPPENDER_H
