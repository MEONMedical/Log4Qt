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

#ifndef LOG4QT_BINARYLAYOUT_H
#define LOG4QT_BINARYLAYOUT_H

#include "layout.h"

#include <QByteArray>

namespace Log4Qt
{

class LoggingEvent;
class BinaryLoggingEvent;

class LOG4QT_EXPORT BinaryLayout : public Layout
{
    Q_OBJECT
    Q_PROPERTY(QByteArray binaryFooter READ binaryFooter WRITE setBinaryFooter)
    Q_PROPERTY(QByteArray binaryHeader READ binaryHeader WRITE setBinaryHeader)

public:
    explicit BinaryLayout(QObject *parent = nullptr);

    virtual QByteArray binaryFormat(const BinaryLoggingEvent &event) const;
    virtual QString format(const LoggingEvent &event) override;

    virtual QString contentType() const override;

    virtual QByteArray binaryFooter() const;
    void setBinaryFooter(const QByteArray &footer);

    virtual QByteArray binaryHeader() const;
    void setBinaryHeader(const QByteArray &header);

private:
    Q_DISABLE_COPY_MOVE(BinaryLayout)

    QByteArray mFooter;
    QByteArray mHeader;
};

inline QByteArray BinaryLayout::binaryFooter() const
{
    return mFooter;
}

inline void BinaryLayout::setBinaryFooter(const QByteArray &footer)
{
    mFooter = footer;
}

inline QByteArray BinaryLayout::binaryHeader() const
{
    return mHeader;
}

inline void BinaryLayout::setBinaryHeader(const QByteArray &header)
{
    mHeader = header;
}

} // namespace Log4Qt

#endif // LOG4QT_BINARYLAYOUT_H
