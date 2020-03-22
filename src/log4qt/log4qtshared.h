/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2020 Log4Qt contributors
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

#ifndef LOG4QT_SHARED_H
#define LOG4QT_SHARED_H

#include <QtGlobal>

// Define LOG4QT_STATIC in you applikation if you want to link against the
// static version of Log4Qt

#ifdef LOG4QT_STATIC
#   define LOG4QT_EXPORT
#else
#  if defined(LOG4QT_LIBRARY)
#    define LOG4QT_EXPORT Q_DECL_EXPORT
#  else
#    define LOG4QT_EXPORT Q_DECL_IMPORT
#  endif
#endif

#endif // LOG4QT_SHARED_H
