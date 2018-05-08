/******************************************************************************
 *
 * package:     Log4Qt
 * file:        factory.h
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

#ifndef LOG4QT_HELPERS_FACTORY_H
#define LOG4QT_HELPERS_FACTORY_H

#include <log4qt/log4qtshared.h>

#include <QHash>
#include <QMutex>
#include <QStringList>

class QObject;
class QMetaProperty;

namespace Log4Qt
{

class Appender;
class Filter;
class Layout;

/*!
 * \brief The class Factory provides factories for Appender, Filter and
 *        Layout objects.
 *
 * The functions createAppender(), createFilter() and createLayout()
 * allow to create objects by specifying their class names. By default
 * all classes of the package are recognised with their Log4j and Log4Qt
 * classanmes. For example an object of the class FileAppender can be
 * craeted using "org.apache.log4j.FileAppender" or "Log4Qt::FileAppender".
 * Additional classes can be registered using registerAppender(),
 * registerFilter() and registerLayout().
 *
 * An QObject property can be set from a string value with
 * setObjectProperty(). The function handles the required error checking
 * and type conversion.
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \sa PropertyConfigurator
 */
class  LOG4QT_EXPORT Factory
{
public:
    /*!
         * Prototype for an Appender factory function. The function creates
         * an Appender object on the heap and returns a pointer to it.
         *
         * \sa registerAppender(), createAppender()
     */
    typedef Appender *(*AppenderFactoryFunc)();

    /*!
         * Prototype for a Filter factory function. The function creates
         * a Filter object on the heap and returns a pointer to it.
         *
         * \sa registerFilter(), createFilter()
     */
    typedef Filter *(*FilterFactoryFunc)();

    /*!
         * Prototype for a Layout factory function. The function creates
         * a Layout object on the heap and returns a pointer to it.
         *
         * \sa registerLayout(), createLayout()
     */
    typedef Layout *(*LayoutFactoryFunc)();

private:
    Factory();
    Q_DISABLE_COPY(Factory)

public:
    /*!
     * Creates an object for the class \a appenderClassName on the heap
     * and returns a pointer to it. If the class has no registered factory
     * function a null pointer is returned.
     *
     * \sa registerAppender(), unregisterAppender(), registeredAppenders()
     */
    static Appender *createAppender(const QString &appenderClassName);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static Appender *createAppender(const char *appenderClassName);

    /*!
     * Creates an object for the class \a filterClassName on the heap
     * and returns a pointer to it. If the class has no registered factory
     * function a null pointer is returned.
     *
     * \sa registerFilter(), unregisterFilter(), registeredFilters()
     */
    static Filter *createFilter(const QString &filterClassName);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static Filter *createFilter(const char *filterClassName);

    /*!
     * Creates an object for the class \a layoutClassName on the heap
     * and returns a pointer to it. If the class has no registered factory
     * function a null pointer is returned.
     *
     * \sa registerLayout(), unregisterLayout(), registeredLayouts()
     */
    static Layout *createLayout(const QString &layoutClassName);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static Layout *createLayout(const char *layoutClassName);

    /*!
     * Returns the Factory instance.
     */
    static Factory *instance();

    /*!
    * Registers the Appender factory function \a appenderFactoryFunc
    * for the class \a appenderClassName. If a registered factory
    * function exists for the class, it is replaced with
    * \a appenderFactoryFunc.
    *
    * \sa unregisterAppender(), registeredAppenders(), createAppender()
    */
    static void registerAppender(const QString &appenderClassName,
                                 AppenderFactoryFunc appenderFactoryFunc);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void registerAppender(const char *appenderClassName,
                                 AppenderFactoryFunc appenderFactoryFunc);

    /*!
     * Registers the Filter factory function \a filterFactoryFunc
     * for the class \a filterClassName. If a registered factory
     * function exists for the class, it is replaced with
     * \a filterFactoryFunc.
     *
     * \sa unregisterFilter(), registeredFilters(), createFilter()
     */
    static void registerFilter(const QString &filterClassName,
                               FilterFactoryFunc filterFactoryFunc);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void registerFilter(const char *filterClassName,
                               FilterFactoryFunc filterFactoryFunc);

