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

#ifndef LOG4QT_SIZEBASEDTRIGGERINGPOLICY_H
#define LOG4QT_SIZEBASEDTRIGGERINGPOLICY_H

#include "triggeringpolicy.h"

namespace Log4Qt
{

/*!
 * \brief The class SizeBasedTriggeringPolicy triggers a rollover when the
 *        file size exceeds a configured maximum.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class LOG4QT_EXPORT SizeBasedTriggeringPolicy : public TriggeringPolicy
{
    Q_OBJECT

    /*!
     * The property holds the maximum file size.
     * The default is 10 MB (10 * 1024 * 1024).
     */
    Q_PROPERTY(qint64 maximumFileSize READ maximumFileSize WRITE setMaximumFileSize)

    /*!
     * The property sets the maximum file size from a string value
     * (e.g. "10MB", "500KB").
     */
    Q_PROPERTY(QString maxFileSize READ maxFileSize WRITE setMaxFileSize)

public:
    static constexpr qint64 defaultMaximumFileSize = 10LL * 1024 * 1024;

    explicit SizeBasedTriggeringPolicy(QObject *parent = nullptr);

    [[nodiscard]] qint64 maximumFileSize() const { return mMaximumFileSize; }
    void setMaximumFileSize(qint64 maximumFileSize);

    [[nodiscard]] QString maxFileSize() const { return QString::number(mMaximumFileSize); }
    void setMaxFileSize(const QString &maxFileSize);

    bool isTriggeringEvent(QIODevice *activeDevice,
                           const LoggingEvent &event) override;

private:
    Q_DISABLE_COPY_MOVE(SizeBasedTriggeringPolicy)
    qint64 mMaximumFileSize;
};

} // namespace Log4Qt

#endif // LOG4QT_SIZEBASEDTRIGGERINGPOLICY_H
