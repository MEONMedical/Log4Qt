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

#ifndef LOG4QT_PATTERNLAYOUT_H
#define LOG4QT_PATTERNLAYOUT_H

#include "abstractstringlayout.h"
#include "helpers/patternformatter.h"

#include <memory>

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
class LOG4QT_EXPORT PatternLayout : public AbstractStringLayout
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

    /*!
     * The property holds an optional conversion pattern for the layout header.
     *
     * When set, the header is formatted using the pattern at the time the
     * file is opened. Useful conversions are \c %d (current date/time) and
     * \c %r (milliseconds since start). Overrides the plain \c header
     * property when both are set.
     *
     * \sa headerPattern(), setHeaderPattern()
     */
    Q_PROPERTY(QString headerPattern READ headerPattern WRITE setHeaderPattern)

    /*!
     * The property holds an optional conversion pattern for the layout footer.
     *
     * Symmetric to \c headerPattern; formatted at the time the file is closed.
     *
     * \sa footerPattern(), setFooterPattern()
     */
    Q_PROPERTY(QString footerPattern READ footerPattern WRITE setFooterPattern)

public:
    /*!
     * The enum ConversionPattern defines constants for pattern strings.
     *
     * \sa setConversionPattern(ConversionPattern);
     */
    enum ConversionPattern : int
    {
        /*! The default conversion pattern string is "%m,%n". */
        DefaultPattern,
        /*!
         * The ttcc conversion pattern string is
         * "%r [%t] %p %c %x - %m%n".
         */
        TtccPattern,
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
    Q_DISABLE_COPY_MOVE(PatternLayout)

public:
    [[nodiscard]] QString conversionPattern() const { return mPattern; }
    void setConversionPattern(const QString &pattern)
    {
        mPattern = pattern;
        updatePatternFormatter();
    }

    /*!
     * Sets the conversion pattern to the value specified by the
     * \a conversionPattern constant.
     */
    void setConversionPattern(ConversionPattern conversionPattern);

    [[nodiscard]] QString headerPattern() const { return mHeaderPattern; }
    void setHeaderPattern(const QString &pattern);
    [[nodiscard]] QString footerPattern() const { return mFooterPattern; }
    void setFooterPattern(const QString &pattern);

    [[nodiscard]] QString header() const override;
    [[nodiscard]] QString footer() const override;

    [[nodiscard]] QString format(const LoggingEvent &event) override;

    /*!
     * Returns true if the current pattern contains at least one
     * location-sensitive conversion character (\c %F, \c %L, \c %M,
     * \c %l).
     */
    [[nodiscard]] bool requiresLocation() const override;

private:
    void updatePatternFormatter();

private:
    QString mPattern;
    std::unique_ptr<PatternFormatter> mpPatternFormatter;
    QString mHeaderPattern;
    QString mFooterPattern;
    std::unique_ptr<PatternFormatter> mHeaderFormatter;
    std::unique_ptr<PatternFormatter> mFooterFormatter;
};

} // namespace Log4Qt

#endif // LOG4QT_PATTERNLAYOUT_H
