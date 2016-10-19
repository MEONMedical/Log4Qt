#include "systemlogappender.h"

#include "layout.h"
#include "level.h"
#include "loggingevent.h"

#include <QCoreApplication>

#if defined(Q_OS_WIN32)
#ifndef UNICODE
#define UNICODE
#endif
#include <qt_windows.h>

#include <QLibrary>

typedef HANDLE(WINAPI *PDeregisterEventSource)(HANDLE);
static PDeregisterEventSource pDeregisterEventSource = 0;
typedef BOOL(WINAPI *PReportEvent)(HANDLE, WORD, WORD, DWORD, PSID, WORD, DWORD, LPCTSTR *, LPVOID);
static PReportEvent pReportEvent = 0;
typedef HANDLE(WINAPI *PRegisterEventSource)(LPCTSTR, LPCTSTR);
static PRegisterEventSource pRegisterEventSource = 0;

#define RESOLVE(name) p##name = reinterpret_cast<P##name>(lib.resolve(#name));
#define RESOLVEA(name) p##name = reinterpret_cast<P##name>(lib.resolve(#name"A"));
#define RESOLVEW(name) p##name = reinterpret_cast<P##name>(lib.resolve(#name"W"));

static bool winServiceInit()
{
    if (!pDeregisterEventSource)
    {
        QLibrary lib("advapi32");

        // only resolve unicode versions
        RESOLVE(DeregisterEventSource);
        RESOLVEW(ReportEvent);
        RESOLVEW(RegisterEventSource);
    }
    return pDeregisterEventSource != 0;
}
#else

#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>

static QString encodeName(const QString &name, bool allowUpper = false)
{
    QString n = name.toLower();
    QString legal = QLatin1String("abcdefghijklmnopqrstuvwxyz1234567890");
    if (allowUpper)
        legal += QLatin1String("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    int pos = 0;
    while (pos < n.size())
    {
        if (legal.indexOf(n[pos]) == -1)
            n.remove(pos, 1);
        else
            ++pos;
    }
    return n;
}
#endif //#if defined(Q_OS_WIN32)
namespace Log4Qt
{
SystemLogAppender::SystemLogAppender(QObject *parent) :
    AppenderSkeleton(parent), ident(0)
{
    setServiceName(QCoreApplication::applicationName());
}

SystemLogAppender::~SystemLogAppender()
{
    if (ident)
        delete[] ident;
}

void SystemLogAppender::append(const LoggingEvent &rEvent)
{
    QString message(layout()->format(rEvent));

#if defined(Q_OS_WIN32)
    //  Q_D(QtServiceBase);
    if (!winServiceInit())
        return;
    WORD wType;
    switch (rEvent.level().toInt())
    {
    case Level::WARN_INT:
        wType = EVENTLOG_WARNING_TYPE;
        break;
    case Level::ERROR_INT:
        wType = EVENTLOG_ERROR_TYPE;
        break;
    case Level::FATAL_INT:
        wType = EVENTLOG_ERROR_TYPE;
        break;
    case Level::OFF_INT:
        wType = EVENTLOG_SUCCESS;
        break;
    default:
        wType = EVENTLOG_INFORMATION_TYPE;
        break;
    }

    HANDLE h = pRegisterEventSource(0, serviceName().toStdWString().c_str());
    if (h)
    {
        int id = 0;
        uint category = 0;
        auto msg = message.toStdWString();
        const wchar_t *msg_wstr = msg.c_str();
        const char *bindata = 0;//data.size() ? data.constData() : 0;
        const int datasize = 0;
        pReportEvent(h, wType, category, id, 0, 1, datasize,
                     &msg_wstr, const_cast<char *> (bindata));
        pDeregisterEventSource(h);
    }
#else

    int st;
    switch (rEvent.level().toInt())
    {
    case Level::WARN_INT:
        st = LOG_WARNING;
        break;
    case Level::ERROR_INT:
        st = LOG_ERR;
        break;
    case Level::FATAL_INT:
        st = LOG_ERR;
        break;
    default:
        st = LOG_INFO;
    }

    openlog(ident, LOG_PID, LOG_DAEMON);
    for (const auto &line : message.split('\n', QString::SkipEmptyParts))
        syslog(st, "%s", line.toLocal8Bit().constData());
    closelog();

#endif //#if defined(Q_OS_WIN32)
}

QString SystemLogAppender::serviceName() const
{
    return mServiceName;
}

void SystemLogAppender::setServiceName(const QString &serviceName)
{
    mServiceName = serviceName;

#if !defined(Q_OS_WIN32)
    if (ident)
        delete[] ident;
    QString tmp = encodeName(mServiceName, true);
    int len = tmp.toLocal8Bit().size();
    ident = new char[len + 1];
    ident[len] = '\0';
    ::memcpy(ident, tmp.toLocal8Bit().constData(), len);
#endif
}

}