    /*!
     * Registers the Layout factory function \a layoutFactoryFunc
     * for the class \a filterClassName. If a registered factory
     * function exists for the class, it is replaced with
     * \a layoutFactoryFunc.
     *
     * \sa unregisterLayout(), registeredLayout(), createLayout()
     */
    static void registerLayout(const QString &layoutClassName,
                               LayoutFactoryFunc layoutFactoryFunc);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void registerLayout(const char *layoutClassName,
                               LayoutFactoryFunc layoutFactoryFunc);

    /*!
    * Returns a list of the class names for registered Appender factory
    * functions.
    *
    * \sa registerAppender(), unregisterAppender()
    */
    static QStringList registeredAppenders();

    /*!
     * Returns a list of the class names for registered Filter factory
     * functions.
     *
     * \sa registerFilter(), unregisterFilter()
     */
    static QStringList registeredFilters();

    /*!
     * Returns a list of the class names for registered Layout factory
     * functions.
     *
     * \sa registerLayout(), unregisterLayout()
     */
    static QStringList registeredLayouts();

    /*!
     * Sets the property \a rProperty of the object \a pObject to the
     * value \a rValue. The function will test that the property
     * \a rProperty is writeable and of a type the function can convert to.
     * The types bool, int, Level and QString are supported.
     *
     * \sa OptionConverter
     */
    static void setObjectProperty(QObject *object,
                                  const QString &property,
                                  const QString &value);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void setObjectProperty(QObject *object,
                                  const char *property,
                                  const QString &value);

    /*!
    * Unregisters the Appender factory function for the class
    * \a rAppenderClassName.
    *
    * \sa registerAppender(), registeredAppenders()
    */
    static void unregisterAppender(const QString &appenderClassName);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void unregisterAppender(const char *appenderClassName);

    /*!
     * Unregisters the Filter factory function for the class
     * \a filterClassName.
     *
     * \sa registerFilter(), registeredFilters()
     */
    static void unregisterFilter(const QString &filterClassName);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void unregisterFilter(const char *filterClassName);

    /*!
     * Unregisters the Layout factory function for the class
     * \a filterClassName.
     *
     * \sa registerLayout(), registeredLayouts()
     */
    static void unregisterLayout(const QString &filterClassName);

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void unregisterLayout(const char *layoutClassName);

private:
    Appender *doCreateAppender(const QString &appenderClassName);
    Filter *doCreateFilter(const QString &filterClassName);
    Layout *doCreateLayout(const QString &layoutClassName);
    void doRegisterAppender(const QString &appenderClassName,
                            AppenderFactoryFunc appenderFactoryFunc);
    void doRegisterFilter(const QString &filterClassName,
                          FilterFactoryFunc filterFactoryFunc);
    void doRegisterLayout(const QString &filterClassName,
                          LayoutFactoryFunc layoutFactoryFunc);
    void doSetObjectProperty(QObject *object,
                             const QString &property,
                             const QString &value);
    void doUnregisterAppender(const QString &appenderClassName);
    void doUnregisterFilter(const QString &filterClassName);
    void doUnregisterLayout(const QString &filterClassName);
    void registerDefaultAppenders();
    void registerDefaultFilters();
    void registerDefaultLayouts();
    bool validateObjectProperty(QMetaProperty &metaProperty,
                                const QString &property,
                                QObject *object);

private:
    mutable QMutex mObjectGuard;
    QHash<QString, AppenderFactoryFunc> mAppenderRegistry;
    QHash<QString, FilterFactoryFunc> mFilterRegistry;
    QHash<QString, LayoutFactoryFunc> mLayoutRegistry;
};

