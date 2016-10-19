/******************************************************************************
 *
 * package:     Log4Qt
 * file:        properties.h
 * created:     September
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

#ifndef LOG4QT_PROPERTIES_H
#define LOG4QT_PROPERTIES_H

#include "log4qtshared.h"

#include <QHash>
#include <QStringList>

class QIODevice;
class QSettings;

namespace Log4Qt
{
/*!
 * \brief The class Properties implements a JAVA property hash.
 */
class  LOG4QT_EXPORT Properties : public QHash<QString, QString>
{
public:
    Properties(Properties *pDefaultProperties = Q_NULLPTR);

public:
    Properties *defaultProperties() const;
    QString property(const QString &rKey) const;
    QString property(const QString &rKey,
                     const QString &rDefaultValue) const;
    void setDefaultProperties(Properties *pDefault);
    void setProperty(const QString &rKey,
                     const QString &rValue);

    void load(QIODevice *pDevice);

    /*!
     * Reads all child keys from the QSettings object \a rSettings and
     * inserts them into this object. The value is created using
     * QVariant::toString(). Types that do not support toString() are
     * resulting in an empty string.
     *
     * \code
     * QSettings settings;
     * settings.setValue("Package", "Full");
     * settings.setValue("Background", Qt::white);
     * settings.setValue("Support", true);
     * settings.setValue("Help/Language", "en_UK");
     *
     * Properties properties
     * properties.load(&settings)
     *
     * // properties (("Package", "Full"), ("Background", ""), ("Support", "true"))
     * \endcode
     */
    void load(const QSettings &rSettings);

    QStringList propertyNames() const;

private:
    void parseProperty(const QString &rProperty,
                       int line);
    static int hexDigitValue(const QChar &rDigit);
    static QString trimLeft(const QString &rString);

private:
    Properties *mpDefaultProperties;
    static const char msEscapeChar;
    static const char *msValueEscapeCodes;
    static const char *msValueEscapeChars;
    static const char *msKeyEscapeCodes;
    static const char *msKeyEscapeChars;
};

inline Properties::Properties(Properties *pDefaultProperties) :
    mpDefaultProperties(pDefaultProperties)
{}

inline Properties *Properties::defaultProperties() const
{
    return mpDefaultProperties;
}

inline void Properties::setDefaultProperties(Properties *pDefaultProperties)
{
    mpDefaultProperties = pDefaultProperties;
}

inline void Properties::setProperty(const QString &rKey,
                                    const QString &rValue)
{
    insert(rKey, rValue);
}


} // namespace Log4Qt


Q_DECLARE_TYPEINFO(Log4Qt::Properties, Q_MOVABLE_TYPE);


#endif // LOG4QT_PROPERTIES_H
