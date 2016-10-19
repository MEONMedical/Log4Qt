/******************************************************************************
 *
 * package:
 * file:        basicconfigurator.h
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

#ifndef LOG4QT_BASICCONFIGURATOR_H
#define LOG4QT_BASICCONFIGURATOR_H

#include "log4qt.h"

namespace Log4Qt
{

class Appender;

/*!
 * \brief The class BasicConfigurator provides a simple package
 *        configuration.
 *
 * \note All the functions declared in this class are thread-safe.
 */
class LOG4QT_EXPORT BasicConfigurator
{
private:
    Q_DISABLE_COPY(BasicConfigurator)

public:
    static bool configure();
    static void configure(Appender *pAppender);
    static void resetConfiguration();
};


} // namspace

#endif // _BASICCONFIGURATOR_H
