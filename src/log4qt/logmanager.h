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

#ifndef LOG4QT_LOGMANAGER_H
#define LOG4QT_LOGMANAGER_H

#include "level.h"
#include "logger.h"

#include <QList>
#include <QMutex>
#include <QString>
#include <QVersionNumber>

#include <atomic>

namespace Log4Qt
{

class LoggerRepository;

/*!
 * \brief The class LogManager manages Logger in the default
 *        LoggerRepository.
 *
 * The LogManager manages logger in a single Hierarchy instance. It
 * provides access to special logger over the logLogger(), qtLogger()
 * and rootLogger() member functions.
 *
 * The LogManager is handling the initialisation on startup. The
 * initialisation procedure will first attempt to configure the package
 * based on environment variables. If the attempt fails it will check for
 * the existence of configuration files in several location. For detailed
 * description of the initialisation procedure see \ref Init
 * "Initialization procedure".
 *
 * Messages created by qDebug(), qWarning(), qCritical() and qFatal() can
 * be can be handled by the LogManager. By default the message handling
 * is disabled. It can be enabled by calling setHandleQtMessages(). Once
 * enabled all messages are logged using the logger qtLogger().
 *
 * The Log4Qt runtime version is accessible over version(). The macros
     * \ref Log4Qt::LOG4QT_VERSION "LOG4QT_VERSION" and
     * \ref Log4Qt::LOG4QT_VERSION_STR "LOG4QT_VERSION_STR" provide the
     * compile time version.
     *
 * \note All the functions declared in this class are thread-safe.
 */
class LOG4QT_EXPORT LogManager
{
private:
    LogManager();
    ~LogManager();
    Q_DISABLE_COPY_MOVE(LogManager)

public:
    /*!
     * Returns if the handling of messages created by calls to qDebug(),
     * qWarning(), qCritical() and qFatal() is activated.
     *
     * \sa setHandleQtMessages()
     */
    [[nodiscard]] static bool handleQtMessages()
    {
        return instance()->mHandleQtMessages.load(std::memory_order_acquire);
    }

    /*!
     * Returns true, if the current properties file is watched with a QFileWatcher
     *
     * \sa setWatchThisFile()
     */
    [[nodiscard]] static bool watchThisFile()
    {
        return instance()->mWatchThisFile.load(std::memory_order_acquire);
    }

    /*!
     * Returns the filter rules for qc[Info|Debug|Warning|Critical]
     *
     * \sa setFilterRules()
     */
    [[nodiscard]] static QString filterRules();

    /*!
     * Returns the message pattern for qc[Info|Debug|Warning|Critical]
     *
     * \sa setMessagePattern()
     */
    [[nodiscard]] static QString messagePattern();

    [[nodiscard]] static LoggerRepository *loggerRepository()
    {
        return instance()->mLoggerRepository;
    }

    /*!
     * Returns the logger used for logging internal messages. See
     * \ref LogLog "Logging within the package" for more details.
     *
     * Calling this function is equivalent to calling logger("Log4Qt").
     */
    [[nodiscard]] static Logger *logLogger()
    {
        return logger(QStringLiteral("Log4Qt"));
    }

    /*!
     * Returns a pointer to the logger used for logging messages created by
     * calls to qDebug(), qWarning(), qCritical() and qFatal().
     *
     * Calling this function is equivalent to calling logger("Qt").
     *
     * \sa setHandleQtMessages()
     */
    [[nodiscard]] static Logger *qtLogger()
    {
        return logger(QStringLiteral("Qt"));
    }

    [[nodiscard]] static Logger *rootLogger();
    [[nodiscard]] static QList<Logger *> loggers();
    [[nodiscard]] static Level threshold();
    static void setThreshold(Level level);

    /*!
     * Activates or deactivates the handling of messages created by calls
     * to qDebug(), qWarning(), qCritical() and qFatal() is activated.
     *
     * If activated, a Qt message handler is installed. Messages are logged
     * using the logger returned by qtLogger(). For fatal messages the same
     * exit procedure is implemented as for qFatal().
     *
     * The following mappping is used from QtMsgType to Level:
     *
         * <table align="center" border="1" cellpadding="2" cellspacing="0" bordercolor="#84b0c7">
         * <tr bgcolor="#d5e1e8">
         * <th> &nbsp;&nbsp;&nbsp; QtMsgType &nbsp;&nbsp;&nbsp;</th>
         * <th> %Level </th>
         * </tr><tr>
         * <td> QtDebugMsg </td>
         * <td> Level::DEBUG_INT </td>
         * </tr><tr bgcolor="#ffffff">
         * <td> QtWarningMsg </td>
         * <td> Level::WARN_INT </td>
         * </tr><tr>
         * <td> QtCriticalMsg </td>
         * <td> Level::ERROR_INT </td>
         * </tr><tr bgcolor="#ffffff">
         * <td> QtFatalMsg </td>
         * <td> Level::FATAL_INT </td>
         * </tr><tr>
         * <td> QtSystemMsg </td>
         * <td> Level::TRACE_INT </td>
         * </tr>
         * </table>
     *
     * The default value is false for not handling Qt messages.
     *
     * \sa handleQtMessages(), qInstallMsgHandler(), qFatal()
     */
    static void setHandleQtMessages(bool handleQtMessages)
    {
        instance()->doSetHandleQtMessages(handleQtMessages);
    }

