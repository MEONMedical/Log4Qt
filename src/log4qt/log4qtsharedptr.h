#ifndef LOG4QT_LOG4QTSHAREDPTR_H
#define LOG4QT_LOG4QTSHAREDPTR_H

#include <QSharedPointer>
#include <QObject>

#include <type_traits>

namespace Log4Qt
{

template<typename Log4QtClass>
class Log4QtSharedPtr : public QSharedPointer<Log4QtClass>
{
public:
    Log4QtSharedPtr(Log4QtClass *ptr)
        : QSharedPointer<Log4QtClass>(ptr, &Log4QtClass::deleteLater)
    {
        static_assert(std::is_base_of<QObject, Log4QtClass>::value, "Need a QObject derived class here");
    }

    Log4QtSharedPtr()
        : QSharedPointer<Log4QtClass>()
    {
    }

    Log4QtSharedPtr(const QSharedPointer<Log4QtClass> &other)
        : QSharedPointer<Log4QtClass>(other)
    {
    }

    Log4QtSharedPtr(const QWeakPointer<Log4QtClass> &other)
        : QSharedPointer<Log4QtClass>(other)
    {
    }
};

}

#endif // LOG4QT_LOG4QTSHAREDPTR_H
