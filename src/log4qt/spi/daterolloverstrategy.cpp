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

#include "spi/daterolloverstrategy.h"

#include "helpers/datetime.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrentRun>

#include <QRegularExpression>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace
{

void deleteObsoleteFiles(
        Log4Qt::DateRolloverStrategy::NamingMode mode,
        int maxBackups,
        int keepDays,
        const QString &datePattern,
        const QDate &currentDate,
        const QString &fileName,
        const QString &activeFileName)
{
    const QFileInfo fi(fileName);
    const QDir dir(fi.absolutePath());
    const QString base = fi.baseName();
    const QString ext = fi.completeSuffix();

    QString nameFilter;
    if (mode == Log4Qt::DateRolloverStrategy::NamingMode::Suffix)
        nameFilter = fi.fileName() + u"*"_s;
    else
        nameFilter = base + u"*"_s + (ext.isEmpty() ? u""_s : u"."_s + ext);

    QFileInfoList entries = dir.entryInfoList({nameFilter}, QDir::Files, QDir::Name);

    // Exclude the base file and the currently active file (a dated name in
    // Embedded/datedActiveFile operation) — the active file is not a backup
    // and must neither be counted against maxBackups nor deleted.
    const QString activeFilePath = QFileInfo(activeFileName).absoluteFilePath();
    entries.removeIf([&](const QFileInfo &entry) {
        return entry.absoluteFilePath() == fi.absoluteFilePath()
            || entry.absoluteFilePath() == activeFilePath;
    });

    if (keepDays > 0)
    {
        const QDate cutoff = currentDate.addDays(-keepDays);
        // The date extractor must mirror buildBackupName(): in Suffix mode
        // the date is appended after the full filename ("app.log.2026-07-30"),
        // in Embedded mode it is inserted between basename and extension
        // ("app.2026-07-30.log"). Escape the filename-derived parts: base/ext
        // can contain regex metacharacters (e.g. '.' in "app.v2.log") that
        // would otherwise match unintended sibling files and delete them.
        QString extractorPattern;
        if (mode == Log4Qt::DateRolloverStrategy::NamingMode::Suffix)
            extractorPattern = QRegularExpression::escape(fi.fileName()) + u"(.*)"_s;
        else
            extractorPattern = QRegularExpression::escape(base) + u"(.*)"_s
                + (ext.isEmpty() ? u""_s : u"\\."_s + QRegularExpression::escape(ext));
        const QRegularExpression dateExtractor(
            QRegularExpression::anchoredPattern(extractorPattern));

        entries.removeIf([&](const QFileInfo &entry) {
            const auto match = dateExtractor.match(entry.fileName());
            if (!match.hasMatch())
                return false;

            const auto fileDate = QDate::fromString(match.captured(1), datePattern);
            if (fileDate.isValid() && fileDate < cutoff)
                return QFile::remove(entry.absoluteFilePath());
            return false;
        });
    }

    if (maxBackups > 0 && entries.size() > maxBackups)
    {
        std::sort(entries.begin(), entries.end(), [](const QFileInfo &a, const QFileInfo &b) {
            return a.lastModified() > b.lastModified();
        });

        for (int i = maxBackups; i < entries.size(); ++i)
            QFile::remove(entries.at(i).absoluteFilePath());
    }
}

} // anonymous namespace

namespace Log4Qt
{

DateRolloverStrategy::DateRolloverStrategy(QObject *parent) :
    RolloverStrategy(parent),
    mDatePattern(u"'.'yyyy-MM-dd"_s),
    mMode(NamingMode::Suffix),
    mMaxBackups(0),
    mKeepDays(0),
    mDatedActiveFile(false)
{
}

QString DateRolloverStrategy::modeString() const
{
    return mMode == NamingMode::Embedded ? u"Embedded"_s : u"Suffix"_s;
}

void DateRolloverStrategy::setModeString(const QString &mode)
{
    if (mode.compare(u"Embedded"_s, Qt::CaseInsensitive) == 0)
        mMode = NamingMode::Embedded;
    else
        mMode = NamingMode::Suffix;
}

void DateRolloverStrategy::activateOptions()
{
    RolloverStrategy::activateOptions();
    mActiveSuffix = DateTime::currentDateTime().toString(mDatePattern);
}

void DateRolloverStrategy::waitForCleanup()
{
    mCleanupExecutors.waitForFinished();
}

QString DateRolloverStrategy::initialFileName(const QString &fileName) const
{
    if (!mDatedActiveFile)
        return fileName;
    return buildBackupName(fileName, DateTime::currentDateTime());
}

QString DateRolloverStrategy::rollover(const QString &fileName)
{
    const auto dateTime = DateTime::currentDateTime();

    // activeFileName is the file the appender will write to after this
    // rollover — the cleanup must never count or delete it.
    auto scheduleCleanup = [&](const QString &activeFileName) {
        if (mMaxBackups > 0 || mKeepDays > 0)
            mCleanupExecutors.addFuture(
                QtConcurrent::run(deleteObsoleteFiles, mMode, mMaxBackups,
                                  mKeepDays, mDatePattern, dateTime.date(),
                                  fileName, activeFileName));
    };

    if (mDatedActiveFile)
    {
        // Each period writes to its own dated file directly, so no rename
        // is needed — just return the dated name for the new active file.
        mActiveSuffix = dateTime.toString(mDatePattern);
        const QString activeName = buildBackupName(fileName, dateTime);
        scheduleCleanup(activeName);
        return activeName;
    }

    if (mMode == NamingMode::Suffix)
    {
        // Use the active suffix to name the backup after the period it belongs to.
        // mActiveSuffix is set during activateOptions() and updated after each rollover.
        const QString backupName = fileName
            + (mActiveSuffix.isEmpty() ? dateTime.toString(mDatePattern) : mActiveSuffix);
        mActiveSuffix = dateTime.toString(mDatePattern);

        if (QFile::exists(backupName))
            removeFile(backupName);
        if (QFile::exists(fileName))
            renameFile(fileName, backupName);
        scheduleCleanup(fileName);
        return fileName;
    }

    mActiveSuffix = dateTime.toString(mDatePattern);
    const QString backupName = buildBackupName(fileName, dateTime);
    scheduleCleanup(backupName);
    return backupName;
}

QString DateRolloverStrategy::buildBackupName(const QString &fileName,
                                               const QDateTime &dateTime) const
{
    const QString dateStr = dateTime.toString(mDatePattern);

    if (mMode == NamingMode::Suffix)
        return fileName + dateStr;

    // Embedded: insert date between basename and extension
    const QFileInfo fi(fileName);
    const QString dir = fi.absolutePath();
    const QString base = fi.baseName();
    const QString ext = fi.completeSuffix();

    if (ext.isEmpty())
        return dir + u"/"_s + base + dateStr;
    return dir + u"/"_s + base + dateStr + u"."_s + ext;
}

} // namespace Log4Qt

#include "moc_daterolloverstrategy.cpp"
