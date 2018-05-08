/******************************************************************************
 *
 * package:     Log4Qt
 * file:        mdc.h
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

#ifndef LOG4QT_MDC_H
#define LOG4QT_MDC_H

#include "log4qt.h"

#include <QString>
#include <QHash>
#include <QThreadStorage>

namespace Log4Qt
{

/*!
     * \brief The class MDC implements a mapped diagnostic context.
     *
     * \note All the functions declared in this class are thread-safe.
     */
class  LOG4QT_EXPORT MDC
{
private:
    MDC();
    Q_DISABLE_COPY(MDC)

public:
    static QString get(const QString &key);
    static QHash<QString, QString> context();

    /*!
     * Returns the MDC instance.
     */
    static MDC *instance();

    static void put(const QString &key, const QString &value);
    static void remove(const QString &key);

private:
    static QHash<QString, QString> *localData();

private:
    QThreadStorage<QHash<QString, QString> *> mHash;
};

inline MDC::MDC()
{}

inline void MDC::put(const QString &key, const QString &value)
{
    localData()->insert(key, value);
}

inline void MDC::remove(const QString &key)
{
    localData()->remove(key);
}


} // namespace Log4Qt

#endif // LOG4QT_MDC_H