inline Appender *Factory::createAppender(const QString &appenderClassName)
{
    return instance()->doCreateAppender(appenderClassName);
}

inline Appender *Factory::createAppender(const char *appenderClassName)
{
    return instance()->doCreateAppender(QLatin1String(appenderClassName));
}

inline Filter *Factory::createFilter(const QString &filterClassName)
{
    return instance()->doCreateFilter(filterClassName);
}

inline Filter *Factory::createFilter(const char *layoutClassName)
{
    return instance()->doCreateFilter(QLatin1String(layoutClassName));
}

inline Layout *Factory::createLayout(const QString &layoutClassName)
{
    return instance()->doCreateLayout(layoutClassName);
}

inline Layout *Factory::createLayout(const char *layoutClassName)
{
    return instance()->doCreateLayout(QLatin1String(layoutClassName));
}

inline void Factory::registerAppender(const QString &appenderClassName,
                                      AppenderFactoryFunc appenderFactoryFunc)
{
    instance()->doRegisterAppender(appenderClassName, appenderFactoryFunc);
}

inline void Factory::registerAppender(const char *appenderClassName,
                                      AppenderFactoryFunc appenderFactoryFunc)
{
    instance()->doRegisterAppender(QLatin1String(appenderClassName), appenderFactoryFunc);
}

inline void Factory::registerFilter(const QString &filterClassName,
                                    FilterFactoryFunc filterFactoryFunc)
{
    instance()->doRegisterFilter(filterClassName, filterFactoryFunc);
}

inline void Factory::registerFilter(const char *filterClassName,
                                    FilterFactoryFunc filterFactoryFunc)
{
    instance()->doRegisterFilter(QLatin1String(filterClassName), filterFactoryFunc);
}

inline void Factory::registerLayout(const QString &filterClassName,
                                    LayoutFactoryFunc layoutFactoryFunc)
{
    instance()->doRegisterLayout(filterClassName, layoutFactoryFunc);
}

inline void Factory::registerLayout(const char *layoutClassName,
                                    LayoutFactoryFunc layoutFactoryFunc)
{
    instance()->doRegisterLayout(QLatin1String(layoutClassName), layoutFactoryFunc);
}

inline QStringList Factory::registeredAppenders()
{
    QMutexLocker locker(&instance()->mObjectGuard);
    return instance()->mAppenderRegistry.keys();
}

inline QStringList Factory::registeredFilters()
{
    QMutexLocker locker(&instance()->mObjectGuard);
    return instance()->mFilterRegistry.keys();
}

inline QStringList Factory::registeredLayouts()
{
    QMutexLocker locker(&instance()->mObjectGuard);
    return instance()->mLayoutRegistry.keys();
}

inline void Factory::setObjectProperty(QObject *object,
                                       const QString &property,
                                       const QString &value)
{
    instance()->doSetObjectProperty(object, property, value);
}

inline void Factory::setObjectProperty(QObject *object,
                                       const char *property,
                                       const QString &value)
{
    instance()->doSetObjectProperty(object, QLatin1String(property), value);
}

inline void Factory::unregisterAppender(const QString &appenderClassName)
{
    instance()->doUnregisterAppender(appenderClassName);
}

inline void Factory::unregisterAppender(const char *appenderClassName)
{
    instance()->doUnregisterAppender(QLatin1String(appenderClassName));
}

inline void Factory::unregisterFilter(const QString &filterClassName)
{
    instance()->doUnregisterFilter(filterClassName);
}

inline void Factory::unregisterFilter(const char *filterClassName)
{
    instance()->doUnregisterFilter(QLatin1String(filterClassName));
}

inline void Factory::unregisterLayout(const QString &filterClassName)
{
    instance()->doUnregisterLayout(filterClassName);
}

inline void Factory::unregisterLayout(const char *layoutClassName)
{
    instance()->doUnregisterLayout(QLatin1String(layoutClassName));
}

} // namespace Log4Qt

#endif // LOG4QT_HELPERS_FACTORY_H
