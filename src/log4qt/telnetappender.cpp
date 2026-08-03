/******************************************************************************
 *
 * package:     Log4Qt
 * file:        telnetappender.cpp
 * created:     July 2010
 * author:      Andreas Bacher
 *
 *
 * Copyright 2010 Andreas Bacher
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

#include "telnetappender.h"

#include "abstractlayout.h"
#include "loggingevent.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

namespace Log4Qt
{

TelnetAppender::TelnetAppender(QObject *parent)
    : AppenderSkeleton(false, parent)
    , mAddress(QHostAddress::Any)
    , mPort(23)
    , mTcpServer(nullptr)
    , mImmediateFlush(false)
{
}

TelnetAppender::TelnetAppender(const LayoutSharedPtr &layout, QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , mAddress(QHostAddress::Any)
    , mPort(23)
    , mTcpServer(nullptr)
    , mImmediateFlush(false)
{
}

TelnetAppender::TelnetAppender(const LayoutSharedPtr &layout, int port, QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , mAddress(QHostAddress::Any)
    , mPort(port)
    , mTcpServer(nullptr)
    , mImmediateFlush(false)
{
}

TelnetAppender::TelnetAppender(const LayoutSharedPtr &layout, const QHostAddress &address,
                               int port, QObject *parent)
    : AppenderSkeleton(false, layout, parent)
    , mAddress(address)
    , mPort(port)
    , mTcpServer(nullptr)
    , mImmediateFlush(false)
{
}

TelnetAppender::~TelnetAppender()
{
    close();
}

void TelnetAppender::activateOptions()
{
    QMutexLocker locker(&mObjectGuard);

    closeServer();
    openServer();

    AppenderSkeleton::activateOptions();
}

void TelnetAppender::close()
{
    QMutexLocker locker(&mObjectGuard);

    if (isClosed())
        return;

    AppenderSkeleton::close();
    closeServer();
}

void TelnetAppender::setAddress(const QHostAddress &address)
{
    mAddress = address;
}

QHostAddress TelnetAppender::address() const
{
    return mAddress;
}

void TelnetAppender::setPort(int port)
{
    if (port < 1 || port > 65535)
    {
        LogError e = LOG4QT_QCLASS_ERROR("Invalid port %1 for appender '%2'; valid range is 1..65535",
                                         AppenderTelnetServerNotRunning);
        e << port << name();
        logger()->error(e);
        return;
    }
    mPort = port;
}

int TelnetAppender::port() const
{
    return mPort;
}

void TelnetAppender::setWelcomeMessage(const QString &welcomeMessage)
{
    mWelcomeMessage = welcomeMessage;
}

bool TelnetAppender::requiresLayout() const
{
    return true;
}

void TelnetAppender::append(const LoggingEvent &event)
{
    const LayoutSharedPtr &layoutSnap = layoutSnapshot();
    if (!layoutSnap)
        return;

    const QByteArray bytes = layoutSnap->format(event).toLocal8Bit();
    const bool flushAfterWrite = immediateFlush();

    // Iterate over a copy: when the logging call happens on the appender's
    // own thread, the lambda below runs synchronously, and a failed write
    // aborts the socket. abort() emits disconnected() synchronously, so
    // onClientDisconnected() re-enters through the recursive mutex and
    // removes the socket from mTcpSockets — mutating the list here would
    // invalidate the iterators of a range-for over the member itself.
    const QList<QTcpSocket *> sockets = mTcpSockets;
    for (auto *clientConnection : sockets)
    {
        // QTcpSocket must be touched from its owner thread. Marshal the write
        // across via AutoConnection: same-thread → direct, otherwise queued.
        QMetaObject::invokeMethod(clientConnection,
            [clientConnection, bytes, flushAfterWrite]() {
                if (clientConnection->write(bytes) == -1)
                {
                    // Stream broke; abort drops the client and triggers
                    // onClientDisconnected, which removes it from mTcpSockets.
                    clientConnection->abort();
                    return;
                }
                if (flushAfterWrite)
                    clientConnection->flush();
            },
            Qt::AutoConnection);
    }
}

bool TelnetAppender::checkEntryConditions() const
{
    if ((mTcpServer == nullptr) || !mTcpServer->isListening())
    {
        LogError e = LOG4QT_QCLASS_ERROR("Use of appender '%1' without a listening telnet server",
                                AppenderTelnetServerNotRunning);
        e << name();
        logger()->error(e);
        return false;
    }

    return AppenderSkeleton::checkEntryConditions();
}

void TelnetAppender::openServer()
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &TelnetAppender::onNewConnection);
    if (!mTcpServer->listen(mAddress, mPort))
    {
        LogError e = LOG4QT_QCLASS_ERROR("Telnet appender '%1' failed to listen on %2:%3",
                                         AppenderTelnetServerNotRunning);
        e << name() << mAddress.toString() << mPort;
        e.addCausingError(LogError(mTcpServer->errorString(),
                                   static_cast<int>(mTcpServer->serverError())));
        logger()->error(e);
    }
}

void TelnetAppender::closeServer()
{
    for (auto &&clientConnection : mTcpSockets)
    {
        // Stop further signals to this appender and let the event loop reclaim
        // the socket; raw delete can crash if the network layer is mid-callback.
        clientConnection->disconnect(this);
        clientConnection->abort();
        clientConnection->deleteLater();
    }

    mTcpSockets.clear();

    if (mTcpServer != nullptr)
    {
        mTcpServer->close();
        mTcpServer->deleteLater();
        mTcpServer = nullptr;
    }
}

QList<QTcpSocket *> TelnetAppender::clients() const
{
    return mTcpSockets;
}

void TelnetAppender::onNewConnection()
{
    QMutexLocker locker(&mObjectGuard);

    if ((mTcpServer != nullptr) && mTcpServer->hasPendingConnections())
    {
        if (QTcpSocket *clientConnection = mTcpServer->nextPendingConnection(); clientConnection != nullptr)
        {
            mTcpSockets.append(clientConnection);
            connect(clientConnection, &QTcpSocket::disconnected,
                    this, &TelnetAppender::onClientDisconnected);
            sendWelcomeMessage(clientConnection);
        }
    }
}

void TelnetAppender::sendWelcomeMessage(QTcpSocket *clientConnection)
{
    if (mWelcomeMessage.isEmpty())
        return;

    if (clientConnection->write(mWelcomeMessage.toLocal8Bit()) == -1)
        clientConnection->abort();
}

void TelnetAppender::onClientDisconnected()
{
    QMutexLocker locker(&mObjectGuard);

    QTcpSocket *clientConnection = qobject_cast<QTcpSocket *> (sender());
    if (clientConnection != nullptr)
    {
        mTcpSockets.removeOne(clientConnection);
        clientConnection->deleteLater();
    }
}

} // namespace Log4Qt

#include "moc_telnetappender.cpp"
