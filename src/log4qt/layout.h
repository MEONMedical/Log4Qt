/******************************************************************************
 *
 * package:     Log4Qt
 * file:        layout.h
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

#ifndef LOG4QT_LAYOUT_H
#define LOG4QT_LAYOUT_H

#include "log4qt.h"
#include "log4qtsharedptr.h"

#include <QObject>

namespace Log4Qt
{

class LoggingEvent;

/*!
 * \brief The class Layout is the base class for all layouts.
 *
 * \note The ownership and lifetime of objects of this class are managed. See
 *       \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT Layout : public QObject
{
    Q_OBJECT

    /*!
     * The property holds the content type of the layout.
     *
     * \sa contentType()
     */
    Q_PROPERTY(QString footercontentType READ contentType)
    /*!
     * The property holds the footer used by the layout.
     *
     * \sa footer(), setFooter()
     */
    Q_PROPERTY(QString footer READ footer WRITE setFooter)
    /*!
     * The property holds the header used by the layout.
     *
     * \sa header(), setHeader()
     */
    Q_PROPERTY(QString header READ header WRITE setHeader)

public:
    Layout(QObject *parent = nullptr);
    virtual ~Layout();

public:
    virtual QString contentType() const;
    QString footer() const;
    QString header() const;
    QString name() const;
    void setFooter(const QString &footer);
    void setHeader(const QString &header);
    void setName(const QString &name);

    virtual void activateOptions();
    virtual QString format(const LoggingEvent &event) = 0;

    /*!
     * Returns the end of line seperator for the operating system.
     *
     * Windows: \\r\\n
     * Mac: \\r
     * UNIX: \\n
     */
    static QString endOfLine();

    // Member variables
private:
    Q_DISABLE_COPY(Layout)
    QString mFooter;
    QString mHeader;
};

inline QString Layout::footer() const
{
    return mFooter;
}

inline QString Layout::header() const
{
    return mHeader;
}

inline QString Layout::name() const
{
    return objectName();
}

inline void Layout::setFooter(const QString &footer)
{
    mFooter = footer;
}

inline void Layout::setHeader(const QString &header)
{
    mHeader = header;
}

inline void Layout::setName(const QString &name)
{
    setObjectName(name);
}

using LayoutSharedPtr = Log4QtSharedPtr<Layout>;

} // namespace Log4Qt

#endif // LOG4QT_LAYOUT_H
