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
    bool doConfigure(const Properties &properties,
                     LoggerRepository *loggerRepository = nullptr);

    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    bool doConfigure(const QString &configFileName,
                     LoggerRepository *loggerRepository = nullptr);

    /*!
     * Reads the configuration data from the QSettings object
     * \a settings.
     *
     * \sa \ref Properties::load(const QSettings &) "Properties::load()",
     *     ConfiguratorHelper::configureError()
     */
    bool doConfigure(const QSettings &settings,
                     LoggerRepository *loggerRepository = nullptr);


    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    static bool configure(const Properties &properties);

    /*!
     * \sa ConfiguratorHelper::configureError()
     */
    static bool configure(const QString &configFilename);

    /*!
     * Reads the configuration data from the QSettings object
     * \a settings.
     *
     * \sa \ref doConfigure(const QSettings &, LoggerRepository *) "doConfigure()",
     *     \ref Properties::load(const QSettings &) "Properties::load()",
     *     ConfiguratorHelper::configureError()
     */
    static bool configure(const QSettings &settings);

    /*!
     * \sa ConfiguratorHelper::configureError(),
     *     ConfiguratorHelper::configurationFile()
     */
    static bool configureAndWatch(const QString &configFilename);

private:
    void configureFromFile(const QString &configFileName,
                           LoggerRepository *loggerRepository);
    void configureFromProperties(const Properties &properties,
                                 LoggerRepository *loggerRepository);
    void configureFromSettings(const QSettings &settings,
                               LoggerRepository *loggerRepository);
    void configureGlobalSettings(const Properties &properties,
                                 LoggerRepository *loggerRepository) const;
    void configureNonRootElements(const Properties &properties,
                                  LoggerRepository *loggerRepository);
    void configureRootLogger(const Properties &properties,
                             LoggerRepository *loggerRepository);
    void parseAdditivityForLogger(const Properties &properties,
                                  Logger *logger,
                                  const QString &log4jName) const;
    AppenderSharedPtr parseAppender(const Properties &properties,
                                    const QString &name);
    LayoutSharedPtr parseLayout(const Properties &properties,
                                const QString &appendename);
    void parseLogger(const Properties &properties,
                     Logger *logger,
                     const QString &key,
                     const QString &value);
    void setProperties(const Properties &properties,
                       const QString &prefix,
                       const QStringList &exclusion,
                       QObject *object);
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
