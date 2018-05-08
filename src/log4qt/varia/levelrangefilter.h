/******************************************************************************
 *
 * package:     Log4Qt
 * file:        levelrangefilter.h
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

#ifndef LOG4QT_LEVELRANGEFILTER_H
#define LOG4QT_LEVELRANGEFILTER_H

#include <log4qt/spi/filter.h>
#include <log4qt/level.h>

namespace Log4Qt
{

/*!
 * \brief The class LevelMatchFilter allows logging events with levels in a
 *        specified range.
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class  LOG4QT_EXPORT LevelRangeFilter : public Filter
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
     * The property holds the maximum level of the range for this filter.
     *
     * The default is Level::OFF_INT.
     *
     * \sa levelMax(), setLevelMax()
     */
    Q_PROPERTY(Log4Qt::Level levelMax READ levelMax WRITE setLevelMax)

    /*!
         * The property holds the minimum level of the range for this filter.
         *
         * The default is Level::NULL_INT.
         *
         * \sa levelMin(), setLevelMin()
         */
    Q_PROPERTY(Log4Qt::Level levelMin READ levelMin WRITE setLevelMin)

public:
    LevelRangeFilter(QObject *parent = nullptr);

    bool acceptOnMatch() const;
    Level levelMax() const;
    Level levelMin() const;
    void setAcceptOnMatch(bool accept);
    void setLevelMax(Level level);
    void setLevelMin(Level level);

    Decision decide(const LoggingEvent &event) const override;

private:
    bool mAcceptOnMatch;
    Level mLevelMin;
    Level mLevelMax;
};

inline bool LevelRangeFilter::acceptOnMatch() const
{
    return mAcceptOnMatch;
}

inline Level LevelRangeFilter::levelMax() const
{
    return mLevelMax;
}

inline Level LevelRangeFilter::levelMin() const
{
    return mLevelMin;
}

inline void LevelRangeFilter::setAcceptOnMatch(bool accept)
{
    mAcceptOnMatch = accept;
}

inline void LevelRangeFilter::setLevelMax(Level level)
{
    mLevelMax = level;
}

inline void LevelRangeFilter::setLevelMin(Level level)
{
    mLevelMin = level;
}

} // namespace Log4Qt

#endif // LOG4QT_LEVELRANGEFILTER_H
