/******************************************************************************
 *
 * package:     Log4Qt
 * file:        TelnetAppender.h
 * created:     September 2010
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

/******************************************************************************
 * Dependencies
 ******************************************************************************/

#include "appenderskeleton.h"

#include <QString>

/******************************************************************************
 * Declarations
 ******************************************************************************/

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

	public:
		TelnetAppender(QObject *pParent = 0);
		TelnetAppender(Layout *pLayout,
						 QObject *pParent = 0);
		TelnetAppender(Layout *pLayout,
						 int port,
						 QObject *pParent = 0);
		virtual ~TelnetAppender();

	private:
		TelnetAppender(const TelnetAppender &rOther); // Not implemented
		TelnetAppender &operator=(const TelnetAppender &rOther); // Not implemented

	public:
		virtual bool requiresLayout() const;
		virtual void activateOptions();
		virtual void close();

		/*!
		 * Sets the listening port of the telnet server
		 */
		void setPort(int port);
		/*!
		 * Returns the listening port of the telnet server
		 */
		int  getPort() const;
		/*!
		 * Set the property immediate flush (default: false)
		 */
		void setImmediateFlush(bool immediateFlush);
		/*!
		 *  Returns <true> immediate flush is enabled
		 */
		bool immediateFlush() const;

		/*!
		 *  Set the welcome message which is send on
		 */
		void setWelcomeMessage(const QString & welcomeMessage);

	protected:
		virtual void append(const LoggingEvent &rEvent);

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
		virtual bool checkEntryConditions() const;

		/*!
		 *  Creates and starts (listening) the TCP server
		 */
		void openServer();
		/*!
		 *  Stops and destroys the TCP server
		 */
		void closeServer();

#ifndef QT_NO_DEBUG_STREAM
		/*!
		 * Writes all object member variables to the given debug stream
		 * \a rDebug and returns the stream.
		 *
		 * <tt>
		 * %TelnetAppender(name:"WA" filter:"0x0"
		 *                 isactive:false isclosed:false layout:"TTCC"
		 *                 referencecount:1 threshold:"NULL port:"23")
		 * </tt>
		 * \sa QDebug, operator<<(QDebug debug, const LogObject &rLogObject	)
		 */

		virtual QDebug debug(QDebug &rDebug) const;
#endif // QT_NO_DEBUG_STREAM

		virtual bool handleIoErrors() const;

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
		int				mPort;
		QTcpServer *	mpTcpServer;
		QList<QTcpSocket*> mTcpSockets;
		QString			mWelcomeMessage;
		volatile bool	mImmediateFlush;

		void sendWelcomeMessage(QTcpSocket * pClientConnection);
	};


	/**************************************************************************
	 * Operators, Helper
	 **************************************************************************/


	/**************************************************************************
	 * Inline
	 **************************************************************************/

	inline bool TelnetAppender::immediateFlush() const
	{   // QMutexLocker locker(&mObjectGuard); // Read/Write of int is safe
		return mImmediateFlush; }

	inline void TelnetAppender::setImmediateFlush(bool immediateFlush)
	{   // QMutexLocker locker(&mObjectGuard); // Read/Write of int is safe
		mImmediateFlush = immediateFlush;   }

} // namespace Log4Qt


// Q_DECLARE_TYPEINFO(Log4Qt::TelnetAppender, Q_COMPLEX_TYPE); // Use default


#endif // LOG4QT_TELNETAPPENDER_H
