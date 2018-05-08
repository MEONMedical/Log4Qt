/******************************************************************************
 *
 * package:     Log4Qt
 * file:        telnetappender.h
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

#ifndef LOG4QT_TELNETAPPENDER_H
#define LOG4QT_TELNETAPPENDER_H

#include "appenderskeleton.h"

#include <QString>
#include <QHostAddress>

class QTcpServer;
class QTcpSocket;

namespace Log4Qt
{

/*!
 * \brief The class TelnetAppender appends log events to a read-only socket (telnet)
 *T
 * \note All the functions declared in this class are thread-safe.
 * &nbsp;
 * \note The ownership and lifetime of objects of this class are managed.
 *       See \ref Ownership "Object ownership" for more details.
 */
class LOG4QT_EXPORT  TelnetAppender : public AppenderSkeleton
{
    Q_OBJECT

    /*!
     * The property holds the port used by the telenet appender.
     *
     * The default is 23 for the port.
     *
     * \sa port, port(), setPort()
     */
    Q_PROPERTY(int port READ port WRITE setPort)

    /*!
     * The property holds, if the writer flushes after all write operations.
     *
     * The default is false for flushing.
     *
     * \sa immediateFlush(), setImmediateFlush()
     */
    Q_PROPERTY(bool immediateFlush READ immediateFlush WRITE setImmediateFlush)

    Q_PROPERTY(QHostAddress address READ address WRITE setAddress)

public:
    TelnetAppender(QObject *parent = nullptr);
    TelnetAppender(const LayoutSharedPtr &layout,
                   QObject *parent = nullptr);
    TelnetAppender(const LayoutSharedPtr &layout,
                   const QHostAddress &address,
                   int port,
                   QObject *parent = nullptr);
    TelnetAppender(const LayoutSharedPtr &layout,
                   int port,
                   QObject *parent = nullptr);
    ~TelnetAppender() override;

private:
    Q_DISABLE_COPY(TelnetAppender)

public:
    bool requiresLayout() const override;
    void activateOptions() override;
    void close() override;

    /*!
     * Sets the listening port of the telnet server (default = 23)
     */
    void setPort(int port);
    /*!
     * Returns the listening port of the telnet server
     */
    int  port() const;
    /*!
     * Set the property immediate flush (default: false)
     */
    void setImmediateFlush(bool immediateFlush);
    /*!
     *  Returns <true> immediate flush is enabled
     */
    bool immediateFlush() const;

    /*!
     * Sets the listenning address of the telnet server (default QHostAddress::Any)
     */
    void setAddress(const QHostAddress &address);
    /*!
     * Returns the listenning address of the telnet server
     */
    QHostAddress address() const;
    /*!
     *  Set the welcome message which is send on
     */
    void setWelcomeMessage(const QString &welcomeMessage);

protected:
    void append(const LoggingEvent &event) override;

    /*!
     * Tests if all entry conditions for using append() in this class are
     * met.
     *
     * If a conditions is not met, an error is logged and the function
     * returns false. Otherwise the result of
     * AppenderSkeleton::checkEntryConditions() is returned.
     *
     * The checked conditions are:
     * - A writer has been set (APPENDER_USE_MISSING_WRITER_ERROR)
     *
     * The function is called as part of the checkEntryConditions() chain
     * started by AppenderSkeleton::doAppend().
     *
     * \sa AppenderSkeleton::doAppend(),
     *     AppenderSkeleton::checkEntryConditions()
     */
    bool checkEntryConditions() const override;

    /*!
     *  Creates and starts (listening) the TCP server
     */
    void openServer();
    /*!
     *  Stops and destroys the TCP server
     */
    void closeServer();

    QList<QTcpSocket *> clients() const;

private slots:
    /*!
     *  Handles a new incomming connection
     */
    void onNewConnection();
    /*!
     *  Handles a client disconnect
     */
    void onClientDisconnected();

private:
    QHostAddress mAddress;
    int             mPort;
    QTcpServer     *mTcpServer;
    QList<QTcpSocket *> mTcpSockets;
    QString         mWelcomeMessage;
    volatile bool   mImmediateFlush;

    void sendWelcomeMessage(QTcpSocket *clientConnection);
};

inline bool TelnetAppender::immediateFlush() const
{
    return mImmediateFlush;
}

inline void TelnetAppender::setImmediateFlush(bool immediateFlush)
{
    mImmediateFlush = immediateFlush;
}

} // namespace Log4Qt

#endif // LOG4QT_TELNETAPPENDER_H
