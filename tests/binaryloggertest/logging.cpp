#include "logging.h"

#include <QByteArray>
#include <QVariant>
#include <QDateTime>
#include <QStringBuilder>

static QString escape(const QString &string);
static QString toString(const QVariantMap &map);
static QString toString(const QVariantList &list);
static QString toString(const QVariantHash &hash);
static QString toString(const QStringList &stringList);
static QString toString(const QByteArray &byteArray);
static QString toString(const QDate &date);
static QString toString(const QTime &time);
static QString toString(const QDateTime &datetime);
static QString toString(const QString &string);
static QString toString(const QVariant &value, int userType);
static QString toString(const QVariant &value);
static bool isOpenBracket(const QStringList &brackets, const QChar &c);
static bool isCloseBracket(const QStringList &brackets, const QChar &c);

QString Logging::createDumpString(const QByteArray &data, const bool withCaption)
{
    const int bytesPerLine  = 16;
    const QByteArray dist("  ");
    const QByteArray placeHolder("   ");
    const QByteArray intention("      ");
    QByteArray line;
    QByteArray details(dist);
    unsigned char fromChar = 0x20;
    unsigned char toChar   = 0x7e;
    QByteArray out;
    if (withCaption)
        out.append("Dump:\n      offset (hex) - data (hex) - data (text)\n");

    QByteArray hexData = data.toHex();

    line.append(intention);
    line.append("00000000  ");
    for ( int i = 1, ii = 0; i <= data.size(); ++i, ii += 2 )
    {
        unsigned char byte = data[i - 1];

        line.append(hexData.mid(ii, 2));
        line.append(' ');

        if ( byte >= fromChar && byte <= toChar )
            details.append(QChar::fromLatin1(byte));
        else
            details.append('.');

        if (( i % bytesPerLine ) == 0)
        {
            // one line ready
            line.append(details);
            out.append(line);
            out.append('\n');
            line.clear();

            line.append(intention);
            line.append(QString::fromLatin1("%0  ").arg(i, 8, 16, QLatin1Char('0')));

            details.clear();
            details.append(dist);
        }
        else if (i == data.size())
        {
            int remaining = bytesPerLine - ( i % bytesPerLine );
            for ( int j = 0; j < remaining; j++ )
                line.append(placeHolder);

            // end if data
            line.append(details);
            out.append(line);
        }
    }
    return out;
}

QString Logging::toString(const QVariant &value)
{
    return toString(value);
}

QString toString(const QVariant &value)
{
    switch (value.type())
    {
    case QVariant::Invalid:
        return "<Invalid>";
    case QVariant::BitArray:
        return "<BitArray>";
    case QVariant::Bitmap:
        return "<Bitmap>";
    case QVariant::Brush:
        return "<Brush>";
    case QVariant::Color:
        return "<Color>";
    case QVariant::Cursor:
        return "<Cursor>";
    case QVariant::EasingCurve:
        return "<EasingCurve>";
    case QVariant::ModelIndex:
        return "<ModelIndex>";
    case QVariant::Font:
        return "<Font>";
    case QVariant::Icon:
        return "<Icon>";
    case QVariant::Image:
        return "<Image>";
    case QVariant::KeySequence:
        return "<KeySequence>";
    case QVariant::Line:
        return "<Line>";
    case QVariant::LineF:
        return "<LineF>";
    case QVariant::Locale:
        return "<Locale>";
    case QVariant::Matrix:
        return "<Matrix>";
    case QVariant::Transform:
        return "<Transform>";
    case QVariant::Matrix4x4:
        return "<Matrix4x4>";
    case QVariant::Palette:
        return "<Palette>";
    case QVariant::Pen:
        return "<Pen>";
    case QVariant::Pixmap:
        return "<Pixmap>";
    case QVariant::Point:
        return "<Point>";
    case QVariant::PointF:
        return "<PointF>";
    case QVariant::Polygon:
        return "<Polygon>";
    case QVariant::PolygonF:
        return "<PolygonF>";
    case QVariant::Quaternion:
        return "<Quaternion>";
    case QVariant::Rect:
        return "<Rect>";
    case QVariant::RectF:
        return "<RectF>";
    case QVariant::RegExp:
        return "<RegExp>";
    case QVariant::RegularExpression:
        return "<RegularExpression>";
    case QVariant::Region:
        return "<Region>";
    case QVariant::Size:
        return "<Size>";
    case QVariant::SizeF:
        return "<SizeF>";
    case QVariant::SizePolicy:
        return "<SizePolicy>";
    case QVariant::TextFormat:
        return "<TextFormat>";
    case QVariant::TextLength:
        return "<TextLength>";
    case QVariant::Vector2D:
        return "<Vector2D>";
    case QVariant::Vector3D:
        return "<Vector3D>";
    case QVariant::Vector4D:
        return "<Vector4D>";

    case QVariant::Int:
    case QVariant::Double:
    case QVariant::Char:
    case QVariant::Bool:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Url:
    case QVariant::Uuid:
        return value.toString();

    case QVariant::ByteArray:
        return toString(value.toByteArray());

    case QVariant::Date:
        return toString(value.toDate());

    case QVariant::DateTime:
        return toString(value.toDateTime());

    case QVariant::Hash:
        return toString(value.toHash());

    case QVariant::List:
        return toString(value.toList());

    case QVariant::Map:
        return toString(value.toMap());

    case QVariant::String:
        return toString(value.toString());

    case QVariant::StringList:
        return toString(value.toStringList());

    case QVariant::Time:
        return toString(value.toTime());

    case QVariant::UserType:
        return toString(value, value.userType());

    default:
        break;
    }
    return QString("<Unknow variant type>");
}

