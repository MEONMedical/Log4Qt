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

/******************************************************************************
 * Dependencies
 ******************************************************************************/

#include "telnetappender.h"

#include <QtCore/QDebug>

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

#include "layout.h"
#include "loggingevent.h"

namespace Log4Qt {
  /**************************************************************************
   * Declarations
   **************************************************************************/

  /**************************************************************************
   * C helper functions
   **************************************************************************/

  /**************************************************************************
   * Class implementation: TelnetAppender
   **************************************************************************/

  TelnetAppender::TelnetAppender(QObject *pParent) :
    AppenderSkeleton(false, pParent), mAddress(QHostAddress::Any), mPort(23),
        mpTcpServer(0), mImmediateFlush(false)
  {
  }

  TelnetAppender::TelnetAppender(Layout *pLayout, QObject *pParent) :
    AppenderSkeleton(false, pParent), mAddress(QHostAddress::Any), mPort(23),
        mpTcpServer(0), mImmediateFlush(false)
  {
    setLayout(pLayout);
  }

  TelnetAppender::TelnetAppender(Layout *pLayout, int port, QObject *pParent) :
    AppenderSkeleton(false, pParent), mAddress(QHostAddress::Any), mPort(port),
        mpTcpServer(0), mImmediateFlush(false)
  {
    setLayout(pLayout);
  }

  TelnetAppender::TelnetAppender(Layout *pLayout, const QHostAddress& address,
    int port, QObject *pParent) :
    AppenderSkeleton(false, pParent), mAddress(address), mPort(port),
        mpTcpServer(0), mImmediateFlush(false)
  {
    setLayout(pLayout);
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

  void TelnetAppender::setAddress(const QHostAddress& address)
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

  void TelnetAppender::setWelcomeMessage(const QString & welcomeMessage)
  {
    mWelcomeMessage = welcomeMessage;
  }

  bool TelnetAppender::requiresLayout() const
  {
    return true;
  }

  void TelnetAppender::append(const LoggingEvent &rEvent)
  {
    // Q_ASSERT_X(, "TelnetAppender::append()", "Lock must be held by caller");
    Q_ASSERT_X(layout(), "TelnetAppender::append()", "Layout must not be null");

    QString message(layout()->format(rEvent));

    Q_FOREACH (QTcpSocket * pClientConnection, mTcpSockets)
    {
        pClientConnection->write(message.toLocal8Bit().constData());
        if (immediateFlush())
            pClientConnection->flush();
    }
  }

  bool TelnetAppender::checkEntryConditions() const
  {
    // Q_ASSERT_X(, "TelnetAppender::checkEntryConditions()", "Lock must be held by caller")

    if (!mpTcpServer && !mpTcpServer->isListening()) {
      LogError
          e =
              LOG4QT_QCLASS_ERROR(QT_TR_NOOP("Use of appender '%1' without a listing telnet server"),
                  APPENDER_TELNET_SERVER_NOT_RUNNING);
      e << name();
      logger()->error(e);
      return false;
    }

    return AppenderSkeleton::checkEntryConditions();
  }

  void TelnetAppender::openServer()
  {
    mpTcpServer = new QTcpServer(this);
    connect(mpTcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    mpTcpServer->listen(mAddress, mPort);
  }

  void TelnetAppender::closeServer()
  {
    if (mpTcpServer)
      mpTcpServer->close();

    Q_FOREACH(QTcpSocket * pClientConnection, mTcpSockets)
        delete pClientConnection;

    mTcpSockets.clear();

    delete mpTcpServer;
    mpTcpServer = 0;
  }

  QList<QTcpSocket*> TelnetAppender::clients() const
  {
    return mTcpSockets;
  }

#ifndef QT_NO_DEBUG_STREAM
  QDebug TelnetAppender::debug(QDebug &rDebug) const
  {
    QString layout_name;
    if (layout())
      layout_name = layout()->name();

    rDebug.nospace() << "TelnetAppender(" << "name:" << name() << " "
        << "filter:" << firstFilter() << "isactive:" << isActive()
        << "isclosed:" << isClosed() << "layout:" << layout_name
        << "referencecount:" << referenceCount() << " " << "threshold:"
        << threshold().toString() << "address:" << address() << "port:"
        << port() << " " << ")";
    return rDebug.space();
  }
#endif // QT_NO_DEBUG_STREAM
  bool TelnetAppender::handleIoErrors() const
  {
    // Q_ASSERT_X(, "FileAppender::handleIoErrors()", "Lock must be held by caller")
    return false;
  }

  void TelnetAppender::onNewConnection()
  {
    QMutexLocker locker(&mObjectGuard);

    if (mpTcpServer && mpTcpServer->hasPendingConnections()) {
      QTcpSocket * pClientConnection = mpTcpServer->nextPendingConnection();
      if (pClientConnection) {
        mTcpSockets.append(pClientConnection);
        connect(pClientConnection, SIGNAL(disconnected()), this,
          SLOT(onClientDisconnected()));
        sendWelcomeMessage(pClientConnection);
      }
    }
  }

  void TelnetAppender::sendWelcomeMessage(QTcpSocket * pClientConnection)
  {
    if (mWelcomeMessage.isEmpty())
      return;

    pClientConnection->write(mWelcomeMessage.toLocal8Bit().constData());
  }

  void TelnetAppender::onClientDisconnected()
  {
    QMutexLocker locker(&mObjectGuard);

    QTcpSocket* pClientConnection = qobject_cast<QTcpSocket*> (sender());
    if (pClientConnection) {
      mTcpSockets.removeOne(pClientConnection);
      pClientConnection->deleteLater();
    }
  }

/******************************************************************************
 * Implementation: Operators, Helper
 ******************************************************************************/

} // namespace Log4Qt
