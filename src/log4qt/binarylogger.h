#ifndef LOG4QT_BINARYLOGGER_H
#define LOG4QT_BINARYLOGGER_H

#include "logger.h"
#include "binarylogstream.h"
#include "helpers/binaryclasslogger.h"

class QByteArray;

namespace Log4Qt
{

class Appender;
class BinaryLoggingEvent;
class LoggerRepository;
class Hierarchy;

#define LOG4QT_DECLARE_STATIC_BINARYLOGGER(FUNCTION, CLASS) \
    static Log4Qt::BinaryLogger *FUNCTION() \
    { \
        static Log4Qt::Logger * p_logger(Log4Qt::Logger::logger(#CLASS"@@binary@@" )); \
        return qobject_cast<Log4Qt::BinaryLogger*>(p_logger); \
    }

#define LOG4QT_DECLARE_QCLASS_BINARYLOGGER \
    private: \
        mutable Log4Qt::BinaryClassLogger mLog4QtClassLogger; \
    public: \
        inline Log4Qt::BinaryLogger *logger() const \
        { \
            return mLog4QtClassLogger.logger(this); \
        } \
    private:

class LOG4QT_EXPORT  BinaryLogger : public Logger
{
    Q_OBJECT
public:
    BinaryLogStream debug() const {return log(Level::DEBUG_INT);}
    void debug(const QByteArray &rMessage) const {return log(Level::DEBUG_INT, rMessage);}
    BinaryLogStream error() const {return log(Level::ERROR_INT);}
    void error(const QByteArray &rMessage) const {return log(Level::ERROR_INT, rMessage);}
    BinaryLogStream fatal() const {return log(Level::FATAL_INT);}
    void fatal(const QByteArray &rMessage) const {return log(Level::FATAL_INT, rMessage);}
    BinaryLogStream info() const {return log(Level::INFO_INT);}
    void info(const QByteArray &rMessage) const {return log(Level::INFO_INT, rMessage);}
    BinaryLogStream trace() const {return log(Level::TRACE_INT);}
    void trace(const QByteArray &rMessage) const {return log(Level::TRACE_INT, rMessage);}
    BinaryLogStream warn() const {return log(Level::WARN_INT);}
    void warn(const QByteArray &rMessage) const {return log(Level::WARN_INT, rMessage);}

    BinaryLogStream log(Level level) const;
    void log(Level level, const QByteArray &rMessage) const;
    void log(Level level, const QByteArray &rMessage, const QDateTime &timeStamp) const;

protected:
    BinaryLogger(LoggerRepository *pLoggerRepository, Level level, const QString &rName, Logger *pParent = Q_NULLPTR);
    virtual ~BinaryLogger();

    void forcedLog(Level level, const QByteArray &rMessage) const;

private:
    BinaryLogger(const BinaryLogger &rOther); // Not implemented
    BinaryLogger &operator=(const BinaryLogger &rOther); // Not implemented
    // Needs to be friend to create BinaryLogger objects
    friend class Hierarchy;
};

} // namespace Log4Qt

#endif // LOG4QT_BINARYLOGGER_H