QString toString(const QVariantMap &map)
{
    QStringList result;
    for (auto pos = map.cbegin(); pos != map.cend(); ++pos)
        result << QString("%1=%2").arg(toString(pos.key()), toString(pos.value()));
    return QStringLiteral("{") % result.join(", ") % QStringLiteral("}");
}

QString toString(const QVariantList &list)
{
    QStringList result;
    for (const auto &value : list)
        result << toString(value);
    return QStringLiteral("[") % result.join(", ") % QStringLiteral("]");
}

QString toString(const QVariantHash &hash)
{
    QStringList result;
    for (auto pos = hash.cbegin(); pos != hash.cend(); ++pos)
        result << QString("%1=%2").arg(toString(pos.key()), toString(pos.value()));
    return QStringLiteral("{") % result.join(", ") % QStringLiteral("}");
}

QString toString(const QStringList &stringList)
{
    QStringList result;
    for (const auto &string : stringList)
        result << toString(string);
    return QStringLiteral("[") % result.join(", ") % QStringLiteral("]");
}

QString toString(const QByteArray &byteArray)
{
    QStringList result;
    for (auto byte : byteArray)
        result << QString("%1").arg(byte, 2, 16, QChar('0'));
    return QStringLiteral("[") % result.join(", ") % QStringLiteral("]");
}

QString toString(const QDate &date)
{
    return date.toString("yyyy-MM-dd");
}

QString toString(const QTime &time)
{
    return time.toString("hh:mm:ss.zzz");
}

QString toString(const QDateTime &datetime)
{
    return toString(datetime.date()) % QStringLiteral("T") % toString(datetime.time()) + datetime.timeZoneAbbreviation();
}

QString toString(const QString &string)
{
    return QStringLiteral("\"") % escape(string) % QStringLiteral("\"");
}

QString escape(const QString &string)
{
    QString copy(string);
    return copy.replace('"', "\\\"")
           .replace('\n', "\\n")
           .replace('\r', "\\r")
           .replace('\t', "\\t");
}

QString toString(const QVariant &value, int userType)
{
    Q_UNUSED(value)
    return QString("{UserType: %1").arg(userType);
}

bool isOpenBracket(const QStringList &brackets, const QChar &c)
{
    for (const auto &b : brackets)
    {
        if (!b.isEmpty() && b[0] == c)
            return true;
    }
    return false;
}

bool isCloseBracket(const QStringList &brackets, const QChar &c)
{
    for (const auto &b : brackets)
    {
        if (b.length() > 1 && b[1] == c)
            return true;
    }
    return false;
}

QString Logging::indentString(const QString &string, const QStringList &indentBrackets)
{
    QString result;
    QChar quote{'\0'};
    QChar last{'\0'};
    QChar current{'\0'};
    int indent = 0;

    for (int i = 0; i < string.length(); ++i)
    {
        current = string[i];

        if (quote != '\0')
        {
            if (quote == current)
                quote = '\0';
        }
        else
        {
            switch (current.toLatin1())
            {
            case '\'':
            case '"':
                quote = current;
                break;
            case ',':
                result += "\n" + QString(indent, QChar(' '));
                break;
            default:
                if (isOpenBracket(indentBrackets, current))
                {
                    result += "\n" + QString(indent, QChar(' '));
                    indent++;
                }
                else if (isCloseBracket(indentBrackets, current))
                {
                    indent--;
                    result += "\n" + QString(indent, QChar(' '));
                }
                break;
            }
        }
        if (isOpenBracket(indentBrackets, last))
        {
            if (!isCloseBracket(indentBrackets, current))
                result += "\n" + QString(indent, QChar(' '));
        }
        result += QString(current);
        last = string[i];
    }
    return result;
}

