/******************************************************************************
 *
 * package:     Logging
 * file:        propertyconfigurator.h
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

#ifndef LOG4QT_PROPERTYCONFIGURATOR_H
#define LOG4QT_PROPERTYCONFIGURATOR_H

#include "log4qt.h"
#include "layout.h"
#include "appender.h"

#include <QHash>

class QSettings;

namespace Log4Qt
{

class ListAppender;
class Logger;
class Properties;
class LoggerRepository;

/*!
 * \brief The class PropertyConfigurator allows the configuration of the
 *        package from a JAVA properties file.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class  LOG4QT_EXPORT PropertyConfigurator
{
public:
    PropertyConfigurator();

private:
    Q_DISABLE_COPY(PropertyConfigurator)

public:
    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    bool doConfigure(const Properties &rProperties,
                     LoggerRepository *pLoggerRepository = Q_NULLPTR);

    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    bool doConfigure(const QString &rConfigFileName,
                     LoggerRepository *pLoggerRepository = Q_NULLPTR);

    /*!
     * Reads the configuration data from the QSettings object
     * \a rSettings.
     *
     * \sa \ref Properties::load(const QSettings &) "Properties::load()",
     *     ConfiguratorHelper::configureError()
     */
    bool doConfigure(const QSettings &rSettings,
                     LoggerRepository *pLoggerRepository = Q_NULLPTR);


    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    static bool configure(const Properties &rProperties);

    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    static bool configure(const QString &rConfigFilename);

    /*!
     * Reads the configuration data from the QSettings object
     * \a rSettings.
     *
     * \sa \ref doConfigure(const QSettings &, LoggerRepository *) "doConfigure()",
     *     \ref Properties::load(const QSettings &) "Properties::load()",
     *     ConfiguratorHelper::configureError()
     */
    static bool configure(const QSettings &rSettings);

    /*!
     * \sa ConfiguratorHelper::configureError(),
     *     ConfiguratorHelper::configurationFile()
     */
    static bool configureAndWatch(const QString &rConfigFilename);

private:
    void configureFromFile(const QString &rConfigFileName,
                           LoggerRepository *pLoggerRepository);
    void configureFromProperties(const Properties &rProperties,
                                 LoggerRepository *pLoggerRepository);
    void configureFromSettings(const QSettings &rSettings,
                               LoggerRepository *pLoggerRepository);
    void configureGlobalSettings(const Properties &rProperties,
                                 LoggerRepository *pLoggerRepository) const;
    void configureNonRootElements(const Properties &rProperties,
                                  LoggerRepository *pLoggerRepository);
    void configureRootLogger(const Properties &rProperties,
                             LoggerRepository *pLoggerRepository);
    void parseAdditivityForLogger(const Properties &rProperties,
                                  Logger *pLogger,
                                  const QString &rLog4jName) const;
    AppenderSharedPtr parseAppender(const Properties &rProperties,
                                    const QString &rName);
    LayoutSharedPtr parseLayout(const Properties &rProperties,
                                const QString &rAppenderName);
    void parseLogger(const Properties &rProperties,
                     Logger *pLogger,
                     const QString &rKey,
                     const QString &rValue);
    void setProperties(const Properties &rProperties,
                       const QString &rPrefix,
                       const QStringList &rExclusions,
                       QObject *pObject);
    void startCaptureErrors();
    bool stopCaptureErrors();

private:
    AppenderSharedPtr mpConfigureErrors;
    QHash<QString, AppenderSharedPtr> mAppenderRegistry;
};

inline PropertyConfigurator::PropertyConfigurator()
{}

} // namspace Log4Qt

#endif // LOG4QT_PROPERTYCONFIGURATOR_H
