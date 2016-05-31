#ifndef LOGGING_H
#define LOGGING_H

#include <QString>

class QByteArray;
class QVariant;

class Logging
{
public:
    static QString createDumpString(const QByteArray &data, const bool withCaption = true);
    static QString toString(const QVariant &value);
    static QString indentString(const QString &string, const QStringList &indentBrackets);
};

#endif // LOGGING_H
