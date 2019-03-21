/******************************************************************************
 *
 * package:     Log4Qt
 * file:        initialisationhelper.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * changes:     Sep 2008, Martin Heinrich:
 *              - Replaced usage of q_atomic_test_and_set_ptr with
 *                QBasicAtomicPointer
 *              Feb 2016, Andreas Bacher:
 *              - Replaced usage of QBasicAtomicPointer with
 *                magic static initalization (thread safe with c++11)
 *
 *
 * Copyright 2007 - 2008 Martin Heinrich
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

#ifndef LOG4QT_HELPERS_INITIALISATIONHELPER_H
#define LOG4QT_HELPERS_INITIALISATIONHELPER_H

#include <log4qt/log4qtshared.h>

#include <QHash>
#include <QString>

class QMutex;

namespace Log4Qt
{

/*!
 * LOG4QT_IMPLEMENT_INSTANCE implements an instance function for a
 * singleton class \a TYPE.
 *
 *
 * The following example illustrates how to use the macro to create a
 * singleton class:
 *
 * \code
 * #file: mysingleton.h
 *
 * class MySingleton
 * {
 * private:
 *     MySingleton();
 *     ~MySingleton();
 * public:
 *     MySingleton *instance();
 * }
 * \endcode
 * \code
 * #file: mysingleton.cpp
 *
 * #include mysingleton.h
 *
 * MySingleton::MySingleton()
 * {}
 *
 * MySingleton::~MySingleton()
 * {}
 *
 * LOG4QT_IMPLEMENT_INSTANCE(MySingleton)
 *
 * \endcode
 *
 * \note The function created by the macro is thread-safe.
 *
 * \sa \ref Log4Qt::InitialisationHelper "InitialisationHelper"
 */
#define LOG4QT_IMPLEMENT_INSTANCE(TYPE)                  \
                TYPE *TYPE::instance()                   \
                {                                        \
                    static auto * singelton(new TYPE);   \
                    return singelton;                    \
                }

/*!
 * \brief The class InitialisationHelper performs static initialisation
 *        tasks.
 *
 * The InitialisationHelper is either created on the first call or through
 * static initialisation. It will capture the programs startup time,
 * which can be retrieved using startTime(). The system environment
 * is analysed for package related definitions. The result is available
 * over environmentSettings(). The packages custom types are registered with
 * the Qt type system.
 *
 * Settings for the package can be retrieved using setting(). Two macros
 * are available to help with the creation of singletons / global static
 * objects  and \ref Log4Qt::LOG4QT_IMPLEMENT_INSTANCE "LOG4QT_IMPLEMENT_INSTANCE").
 *
 * \note All the functions declared in this class are thread-safe.
 *
 * \sa \ref Init "Initialization procedure",
 */
class  LOG4QT_EXPORT InitialisationHelper
{
private:
    InitialisationHelper();
    virtual ~InitialisationHelper();

    Q_DISABLE_COPY(InitialisationHelper)

public:

    /*!
     * Returns a hash with the settings retrieved from the system
     * environment on startup.
     *
     * The following table shows the environment variables taken into
     * account and the setting key used for them.
     *
     * <table align="center" border="1" cellpadding="2" cellspacing="0" bordercolor="#84b0c7">
     * <tr bgcolor="#d5e1e8">
     * <th width="25%"> Environment variable </th>
     * <th width="25%"> Setting key </th>
     * </tr><tr>
     * <td> LOG4QT_DEBUG </td>
     * <td> Debug </td>
     * </tr><tr bgcolor="#ffffff">
     * <td> LOG4QT_DEFAULTINITOVERRIDE </td>
     * <td> DefaultInitOverride </td>
     * </tr><tr>
     * <td> LOG4QT_CONFIGURATION </td>
     * <td> Configuration </td>
     * </tr>
     * </table>
     *
     * \sa \ref Env "Environment Variables",
     *     setting()
     */
    static QHash<QString, QString> environmentSettings();

    /*!
    * Returns the InitialisationHelper instance.
    */
    static InitialisationHelper *instance();

    /*!
     * Returns the value for the setting \a rKey or \a rDefault, if it is
     * not defined.
     *
     * A setting can be either defined by an environment variable or by a
     * key in the application setting. The function will first test the
     * settings made by environment variables for the key \a rKey using
     * environmentSettings(). If the key is not present and a
     * QCoreApplication exists, the application settings are tested for
     * the key \a rKey in the group \c %Log4Qt.
     *
     * The following setting exists:
     *
     * <table align="center" border="1" cellpadding="2" cellspacing="0" bordercolor="#84b0c7">
     * <tr bgcolor="#d5e1e8">
     * <th width="25%"> Setting key </th>
     * <th> Description </th>
     * </tr><tr>
     * <td> Debug </td>
     * <td> The variable controls the Level value for the logger
     * LogManager::logLogger(). If the value is a valid Level string,
     * the level for the logger is set to the level. If the value is not
     * a valid Level string,  \ref Level::DEBUG_INT "DEBUG_INT" is used.
     * Otherwise \ref Level::ERROR_INT "ERROR_INT" is used. </td>
     * </tr><tr bgcolor="#ffffff">
     * <td> DefaultInitOverride </td>
     * <td> The variable controls the \ref Init "initialization procedure"
     * performed by the \ref LogManager "LogManager" on startup.
     * If it is set to any other value then \c false the \ref Init
     * "initialization procedure" is skipped.</td>
     * </tr><tr>
     * <td> Configuration </td>
     * <td> Specifies the configuration file used for initialising the package.</td>
     * </tr><tr bgcolor="#ffffff">
     * <td> ConfiguratorClass </td>
     * <td> Specifies the configurator class used for initialising the package.</td>
     * </tr>
     * </table>
     *
     * \sa environmentSettings(), \ref Env "Environment Variables",
     *     \ref Init "Initialization procedure",
     *     LogManager::configureLogLogger(), LogManager::startup()
     */
    static QString setting(const QString &key,
                           const QString &defaultValue = QString());

    /*!
     * Returns the start time of the program as the number of milliseconds
     * that have passed since 1970-01-01T00:00:00,000, Coordinated
     * Universal Time (Qt::UTC).
     *
     * \sa DateTime::fromMilliSeconds(),
     *     DateTime::toMilliSeconds()
     */
    static qint64 startTime();

private:
    void doInitialiseEnvironmentSettings();
    void doRegisterTypes();
    QString doSetting(const QString &key,
                      const QString &defaultValue) const;
    static bool shutdown();
    static bool staticInitialisation();

private:
    const qint64 mStartTime;
    QHash <QString, QString> mEnvironmentSettings;
    static bool mStaticInitialisation;

};

inline QHash<QString, QString> InitialisationHelper::environmentSettings()
{
    return instance()->mEnvironmentSettings;
}

inline QString InitialisationHelper::setting(const QString &key,
        const QString &defaultValue)
{
    return instance()->doSetting(key, defaultValue);
}

inline qint64 InitialisationHelper::startTime()
{
    return instance()->mStartTime;
}

} // namespace Log4Qt

#endif // LOG4QT_HELPERS_INITIALISATIONHELPER_H