    /*!
     * Enables/disables watching of the current properties file
     *
     * The default value is false for not watching the properties file.
     *
     * \sa watchThisFile()
     */
    static void setWatchThisFile(bool watchThisFile)
    {
        instance()->doSetWatchThisFile(watchThisFile);
    }

    /*!
     * Set a message pattern for qc[Debug|Info|Warn|Critical]
     *
     * \sa messagePattern()
     */
    static void setMessagePattern(const QString &pattern)
    {
        instance()->doSetMessagePattern(pattern);
    }

    /*!
     * Set the filter rules for qc[Debug|Info|Warn|Critical]
     *
     * \sa filterRules()
     */
    static void setFilterRules(const QString &rules)
    {
        instance()->doSetFilterRules(rules);
    }

    /*!
     * Configures the logging for the package to its default behaviour.
     *
     * The logger logLogger() is configured to be not additive. Messages
     * with the level Level::ERROR_INT and Level::FATAL_INT are written
     * to \c stderr using a ConsoleAppender. The remaining messages are
     * written to \c stdout using a second ConsoleAppender. The level is
     * read from the system environment or application settings using
     * InitialisationHelper::setting() with the key \c Debug. If a level
     * value is found, but it is not a valid Level string,
     * Level::DEBUG_INT is used. If no level string is found
     * Level::ERROR_INT is used.
     *
     * The function does not remove any appender from the logger
     * logLogger().
     *
     * \sa \ref LogLog "Logging within the package",
     *     \ref Env "Environment Variables",
     *     resetConfiguration(), InitialisationHelper::setting()
     */
    static void configureLogLogger()
    {
        instance()->doConfigureLogLogger();
    }

    [[nodiscard]] static bool exists(const char *pName);

    /*!
     * Returns the LogManager instance.
     */
    [[nodiscard]] static LogManager *instance();

    [[nodiscard]] static Logger *logger(const QString &name);

    /*!
     * Reset all values contained in logger repository to their default.
     *
     * All appenders are removed from all loggers. The loggers are handled
     * in no particular order. The last loggers to be reset are qtLogger(),
     * logLogger() and rootLogger() in that order.
     *
     * The handling of messages created by calls to qDebug(), qWarning(),
     * qCritical() and qFatal() is deactivated.
     *
     * The internal logging is initialised to its default bahaviour
     * using configureLogLogger().
    *
    * \sa LoggerRepository::resetConfiguration(), setHandleQtMessages(),
    *     configureLogLogger()
     */
    static void resetConfiguration();

    static void shutdown();

    /*!
     * Executes the default initialisation procedure of the package.
     *
     * The function will test for the setting \c DefaultInitOverride in
     * the system environment and application settings using
     * \ref InitialisationHelper::setting(). If the value is present and
     * set to anything else then \c false, the initialisation is aborted.
     * <br>
     * The system environment and application settings are tested for the
     * setting \c Configuration. If it is found and it is a valid path to
     * a file, the package is configured with the file using
     * \ref PropertyConfigurator::doConfigure(const QString &, LoggerRepository *)
     * "PropertyConfigurator::doConfigure()". If the setting
     * \c Configuration is not available and a QCoreApplication object is
     * present, the application settings are tested for a group
     * \c Properties. If the group exists, the package is configured
     * with the setting using the
     * \ref PropertyConfigurator::doConfigure(const QSettings &properties, LoggerRepository *)
     * "PropertyConfiguratordoConfigure()". If neither a configuration
     * file nor configuration settings could be found, the current working
     * directory is searched for the file \c "log4qt.properties". If it is
     * found, the package is configured with the file using
     * \ref PropertyConfigurator::doConfigure(const QString &, LoggerRepository *)
     * "PropertyConfigurator::doConfigure()".
     *
     * \sa \ref Init "Initialization procedure",
     *     \ref Env "Environment Variables",
     *     InitialisationHelper::setting()
     */
    static void startup()
    {
        instance()->doStartup();
    }

    /*!
     * Returns the version number of Log4Qt at run-time. This may be a
     * different version than the version the application was compiled
     * against.
     *
     * \sa \ref Log4Qt::LOG4QT_VERSION "LOG4QT_VERSION",
     *     \ref Log4Qt::LOG4QT_VERSION_STR "LOG4QT_VERSION_STR"

     */
    [[nodiscard]] static const char *version();
    [[nodiscard]] static QVersionNumber versionNumber();

private:
    void doSetHandleQtMessages(bool handleQtMessages);
    void doSetWatchThisFile(bool watchThisFile);
    void doSetFilterRules(const QString &filterRules);
    void doSetMessagePattern(const QString &messagePattern);
    void doConfigureLogLogger();
    void doStartup();
    void welcome();

    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);

private:
    mutable QRecursiveMutex mObjectGuard;

    LoggerRepository *mLoggerRepository;
    std::atomic<bool> mHandleQtMessages;
    std::atomic<bool> mWatchThisFile;
    QString mFilterRules, mMessagePattern;
    QtMessageHandler mQtMsgHandler;
    static std::atomic<LogManager *> mInstance;
};

} // namespace Log4Qt

#endif // LOG4QT_LOGMANAGER_H
