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

#include "helpers/initialisationhelper.h"

#include "helpers/datetime.h"
#include "helpers/logerror.h"
#include "loggingevent.h"

#include <QCoreApplication>
#include <QMutex>
#include <QProcess>
#include <QSettings>

#ifndef QT_NO_DATASTREAM
#include <QDataStream>
#endif

#if (__cplusplus >= 201703L) // C++17 or later
#include <utility>
#endif

namespace Log4Qt
{

InitialisationHelper::InitialisationHelper() :
    mStartTime(QDateTime::currentMSecsSinceEpoch())
{
    doRegisterTypes();
    doInitialiseEnvironmentSettings();
}

InitialisationHelper::~InitialisationHelper()
{
    Q_ASSERT_X(false, "InitialisationHelper::~InitialisationHelper()", "Unexpected destruction of singleton object");
}

LOG4QT_IMPLEMENT_INSTANCE(InitialisationHelper)

void InitialisationHelper::doInitialiseEnvironmentSettings()
{
    // Is Process::systemEnvironment() safe to be used before a QCoreApplication
    // object has been created?

    QStringList setting_keys;
    setting_keys << QStringLiteral("Debug");
    setting_keys << QStringLiteral("DefaultInitOverride");
    setting_keys << QStringLiteral("Configuration");

    QHash<QString, QString> env_keys;
#if (__cplusplus >= 201703L)
    for (const auto &entry : std::as_const(setting_keys))
#else
    for (const auto &entry : qAsConst(setting_keys))
#endif
        env_keys.insert(QStringLiteral("log4qt_").append(entry).toUpper(), entry);

    QStringList sys_env = QProcess::systemEnvironment();
#if (__cplusplus >= 201703L)
    for (const auto &entry : std::as_const(sys_env))
#else
    for (const auto &entry : qAsConst(sys_env))
#endif
    {
        int i = entry.indexOf(QLatin1Char('='));
        if (i == -1)
            continue;
        QString key = entry.left(i);
        QString value = entry.mid(i + 1).trimmed();
        if (env_keys.contains(key))
            mEnvironmentSettings.insert(env_keys.value(key), value);
    }
}

void InitialisationHelper::doRegisterTypes()
{
    qRegisterMetaType<Log4Qt::LogError>("Log4Qt::LogError");
    qRegisterMetaType<Log4Qt::Level>("Log4Qt::Level");
    qRegisterMetaType<Log4Qt::LoggingEvent>("Log4Qt::LoggingEvent");

#ifndef QT_NO_DATASTREAM
#if QT_VERSION < 0x060000
    qRegisterMetaTypeStreamOperators<Log4Qt::LogError>("Log4Qt::LogError");
    qRegisterMetaTypeStreamOperators<Log4Qt::Level>("Log4Qt::Level");
    qRegisterMetaTypeStreamOperators<LoggingEvent>("Log4Qt::LoggingEvent");
#endif
#endif

}

QString InitialisationHelper::doSetting(const QString &key,
                                        const QString &defaultValue) const
{
    if (mEnvironmentSettings.contains(key))
        return mEnvironmentSettings.value(key);

    if (QCoreApplication::instance() != nullptr)
    {
        QSettings s;
        s.beginGroup(QStringLiteral("Log4Qt"));
        return s.value(key, defaultValue).toString().trimmed();
    }
    return defaultValue;
}

bool InitialisationHelper::staticInitialisation()
{
    instance();
    return true;
}

bool InitialisationHelper::mStaticInitialisation = staticInitialisation();

} // namespace Log4Qt
