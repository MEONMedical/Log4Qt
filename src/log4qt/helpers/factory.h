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

#ifndef LOG4QT_HELPERS_FACTORY_H
#define LOG4QT_HELPERS_FACTORY_H

#include "log4qt/log4qtshared.h"

#include <QHash>
#include <QMutex>
#include <QStringList>
#include <functional>

class QObject;
class QMetaProperty;

namespace Log4Qt
{

class Appender;
class Filter;
class AbstractLayout;
class TriggeringPolicy;
class RolloverStrategy;
class HeaderFooterProvider;

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
class LOG4QT_EXPORT Factory
{
public:
    /*!
         * Prototype for an Appender factory function. The function creates
         * an Appender object on the heap and returns a pointer to it.
         *
         * \sa registerAppender(), createAppender()
     */
    using AppenderFactoryFunc = Appender *(*)();

    /*!
         * Prototype for a Filter factory function. The function creates
         * a Filter object on the heap and returns a pointer to it.
         *
         * \sa registerFilter(), createFilter()
     */
    using FilterFactoryFunc = Filter *(*)();

    /*!
         * Prototype for a Layout factory function. The function creates
         * a Layout object on the heap and returns a pointer to it.
         *
         * \sa registerLayout(), createLayout()
     */
    using LayoutFactoryFunc = AbstractLayout *(*)();

    /*!
         * Prototype for a TriggeringPolicy factory function. The function creates
         * a TriggeringPolicy object on the heap and returns a pointer to it.
         *
         * \sa registerTriggeringPolicy(), createTriggeringPolicy()
     */
    using TriggeringPolicyFactoryFunc = TriggeringPolicy *(*)();

    /*!
         * Prototype for a RolloverStrategy factory function. The function creates
         * a RolloverStrategy object on the heap and returns a pointer to it.
         *
         * \sa registerRolloverStrategy(), createRolloverStrategy()
     */
    using RolloverStrategyFactoryFunc = RolloverStrategy *(*)();

