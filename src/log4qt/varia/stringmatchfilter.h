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

#ifndef LOG4QT_STRINGMATCHFILTER_H
#define LOG4QT_STRINGMATCHFILTER_H

#include "log4qt/spi/filter.h"

namespace Log4Qt
{

/*!
 * \brief The class StringMatchFilter allows logging events with a
 *        specified level.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See  \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT StringMatchFilter : public Filter
{
    Q_OBJECT

    /*!
     * The property holds if an event is accpeted on a match.
     *
     * The default is true.
     *
     * \sa acceptOnMatch(), acceptOnMatch()
     */
    Q_PROPERTY(bool acceptOnMatch READ acceptOnMatch WRITE setAcceptOnMatch)

    /*!
     * The property holds the string to match for this filter.
     *
     * \sa stringToMatch(), setStringToMatch()
     */
    Q_PROPERTY(QString stringToMatch READ stringToMatch WRITE setStringToMatch)

    /*!
     * The property holds the case sensitivity used to match the string.
     *
     * The default is Qt::CaseSensitive.
     *
     * \sa caseSensitivity(), setCaseSensitivity()
     */
    Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity)

public:
    StringMatchFilter(QObject *parent = nullptr);

    [[nodiscard]] bool acceptOnMatch() const { return mAcceptOnMatch; }
    [[nodiscard]] QString stringToMatch() const { return mStringToMatch; }
    [[nodiscard]] Qt::CaseSensitivity caseSensitivity() const { return mCaseSensitivity; }
    void setAcceptOnMatch(bool accept) { mAcceptOnMatch = accept; }
    void setStringToMatch(const QString &string) { mStringToMatch = string; }
    void setStringToMatch(const QString &string, Qt::CaseSensitivity cs)
    {
        mStringToMatch = string;
        mCaseSensitivity = cs;
    }
    void setCaseSensitivity(Qt::CaseSensitivity cs) { mCaseSensitivity = cs; }

    Decision decide(const LoggingEvent &event) const override;

private:
    bool mAcceptOnMatch = true;
    QString mStringToMatch;
    // Default is case-sensitive: pre-D-040 the single-argument setter defaulted
    // its Qt::CaseSensitivity argument to Qt::CaseSensitive, so the single-arg
    // setStringToMatch() must keep matching case-sensitively for backward
    // compatibility (the dedicated two-arg overload / property opt into
    // case-insensitive matching).
    Qt::CaseSensitivity mCaseSensitivity = Qt::CaseSensitive;
};

} // namespace Log4Qt

#endif // LOG4QT_STRINGMATCHFILTER_H
