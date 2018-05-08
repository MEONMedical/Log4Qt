/******************************************************************************
 *
 * package:     Log4Qt
 * file:        logger.h
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

#ifndef LOG4QT_LOGGER_H
#define LOG4QT_LOGGER_H

#include <QObject>

#include "helpers/logerror.h"
#include "helpers/classlogger.h"
#include "helpers/appenderattachable.h"
#include "level.h"
#include "logstream.h"
#include "loggingevent.h"

namespace Log4Qt
{

/*!
 * LOG4QT_DECLARE_STATIC_LOGGER declares a static function \a FUNCTION that
 * returns a pointer to a \ref Log4Qt::Logger "Logger" named after \a CLASS.
 *
 * On the first invocation the \ref Log4Qt::Logger "Logger" is requested
 * by calling \ref Log4Qt::Logger::logger(const char *pName)
 * "Logger::logger( #CLASS )". The pointer is stored to be returned on
 * subsequent invocations.
 *
 * The following example shows how to use the macro to define a logger to be
 * used within a class not derived from QObject.
 *
 * \code
 * #file: counter.h
 *
 * #include logger.h
 *
 * class Counter
 * {
 * public:
 *     Counter();
 *     Counter(int preset);
 * private:
 *     int mCount;
 * }
 * \endcode
 * \code
 * #file: counter.cpp
 *
 * #include counter.h
 *
 * LOG4QT_DECLARE_STATIC_LOGGER(logger, Counter)
 *
 * Counter::Counter() :
 *     mCount(0)
 * {}
 *
 * void Counter::Counter(int preset) :
 *     mCount(preset)
 * {
 *     if (preset < 0)
 *     {
 *         logger()->warn("Invalid negative counter preset %1. Using 0 instead.", preset);
 *         mCount = 0;
 *     }
 * }
 * \endcode
 *
 * \note The function created by the macro is thread-safe.
 *
 * \sa \ref Log4Qt::Logger::logger(const char *pName) "Logger::logger(const char *pName)"
 */
