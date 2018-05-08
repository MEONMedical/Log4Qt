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

#include "layout.h"
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
    Q_ASSERT_X(layout(), "TelnetAppender::append()", "Layout must not be null");

    QString message(layout()->format(event));
    for (auto &&clientConnection : qAsConst(mTcpSockets))
    {
        clientConnection->write(message.toLocal8Bit().constData());
        if (immediateFlush())
            clientConnection->flush();
    }
}

bool TelnetAppender::checkEntryConditions() const
{
    if ((mTcpServer == nullptr) || !mTcpServer->isListening())
    {
        LogError e = LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without a listing telnet server"),
                                APPENDER_TELNET_SERVER_NOT_RUNNING);
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
    mTcpServer->listen(mAddress, mPort);
}

void TelnetAppender::closeServer()
{
    if (mTcpServer != nullptr)
        mTcpServer->close();

    for (auto &&clientConnection : qAsConst(mTcpSockets))
        delete clientConnection;

    mTcpSockets.clear();

    delete mTcpServer;
    mTcpServer = nullptr;
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
        QTcpSocket *clientConnection = mTcpServer->nextPendingConnection();
        if (clientConnection != nullptr)
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

    clientConnection->write(mWelcomeMessage.toLocal8Bit().constData());
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
