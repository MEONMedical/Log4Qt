/******************************************************************************
 *
 * This file is part of Log4Qt library.
 *
 * Copyright (C) 2007 - 2026 Log4Qt contributors
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

#include "abstractlayout.h"

#include <QReadWriteLock>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

// Static member definitions
HeaderFooterProviderSharedPtr AbstractLayout::s_globalProvider;
QReadWriteLock AbstractLayout::s_providerLock;

AbstractLayout::AbstractLayout(QObject *parent) :
    QObject(parent)
{}

AbstractLayout::~AbstractLayout() = default;

QString AbstractLayout::contentType() const
{
    return u"text/plain"_s;
}

QString AbstractLayout::name() const
{
    return objectName();
}

void AbstractLayout::setFooter(const QString &footer)
{
    mFooter = footer;
}

void AbstractLayout::setHeader(const QString &header)
{
    mHeader = header;
}

void AbstractLayout::setName(const QString &name)
{
    setObjectName(name);
}

QString AbstractLayout::header() const
{
    if (mHeaderFooterProvider) {
        QString h = mHeaderFooterProvider->header();
        if (!h.isEmpty()) return h;
    }
    if (!mHeader.isEmpty()) return mHeader;
    {
        QReadLocker lk(&s_providerLock);
        if (s_globalProvider) {
            QString h = s_globalProvider->header();
            if (!h.isEmpty()) return h;
        }
    }
    return {};
}

QString AbstractLayout::footer() const
{
    if (mHeaderFooterProvider) {
        QString f = mHeaderFooterProvider->footer();
        if (!f.isEmpty()) return f;
    }
    if (!mFooter.isEmpty()) return mFooter;
    {
        QReadLocker lk(&s_providerLock);
        if (s_globalProvider) {
            QString f = s_globalProvider->footer();
            if (!f.isEmpty()) return f;
        }
    }
    return {};
}

void AbstractLayout::setHeaderFooterProvider(const HeaderFooterProviderSharedPtr &provider)
{
    Q_ASSERT_X(!mActivated,
               "AbstractLayout::setHeaderFooterProvider",
               "setHeaderFooterProvider() must be called before activateOptions()");
    mHeaderFooterProvider = provider;
}

HeaderFooterProviderSharedPtr AbstractLayout::headerFooterProvider() const
{
    return mHeaderFooterProvider;
}

void AbstractLayout::setGlobalHeaderFooterProvider(const HeaderFooterProviderSharedPtr &provider)
{
    QWriteLocker lk(&s_providerLock);
    s_globalProvider = provider;
}

HeaderFooterProviderSharedPtr AbstractLayout::globalHeaderFooterProvider()
{
    QReadLocker lk(&s_providerLock);
    return s_globalProvider;
}

void AbstractLayout::activateOptions()
{
    mActivated = true;
    if (mHeaderFooterProvider)
        mHeaderFooterProvider->activateOptions();
}

bool AbstractLayout::requiresLocation() const
{
    return false;
}

QString AbstractLayout::endOfLine()
{
    // There seams to be no function in Qt for this. MinGW enter '\r\n' automatically
    return u"\n"_s;
}

} // namespace Log4Qt

#include "moc_abstractlayout.cpp"
