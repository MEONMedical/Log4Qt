/******************************************************************************
 *
 * package:     Log4Qt
 * file:        patternlayout.h
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

#ifndef LOG4QT_PATTERNLAYOUT_H
#define LOG4QT_PATTERNLAYOUT_H

#include "layout.h"
#include "helpers/patternformatter.h"

#include <QScopedPointer>

namespace Log4Qt
{

/*!
 * \brief The class PatternLayout outputs a logging event based on a
 *        pattern string.
        \li c{section_count} : logger name with optional parameter section_count. Section count from end of logger name, sections delimiter is "::";
        \li d{format_string} : date with optional parameters in "{}"-brackets which used by QDateTime::toString();
        \li m : message
        \li p : level name
        \li r : relative date/time to start application
        \li t : thread name
        \li x : ndc name
        \li X : mdc name
        \li F : file name
        \li M : method name
        \li L : line number
 *
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT PatternLayout : public Layout
{
    Q_OBJECT

    /*!
     * The property holds the conversion pattern used by the appender.
     *
     * The default is "%m%n".
     *
     * \sa conversionPattern(), setConversionPattern()
     */
    Q_PROPERTY(QString conversionPattern READ conversionPattern WRITE setConversionPattern)

public:
    /*!
     * The enum ConversionPattern defines constants for pattern strings.
     *
     * \sa setConversionPattern(ConversionPattern);
     */
    enum ConversionPattern
    {
        /*! The default conversion pattern string is "%m,%n". */
        DEFAULT_CONVERSION_PATTERN,
        /*!
         * The ttcc conversion pattern string is
         * "%r [%t] %p %c %x - %m%n".
         */
        TTCC_CONVERSION_PATTERN
    };
    Q_ENUM(ConversionPattern)

    PatternLayout(QObject *parent = nullptr);
    PatternLayout(const QString &pattern,
                  QObject *parent = nullptr);

    /*!
     * Creates a PatternLayout with the conversion pattern value specified
     * by the \a conversionPattern constant.
     */
    PatternLayout(ConversionPattern conversionPattern,
                  QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(PatternLayout)

public:
    QString conversionPattern() const;
    void setConversionPattern(const QString &pattern);

    /*!
     * Sets the conversion pattern to the value specified by the
     * \a conversionPattern constant.
     */
    void setConversionPattern(ConversionPattern conversionPattern);

    QString format(const LoggingEvent &event) override;

private:
    void updatePatternFormatter();

private:
    QString mPattern;
    QScopedPointer<PatternFormatter> mPatternFormatter;
};

inline QString PatternLayout::conversionPattern() const
{
    return PatternLayout::mPattern;
}

inline void PatternLayout::setConversionPattern(const QString &pattern)
{
    mPattern = pattern;
    updatePatternFormatter();
}

} // namespace Log4Qt

#endif // LOG4QT_PATTERNLAYOUT_H
