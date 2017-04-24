#include "loggerstatic.h"

#include "log4qt/logger.h"

LOG4QT_DECLARE_STATIC_LOGGER(logger, LoggerStatic)

LoggerStatic::LoggerStatic()
{
    logger()->trace() << "ctor Debug output";
}

LoggerStatic::~LoggerStatic()
{
    logger()->trace() << "dtor Debug output";
}