    /*!
         * Prototype for a HeaderFooterProvider factory function. The function creates
         * a HeaderFooterProvider object on the heap and returns a pointer to it.
         *
         * \sa registerHeaderFooterProvider(), createHeaderFooterProvider()
     */
    using HeaderFooterProviderFactoryFunc = std::function<HeaderFooterProvider *()>;

private:
    Factory();
    Q_DISABLE_COPY_MOVE(Factory)

public:
    /*!
     * Creates an object for the class \a appenderClassName on the heap
     * and returns a pointer to it. If the class has no registered factory
     * function a null pointer is returned.
     *
     * \sa registerAppender(), unregisterAppender(), registeredAppenders()
     */
    static Appender *createAppender(const QString &appenderClassName)
    {
        return instance()->doCreateAppender(appenderClassName);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static Appender *createAppender(const char *appenderClassName)
    {
        return instance()->doCreateAppender(QLatin1String(appenderClassName));
    }

    /*!
     * Creates an object for the class \a filterClassName on the heap
     * and returns a pointer to it. If the class has no registered factory
     * function a null pointer is returned.
     *
     * \sa registerFilter(), unregisterFilter(), registeredFilters()
     */
    static Filter *createFilter(const QString &filterClassName)
    {
        return instance()->doCreateFilter(filterClassName);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static Filter *createFilter(const char *filterClassName)
    {
        return instance()->doCreateFilter(QLatin1String(filterClassName));
    }

    /*!
     * Creates an object for the class \a layoutClassName on the heap
     * and returns a pointer to it. If the class has no registered factory
     * function a null pointer is returned.
     *
     * \sa registerLayout(), unregisterLayout(), registeredLayouts()
     */
    static AbstractLayout *createLayout(const QString &layoutClassName)
    {
        return instance()->doCreateLayout(layoutClassName);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static AbstractLayout *createLayout(const char *layoutClassName)
    {
        return instance()->doCreateLayout(QLatin1String(layoutClassName));
    }

    static TriggeringPolicy *createTriggeringPolicy(const QString &className)
    {
        return instance()->doCreateTriggeringPolicy(className);
    }
    static TriggeringPolicy *createTriggeringPolicy(const char *className)
    {
        return instance()->doCreateTriggeringPolicy(QLatin1String(className));
    }

    static RolloverStrategy *createRolloverStrategy(const QString &className)
    {
        return instance()->doCreateRolloverStrategy(className);
    }
    static RolloverStrategy *createRolloverStrategy(const char *className)
    {
        return instance()->doCreateRolloverStrategy(QLatin1String(className));
    }

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
                                 AppenderFactoryFunc appenderFactoryFunc)
    {
        instance()->doRegisterAppender(appenderClassName, appenderFactoryFunc);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void registerAppender(const char *appenderClassName,
                                 AppenderFactoryFunc appenderFactoryFunc)
    {
        instance()->doRegisterAppender(QLatin1String(appenderClassName), appenderFactoryFunc);
    }

    /*!
     * Registers the Filter factory function \a filterFactoryFunc
     * for the class \a filterClassName. If a registered factory
     * function exists for the class, it is replaced with
     * \a filterFactoryFunc.
     *
     * \sa unregisterFilter(), registeredFilters(), createFilter()
     */
    static void registerFilter(const QString &filterClassName,
                               FilterFactoryFunc filterFactoryFunc)
    {
        instance()->doRegisterFilter(filterClassName, filterFactoryFunc);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void registerFilter(const char *filterClassName,
                               FilterFactoryFunc filterFactoryFunc)
    {
        instance()->doRegisterFilter(QLatin1String(filterClassName), filterFactoryFunc);
    }

    /*!
     * Registers the Layout factory function \a layoutFactoryFunc
     * for the class \a layoutClassName. If a registered factory
     * function exists for the class, it is replaced with
     * \a layoutFactoryFunc.
     *
     * \sa unregisterLayout(), registeredLayout(), createLayout()
     */
    static void registerLayout(const QString &layoutClassName,
                               LayoutFactoryFunc layoutFactoryFunc)
    {
        instance()->doRegisterLayout(layoutClassName, layoutFactoryFunc);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void registerLayout(const char *layoutClassName,
                               LayoutFactoryFunc layoutFactoryFunc)
    {
        instance()->doRegisterLayout(QLatin1String(layoutClassName), layoutFactoryFunc);
    }

    static void registerTriggeringPolicy(const QString &className,
                                          TriggeringPolicyFactoryFunc func)
    {
        instance()->doRegisterTriggeringPolicy(className, func);
    }
    static void registerTriggeringPolicy(const char *className,
                                          TriggeringPolicyFactoryFunc func)
    {
        instance()->doRegisterTriggeringPolicy(QLatin1String(className), func);
    }

    static void registerRolloverStrategy(const QString &className,
                                          RolloverStrategyFactoryFunc func)
    {
        instance()->doRegisterRolloverStrategy(className, func);
    }
    static void registerRolloverStrategy(const char *className,
                                          RolloverStrategyFactoryFunc func)
    {
        instance()->doRegisterRolloverStrategy(QLatin1String(className), func);
    }

    /*!
    * Returns a list of the class names for registered Appender factory
    * functions.
    *
    * \sa registerAppender(), unregisterAppender()
    */
    static QStringList registeredAppenders()
    {
        QMutexLocker locker(&instance()->mObjectGuard);
        return instance()->mAppenderRegistry.keys();
    }

    /*!
     * Returns a list of the class names for registered Filter factory
     * functions.
     *
     * \sa registerFilter(), unregisterFilter()
     */
    static QStringList registeredFilters()
    {
        QMutexLocker locker(&instance()->mObjectGuard);
        return instance()->mFilterRegistry.keys();
    }

    /*!
     * Returns a list of the class names for registered Layout factory
     * functions.
     *
     * \sa registerLayout(), unregisterLayout()
     */
    static QStringList registeredLayouts()
    {
        QMutexLocker locker(&instance()->mObjectGuard);
        return instance()->mLayoutRegistry.keys();
    }

    static QStringList registeredTriggeringPolicies()
    {
        QMutexLocker locker(&instance()->mObjectGuard);
        return instance()->mTriggeringPolicyRegistry.keys();
    }
    static QStringList registeredRolloverStrategies()
    {
        QMutexLocker locker(&instance()->mObjectGuard);
        return instance()->mRolloverStrategyRegistry.keys();
    }

    static void unregisterTriggeringPolicy(const QString &className)
    {
        instance()->doUnregisterTriggeringPolicy(className);
    }
    static void unregisterTriggeringPolicy(const char *className)
    {
        instance()->doUnregisterTriggeringPolicy(QLatin1String(className));
    }

    static void unregisterRolloverStrategy(const QString &className)
    {
        instance()->doUnregisterRolloverStrategy(className);
    }
    static void unregisterRolloverStrategy(const char *className)
    {
        instance()->doUnregisterRolloverStrategy(QLatin1String(className));
    }

    static HeaderFooterProvider *createHeaderFooterProvider(const QString &className)
    {
        return instance()->doCreateHeaderFooterProvider(className);
    }
    static HeaderFooterProvider *createHeaderFooterProvider(const char *className)
    {
        return instance()->doCreateHeaderFooterProvider(QLatin1String(className));
    }

    static void registerHeaderFooterProvider(const QString &className,
                                              HeaderFooterProviderFactoryFunc func)
    {
        instance()->doRegisterHeaderFooterProvider(className, func);
    }
    static void registerHeaderFooterProvider(const char *className,
                                              HeaderFooterProviderFactoryFunc func)
    {
        instance()->doRegisterHeaderFooterProvider(QLatin1String(className), func);
    }

    static void unregisterHeaderFooterProvider(const QString &className)
    {
        instance()->doUnregisterHeaderFooterProvider(className);
    }
    static void unregisterHeaderFooterProvider(const char *className)
    {
        instance()->doUnregisterHeaderFooterProvider(QLatin1String(className));
    }

    static QStringList registeredHeaderFooterProviders()
    {
        QMutexLocker locker(&instance()->mObjectGuard);
        return instance()->mHeaderFooterProviderRegistry.keys();
    }

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
                                  const QString &value)
    {
        instance()->doSetObjectProperty(object, property, value);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void setObjectProperty(QObject *object,
                                  const char *property,
                                  const QString &value)
    {
        instance()->doSetObjectProperty(object, QLatin1String(property), value);
    }

    /*!
    * Unregisters the Appender factory function for the class
    * \a rAppenderClassName.
    *
    * \sa registerAppender(), registeredAppenders()
    */
    static void unregisterAppender(const QString &appenderClassName)
    {
        instance()->doUnregisterAppender(appenderClassName);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void unregisterAppender(const char *appenderClassName)
    {
        instance()->doUnregisterAppender(QLatin1String(appenderClassName));
    }

    /*!
     * Unregisters the Filter factory function for the class
     * \a filterClassName.
     *
     * \sa registerFilter(), registeredFilters()
     */
    static void unregisterFilter(const QString &filterClassName)
    {
        instance()->doUnregisterFilter(filterClassName);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void unregisterFilter(const char *filterClassName)
    {
        instance()->doUnregisterFilter(QLatin1String(filterClassName));
    }

    /*!
     * Unregisters the Layout factory function for the class
     * \a layoutClassName.
     *
     * \sa registerLayout(), registeredLayouts()
     */
    static void unregisterLayout(const QString &layoutClassName)
    {
        instance()->doUnregisterLayout(layoutClassName);
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    static void unregisterLayout(const char *layoutClassName)
    {
        instance()->doUnregisterLayout(QLatin1String(layoutClassName));
    }

private:
    Appender *doCreateAppender(const QString &appenderClassName);
    Filter *doCreateFilter(const QString &filterClassName);
    AbstractLayout *doCreateLayout(const QString &layoutClassName);
    void doRegisterAppender(const QString &appenderClassName,
                            AppenderFactoryFunc appenderFactoryFunc);
    void doRegisterFilter(const QString &filterClassName,
                          FilterFactoryFunc filterFactoryFunc);
    void doRegisterLayout(const QString &filterClassName,
                          LayoutFactoryFunc layoutFactoryFunc);
    TriggeringPolicy *doCreateTriggeringPolicy(const QString &className);
    void doRegisterTriggeringPolicy(const QString &className,
                                     TriggeringPolicyFactoryFunc func);
    void doUnregisterTriggeringPolicy(const QString &className);
    RolloverStrategy *doCreateRolloverStrategy(const QString &className);
    void doRegisterRolloverStrategy(const QString &className,
                                     RolloverStrategyFactoryFunc func);
    void doUnregisterRolloverStrategy(const QString &className);
    HeaderFooterProvider *doCreateHeaderFooterProvider(const QString &className);
    void doRegisterHeaderFooterProvider(const QString &className,
                                         HeaderFooterProviderFactoryFunc func);
    void doUnregisterHeaderFooterProvider(const QString &className);
    void doSetObjectProperty(QObject *object,
                             const QString &property,
                             const QString &value);
    void doUnregisterAppender(const QString &appenderClassName);
    void doUnregisterFilter(const QString &filterClassName);
    void doUnregisterLayout(const QString &filterClassName);
    void registerDefaultAppenders();
    void registerDefaultFilters();
    void registerDefaultLayouts();
    void registerDefaultTriggeringPolicies();
    void registerDefaultRolloverStrategies();
    void registerDefaultHeaderFooterProviders();
    bool validateObjectProperty(QMetaProperty &metaProperty,
                                const QString &property,
                                QObject *object);

private:
    mutable QMutex mObjectGuard;
    QHash<QString, AppenderFactoryFunc> mAppenderRegistry;
    QHash<QString, FilterFactoryFunc> mFilterRegistry;
    QHash<QString, LayoutFactoryFunc> mLayoutRegistry;
    QHash<QString, TriggeringPolicyFactoryFunc> mTriggeringPolicyRegistry;
    QHash<QString, RolloverStrategyFactoryFunc> mRolloverStrategyRegistry;
    QHash<QString, HeaderFooterProviderFactoryFunc> mHeaderFooterProviderRegistry;
};

} // namespace Log4Qt

#endif // LOG4QT_HELPERS_FACTORY_H
