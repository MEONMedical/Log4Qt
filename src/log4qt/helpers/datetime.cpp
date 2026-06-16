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

#include "helpers/datetime.h"

#include "helpers/initialisationhelper.h"

#include <atomic>
#include <QReadWriteLock>

using namespace Qt::StringLiterals;

namespace Log4Qt
{

namespace
{

// Thread-local cache for a single formatted timestamp.
// epochMs == -1 means the cache is empty.
struct TimestampCache
{
    qint64 epochMs = -1;
    QString str;
};

// -----------------------------------------------------------------------
// currentMSecsSinceEpoch() caching state
// -----------------------------------------------------------------------

// Per-thread cached epoch-ms value and the monotonic counter at which it
// was captured. Used by DateTime::currentMSecsSinceEpoch().
static thread_local qint64 s_cachedTimestamp = 0;
static thread_local qint64 s_lastCounterValue = 0;

// Configurable cache window (ms). Atomic because setCacheWindow() and
// currentMSecsSinceEpoch() may be called from different threads.
static std::atomic<qint64> s_cacheWindowMs{1};

// Monotonic timer started at library load time.
static QElapsedTimer s_elapsedTimer = []() {
    QElapsedTimer t;
    t.start();
    return t;
}();

// -----------------------------------------------------------------------
// Global provider
// -----------------------------------------------------------------------

static QReadWriteLock s_providerLock;
static DateTime::Provider s_globalProvider = []() { return QDateTime::currentDateTime(); };

} // anonymous namespace

DateTime::DateTime() = default;

DateTime::~DateTime() = default;

DateTime::DateTime(const DateTime &other) noexcept = default;

// Static fast path: formats epoch ms without constructing a QDateTime for the
// named formats. Named formats are cached thread-locally (keyed by epoch ms)
// so that repeated calls within the same millisecond cost only a qint64
// comparison and a string copy (~1 ns). On a cache miss, formatting is
// delegated to QDateTime::toString() as usual.
QString DateTime::formatMsecs(qint64 msecs, const QString &format)
{
    if (format.isEmpty())
        return {};

    if (format == u"NONE"_s)
        return {};
    if (format == u"RELATIVE"_s)
        return QString::number(msecs - InitialisationHelper::startTime());

    if (format == u"ISO8601"_s)
    {
        thread_local TimestampCache cache;
        if (cache.epochMs != msecs)
        {
            cache.epochMs = msecs;
            cache.str = QDateTime::fromMSecsSinceEpoch(msecs).toString(u"yyyy-MM-dd hh:mm:ss.zzz"_s);
        }
        return cache.str;
    }

    if (format == u"ABSOLUTE"_s)
    {
        thread_local TimestampCache cache;
        if (cache.epochMs != msecs)
        {
            cache.epochMs = msecs;
            cache.str = QDateTime::fromMSecsSinceEpoch(msecs).toString(u"HH:mm:ss.zzz"_s);
        }
        return cache.str;
    }

    if (format == u"DATE"_s)
    {
        thread_local TimestampCache cache;
        if (cache.epochMs != msecs)
        {
            cache.epochMs = msecs;
            cache.str = QDateTime::fromMSecsSinceEpoch(msecs).toString(u"dd MM yyyy HH:mm:ss.zzz"_s);
        }
        return cache.str;
    }

    return QDateTime::fromMSecsSinceEpoch(msecs).toString(format);
}

QString DateTime::toString(const QString &format) const
{
    if (!isValid())
        return {};
    return formatMsecs(toMSecsSinceEpoch(), format);
}

QString DateTime::formatDateTime(const QString &format) const
{
    return QDateTime::toString(format);
}

DateTime DateTime::currentDateTime()
{
    QReadLocker lk(&s_providerLock);
    return DateTime(s_globalProvider());
}

void DateTime::setProvider(Provider provider)
{
    QWriteLocker lk(&s_providerLock);
    s_globalProvider = provider ? std::move(provider)
                                : []() { return QDateTime::currentDateTime(); };
}

qint64 DateTime::currentMSecsSinceEpoch()
{
    const qint64 cacheWindow = s_cacheWindowMs.load(std::memory_order_relaxed);

    auto wallClock = []() -> qint64 {
        QReadLocker lk(&s_providerLock);
        return s_globalProvider().toMSecsSinceEpoch();
    };

    if (cacheWindow <= 0)
        return wallClock();

    if (Q_UNLIKELY(!s_elapsedTimer.isValid()))
        s_elapsedTimer.start();

    const qint64 now = s_elapsedTimer.elapsed();
    const qint64 elapsed = now - s_lastCounterValue;

    if (s_cachedTimestamp == 0 || elapsed < 0 || elapsed >= cacheWindow)
    {
        s_cachedTimestamp = wallClock();
        s_lastCounterValue = now;
    }

    return s_cachedTimestamp;
}

void DateTime::setCacheWindow(qint64 cacheWindowMs)
{
    s_cacheWindowMs.store(cacheWindowMs, std::memory_order_relaxed);
}

qint64 DateTime::cacheWindow()
{
    return s_cacheWindowMs.load(std::memory_order_relaxed);
}

} // namespace Log4Qt
