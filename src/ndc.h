/******************************************************************************
 *
 * package:     Log4Qt
 * file:        ndc.h
 * created:     September 2007
 * author:      Martin Heinrich
 *
 *
 * Copyright 2007 Martin Heinrich
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

#ifndef LOG4QT_NDC_H
#define LOG4QT_NDC_H

#include <QString>
#include <QStack>
#include <QThreadStorage>
#include "log4qt.h"

namespace Log4Qt
{

/*!
 * \brief The class NDC implements a nested diagnostic context.
 *
 * The method remove() is not required. QThreadStorage cleans up on thread
 * exit.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class  LOG4QT_EXPORT NDC
{
private:
    NDC();
    Q_DISABLE_COPY(NDC)

public:
    static void clear();
    static int depth();

    /*!
         * Returns the NDC instance.
         */
    static NDC *instance();

    static QString pop();
    static void push(const QString &rMessage);
    static void setMaxDepth(int maxDepth);
    static QString peek();

private:
    QThreadStorage<QStack<QString> *> mStack;
};

#ifndef QT_NO_DEBUG_STREAM
/*!
 * \relates NDC
 *
 * Writes all object member variables to the given debug stream \a rDebug and
 * returns the stream.
 *
 * <tt>
 * %NDC(thread:"main" peek:"i = 3" depth:4)
 * </tt>
 * \sa QDebug
 */
QDebug operator<<(QDebug debug,
                  const NDC &rNDC);
#endif // QT_NO_DEBUG_STREAM

inline NDC::NDC() :
    mStack()
{}

} // namespace Log4Qt

#endif // LOG4QT_NDC_H
