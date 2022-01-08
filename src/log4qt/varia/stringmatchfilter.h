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

public:
    StringMatchFilter(QObject *parent = nullptr);

    bool acceptOnMatch() const;
    QString stringToMatch() const;
    void setAcceptOnMatch(bool accept);
    void setStringToMatch(const QString &string, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    Decision decide(const LoggingEvent &event) const override;

private:
    bool mAcceptOnMatch;
    QString mStringToMatch;
    Qt::CaseSensitivity mCaseSensitivity;
};

inline bool StringMatchFilter::acceptOnMatch() const
{
    return mAcceptOnMatch;
}

inline QString StringMatchFilter::stringToMatch() const
{
    return mStringToMatch;
}

inline void StringMatchFilter::setAcceptOnMatch(bool accept)
{
    mAcceptOnMatch = accept;
}

inline void StringMatchFilter::setStringToMatch(const QString &string, Qt::CaseSensitivity cs)
{
    mStringToMatch = string;
    mCaseSensitivity = cs;
}

} // namespace Log4Qt

#endif // LOG4QT_STRINGMATCHFILTER_H
