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

#ifndef LOGGEROBJECT_H
#define LOGGEROBJECT_H

#include "log4qt/logger.h"

#include <QObject>
#include <QLoggingCategory>

class LoggerObject : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    explicit LoggerObject(QObject *parent = nullptr);

Q_SIGNALS:
    void exit(int code);

private Q_SLOTS:
    void onTimeout();

private:
    Q_DISABLE_COPY_MOVE(LoggerObject)
    int mCounter;
};

#endif // LOGGEROBJECT_H
