/******************************************************************************
 *
 * package:
 * file:        appenderskeleton.h
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

#ifndef LOG4QT_APPENDERSKELETON_H
#define LOG4QT_APPENDERSKELETON_H

#include "appender.h"
#include "log4qtshared.h"
#include "layout.h"
#include "spi/filter.h"
#include "logger.h"

#include <QMutex>

namespace Log4Qt
{

class Logger;
class LoggingEvent;

/*!
 * \brief The class AppenderSkeleton implements general Appender functionality.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \note The ownership and lifetime of objects of this class are managed. See
 *       \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT AppenderSkeleton : public Appender
{
    Q_OBJECT

    /*!
     * The property holds if the Appender has been activated.
     *
     * \sa isActive()
     */
    Q_PROPERTY(bool isActive READ isActive)

    /*!
     * The property holds if the Appender has been closed.
     *
     * \sa isClosed()
     */
    Q_PROPERTY(bool isClosed READ isClosed)

    /*!
     * The property holds the threshold level used by the Appender.
     *
     * \sa threshold(), setThreshold()
     */
    Q_PROPERTY(Log4Qt::Level threshold READ threshold WRITE setThreshold)

public:
    explicit AppenderSkeleton(QObject *parent = nullptr);

protected:
    explicit AppenderSkeleton(bool isActive,
                              QObject *parent = nullptr);
    explicit AppenderSkeleton(bool isActive,
                              const LayoutSharedPtr &layout,
                              QObject *parent = nullptr);
    ~AppenderSkeleton() override;

public:
    FilterSharedPtr filter() const override;
    LayoutSharedPtr layout() const override;
    bool isActive() const;
    bool isClosed() const;
    QString name() const override;
    Level threshold() const;
    void setLayout(const LayoutSharedPtr &layout) override;
    void setName(const QString &name) override;
    void setThreshold(Level level);

    virtual void activateOptions();
    void addFilter(const FilterSharedPtr &filter) override;
    void clearFilters() override;
    void close() override;

    /*!
     * Performs checks and delegates the actuall appending to the subclass
     * specific append() function.
     *
     * \sa append(), checkEntryConditions(), isAsSevereAsThreshold(), Filter
     */
    void doAppend(const LoggingEvent &event) override;

    FilterSharedPtr firstFilter() const;
    bool isAsSevereAsThreshold(Level level) const;

protected:
    virtual void append(const LoggingEvent &event) = 0;
    void customEvent(QEvent *event) override;

    /*!
     * Tests if all entry conditions for using append() in this class are
     * met.
     *
     * If a conditions is not met, an error is logged and the function
     * returns false.
     *
     * The checked conditions are:
     * - That the appender has been activated (APPENDER_NOT_ACTIVATED_ERROR)
     * - That the appender was not closed (APPENDER_CLOSED_ERROR)
     * - That the appender has a layout set, if it requires one
     *   (logging_error(APPENDER_USE_MISSING_LAYOUT_ERROR)
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by doAppend(). The doAppend() function calls the subclass
     * specific checkEntryConditions() function. The function checks the
     * class specific conditions and calls checkEntryConditions() of
     * it's parent class. The last function called is
     * AppenderSkeleton::checkEntryConditions().
     *
     * \sa doAppend()
     */
    virtual bool checkEntryConditions() const;

protected:
    mutable QMutex mObjectGuard;

private:
    Q_DISABLE_COPY(AppenderSkeleton)
    bool mAppendRecursionGuard;
    volatile bool mIsActive;
    volatile bool mIsClosed;
    LayoutSharedPtr mpLayout;
    Level mThreshold;
    FilterSharedPtr mpHeadFilter;
    FilterSharedPtr mpTailFilter;
    void closeInternal();
};

inline FilterSharedPtr AppenderSkeleton::filter() const
{
    QMutexLocker locker(&mObjectGuard);
    return mpHeadFilter;
}

inline QString AppenderSkeleton::name() const
{
    QMutexLocker locker(&mObjectGuard);
    return objectName();
}

inline Level AppenderSkeleton::threshold() const
{
    return mThreshold;
}

inline void AppenderSkeleton::setName(const QString &name)
{
    QMutexLocker locker(&mObjectGuard);
    setObjectName(name);
}

inline void AppenderSkeleton::setThreshold(Level level)
{
    mThreshold = level;
}

inline bool AppenderSkeleton::isActive() const
{
    return mIsActive;
}

inline bool AppenderSkeleton::isClosed() const
{
    return mIsClosed;
}

inline FilterSharedPtr AppenderSkeleton::firstFilter() const
{
    QMutexLocker locker(&mObjectGuard);
    return filter();
}

inline bool AppenderSkeleton::isAsSevereAsThreshold(Level level) const
{
    return (mThreshold <= level);
}

} // namespace Log4Qt

#endif // LOG4QT_APPENDERSKELETON_H