#define LOG4QT_DECLARE_STATIC_LOGGER(FUNCTION, CLASS)                     \
        static Log4Qt::Logger *FUNCTION()                                 \
        {                                                                 \
            static Log4Qt::Logger * p_logger(Log4Qt::Logger::logger(#CLASS )); \
            return p_logger;                                               \
        }

/*!
 * LOG4QT_DECLARE_QCLASS_LOGGER declares member functions to retrieve
 * \ref Log4Qt::Logger "Logger" for the class it is used in.
 *
 * On the first invocation the \ref Log4Qt::Logger "Logger" is requested
 * by a call to \ref Log4Qt::Logger::logger(const char *pName)
 * "Logger::logger(const char *pName)". The pointer is stored to be
 * returned on subsequent invocations.
 *
 * The following example shows how to use the macro to define a logger to be
 * used within a class derived from QObject.
 *
 * \code
 * #file: counter.h
 *
 * #include qobject.h
 * #include logger.h
 *
 * class Counter : public QObject
 * {
 *     Q_OBJECT
 *     LOG4QT_DECLARE_QCLASS_LOGGER
 * public:
 *     Counter();
 *     Counter(int preset);
 * private:
 *     int mCount;
 * }
 * \endcode
 * \code
 * #file: counter.cpp
 *
 * #include counter.h
 *
 * Counter::Counter() :
 *     mCount(0)
 * {}
 *
 * void Counter::Counter(int preset)
 *     mCount(preset)
 * {
 *     if (preset < 0)
 *     {
 *         logger()->warn("Invalid negative counter preset %1. Using 0 instead.", preset);
 *         mCount = 0;
 *     }
 * }
 * \endcode
 *
 * \note The function created by the macro is thread-safe.
 *
 * \sa \ref Log4Qt::Logger::logger(const char *pName) "Logger::logger(const char *pName)",
 *     \ref Log4Qt::ClassLogger "ClassLogger"
 */
#define LOG4QT_DECLARE_QCLASS_LOGGER                                      \
            private:                                                              \
                    mutable Log4Qt::ClassLogger mLog4QtClassLogger;                   \
            public:                                                               \
                    inline Log4Qt::Logger *logger() const                             \
                    {   return mLog4QtClassLogger.logger(this);    }                  \
            private:


class LOG4QT_EXPORT MessageLogger
{
    Q_DISABLE_COPY(MessageLogger)

public:
    explicit MessageLogger(Logger *logger, Level level) : mLogger(logger), mLevel(level) {}
    explicit MessageLogger(Logger *logger, Level level, const char *file, int line, const char *function)
        : mLogger(logger), mLevel(level), mContext(file, line, function) {}

    void log(const QString &message) const;
    template <typename T, typename ...Ts>
    void log(const QString &message, T &&t, Ts &&...ts) const
    {
        log(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }
    LogStream log() const;

private:
    QPointer<const Logger> mLogger;
    Level mLevel;
    MessageContext mContext;
};

// Macros to log with location information, teh logger must have the name
#define l4qFatal(...) \
    for (bool enabled = logger()->isEnabledFor(Log4Qt::Level::FATAL_INT); enabled; enabled = false) \
        Log4Qt::MessageLogger(logger(), Log4Qt::Level::FATAL_INT, __FILE__, __LINE__, Q_FUNC_INFO).log(__VA_ARGS__)
#define l4qError(...) \
    for (bool enabled = logger()->isEnabledFor(Log4Qt::Level::ERROR_INT); enabled; enabled = false) \
        Log4Qt::MessageLogger(logger(), Log4Qt::Level::ERROR_INT, __FILE__, __LINE__, Q_FUNC_INFO).log(__VA_ARGS__)
#define l4qWarn(...) \
    for (bool enabled = logger()->isEnabledFor(Log4Qt::Level::WARN_INT); enabled; enabled = false) \
        Log4Qt::MessageLogger(logger(), Log4Qt::Level::WARN_INT, __FILE__, __LINE__, Q_FUNC_INFO).log(__VA_ARGS__)
#define l4qInfo(...) \
    for (bool enabled = logger()->isEnabledFor(Log4Qt::Level::INFO_INT); enabled; enabled = false) \
        Log4Qt::MessageLogger(logger(), Log4Qt::Level::INFO_INT, __FILE__, __LINE__, Q_FUNC_INFO).log(__VA_ARGS__)
#define l4qDebug(...) \
    for (bool enabled = logger()->isEnabledFor(Log4Qt::Level::DEBUG_INT); enabled; enabled = false) \
        Log4Qt::MessageLogger(logger(), Log4Qt::Level::DEBUG_INT, __FILE__, __LINE__, Q_FUNC_INFO).log(__VA_ARGS__)
#define l4qTrace(...) \
    for (bool enabled = logger()->isEnabledFor(Log4Qt::Level::TRACE_INT); enabled; enabled = false) \
        Log4Qt::MessageLogger(logger(), Log4Qt::Level::TRACE_INT, __FILE__, __LINE__, Q_FUNC_INFO).log(__VA_ARGS__)

class Appender;
class LoggerRepository;

/*!
 * \brief The class Logger provides logging services.
 *
 * A pointer to a logger can be retrieved by calling Logger::logger() or
 * LogManager::logger() with the class name as argument. Because a logger
 * is never destroyed it is possible to store the pointer to the logger.
 * This way the lookup of the pointer in the repository is only required
 * on the first logging operation. The macros \ref
 * Log4Qt::LOG4QT_DECLARE_STATIC_LOGGER "LOG4QT_DECLARE_STATIC_LOGGER" and
 * \ref Log4Qt::LOG4QT_DECLARE_QCLASS_LOGGER "LOG4QT_DECLARE_QCLASS_LOGGER"
 * provide a thread-safe implementation to store the logger pointer.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class LOG4QT_EXPORT Logger : public QObject, public AppenderAttachable
{
    Q_OBJECT

    /*!
     * The property holds, if the logger is additive.
     *
     * The default is true for being additive.
     *
     * \sa additive(), setAdditive()
     */
    Q_PROPERTY(bool additivity READ additivity WRITE setAdditivity)

    /*!
     * The property holds the level used by the logger.
     *
     * The default is Level::NULL_INT.
     * \sa level(), setLevel()
     */
    Q_PROPERTY(Log4Qt::Level level READ level WRITE setLevel)

    /*!
     * The property holds the LoggerRepository of the logger.
     *
     * \sa loggerRepository()
     */
    Q_PROPERTY(LoggerRepository *loggerRepository READ loggerRepository)

    /*!
     * The property holds the name of the logger.
     *
     * \sa name()
     */
    Q_PROPERTY(QString name READ name)

    /*!
     * The property holds the parent logger of the logger.
     *
     * \sa parentLogger()
     */
    Q_PROPERTY(Logger *parentLogger READ parentLogger)

    LOG4QT_DECLARE_QCLASS_LOGGER

protected:
    Logger(LoggerRepository *loggerRepository, Level level, const QString &name, Logger *parent = nullptr);
    ~Logger() override;

private:
    Q_DISABLE_COPY(Logger)

public:
    bool additivity() const;
    Level level() const;
    LoggerRepository *loggerRepository() const;
    QString name() const;
    Logger *parentLogger() const;

    void setAdditivity(bool additivity);
    virtual void setLevel(Level level);

    void callAppenders(const LoggingEvent &event) const;

    Level effectiveLevel() const;
    bool isDebugEnabled() const;

    /*!
     * Checks if this logger is enabled for a given Level \a level. If the
     * logger is enabled the function returns true. Otherwise it returns
     * false.
     *
     * A logger is enabled for a level, if the level is greater or equal
     * then the repository threshold and greater and equal then the loggers
     * effective level.
     *
     * \sa LoggerRepository::isDisabled(), effectiveLevel()
     */
    bool isEnabledFor(Level level) const;

    bool isErrorEnabled() const;
    bool isFatalEnabled() const;
    bool isInfoEnabled() const;
    bool isTraceEnabled() const;
    bool isWarnEnabled() const;

    LogStream debug() const;
    void debug(const LogError &logError) const;
    void debug(const QString &message) const;

    template<typename T, typename ...Ts>
    void debug(const QString &message, T &&t, Ts &&...ts)
    {
        debug(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }


    LogStream error() const;
    void error(const LogError &logError) const;
    void error(const QString &message) const;

    template<typename T, typename ...Ts>
    void error(const QString &message, T &&t, Ts &&...ts)
    {
        error(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    LogStream fatal() const;
    void fatal(const LogError &logError) const;
    void fatal(const QString &message) const;

    template<typename T, typename ...Ts>
    void fatal(const QString &message, T &&t, Ts &&...ts)
    {
        fatal(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    LogStream info() const;
    void info(const LogError &logError) const;
    void info(const QString &message) const;

    template<typename T, typename ...Ts>
    void info(const QString &message, T &&t, Ts &&...ts)
    {
        info(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    LogStream log(Level level) const;
    void log(Level level, const LogError &logError) const;
    void log(const LoggingEvent &logEvent) const;

    void log(Level level, const QString &message) const;
    template<typename T, typename ...Ts>
    void log(Level level, const QString &message, T &&t, Ts &&...ts)
    {
        log(level, message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    void logWithLocation(Level level, const char *file, int line, const char *function, const QString &message) const;
    template<typename T, typename ...Ts>
    void logWithLocation(Level level, const char *file, int line, const char *function, const QString &message, T &&t, Ts &&...ts)
    {
        logWithLocation(level, file, line, function, message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    LogStream trace() const;
    void trace(const LogError &logError) const;
    void trace(const QString &message) const;

    template<typename T, typename ...Ts>
    void trace(const QString &message, T &&t, Ts &&...ts)
    {
        trace(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    LogStream warn() const;
    void warn(const LogError &logError) const;
    void warn(const QString &message) const;

    template<typename T, typename ...Ts>
    void warn(const QString &message, T &&t, Ts &&...ts)
    {
        warn(message.arg(std::forward<T>(t)), std::forward<Ts>(ts)...);
    }

    // LogManager operations
    static Logger *logger(const QString &name);
    static Logger *logger(const char *name);
    static Logger *rootLogger();

protected:
    void forcedLog(Level level, const QString &message) const;
    void forcedLog(const LoggingEvent &logEvent) const;

private:
    const QString mName;
    LoggerRepository *mLoggerRepository;
    volatile bool mAdditivity;
    Level mLevel;
    Logger *mParentLogger;

    // Needs to be friend to create Logger objects
    friend class Hierarchy;
};

} // namespace Log4Qt

#endif // LOG4QT_LOGGER_H
