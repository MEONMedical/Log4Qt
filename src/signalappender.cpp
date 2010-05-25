/******************************************************************************
 *
 * package:     Log4Qt
 * file:        signalappender.cpp
 * created:     March 2010
 * author:      Filonenko Michael
 *
 *
 * Copyright 2010 Filonenko Michael
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

#include "signalappender.h"

#include "layout.h"

#include <QtCore/QDebug>

namespace Log4Qt {
	SignalAppender::SignalAppender(QObject *parent) :
			AppenderSkeleton(parent)
	{
	}

	void SignalAppender::append(const LoggingEvent &rEvent)
	{
		QString message(layout()->format(rEvent));
		emit appended(message);
	}

#ifndef QT_NO_DEBUG_STREAM
	QDebug SignalAppender::debug(QDebug &rDebug) const
	{
		QString layout_name;
		if (layout())
			layout_name = layout()->name();

		rDebug.nospace() << "WriterAppender("
				<< "name:" << name() << " "
				<< "filter:" << firstFilter()
				<< "isactive:" << isActive()
				<< "isclosed:" << isClosed()
				<< "layout:" << layout_name
				<< "referencecount:" << referenceCount() << " "
				<< "threshold:" << threshold().toString()
				<< ")";
		return rDebug.space();
	}
#endif // QT_NO_DEBUG_STREAM

}
