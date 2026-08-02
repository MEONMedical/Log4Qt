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

#ifndef LOG4QT_LOGERROR_H
#define LOG4QT_LOGERROR_H

#include "log4qt/log4qtshared.h"

#include <QString>
#include <QVariant>

namespace Log4Qt
{
/*!
 * Creates an LogError object with the error message \a message, the error
 * code \a code and the context \a context. The symbol of the error is
 * set to \a code as string value.
 *
 * The following example logs an error, if a character is not a digit.
 *
 * \code
 * if (!c.isDigit())
     * {
     *     Error e = LOG4QT_ERROR("Found character '%1' where digit was expected.",
     *                            LayoutExpectedDigitError,
     *                            "Log4Qt::PatternFormatter");
     *     e << QString(c);
 *     logger()->error(e);
     * }
 * \endcode
 */
#define LOG4QT_ERROR(message, code, context)                              \
        LogError(message, code, #code, context)

/*!
 * Creates an LogError object with the error message \a message and the
 * error code \a code. The symbol of the error is set to \a code as string
 * value. The context is set to the class name of the current object. The
 * current objects class must be derived from QObject.
 *
 * The following example handles an error while opening a file.
 *
 * \code
 * if (!mpFile->open(mode))
 * {
 *      LogError e = LOG4QT_QCLASS_ERROR("Unable to open file '%1' for appender '%2'",
     *                                       AppenderOpeningFileError);
 *      e << mFileName << name();
 *      e.addCausingError(LogError(mpFile->errorString(), mpFile->error()));
 *      logger()->error(e);
 *      return;
 * }
 * \endcode
 */
#define LOG4QT_QCLASS_ERROR(message, code)                                \
        LogError(message, code, #code, this->metaObject()->className())

/*!
 * \brief The class LogError represents an error.
 *
 * The class error allows storing error information in a structured way.
 * The error message is stored separately from the information that may be
 * substituted into the message string. This way all information is still
 * accessible after the error has been raised.
 *
 * The message is accessed using message() and setMessage(). Arguments for
 * the message can be added using addArg() or operator<<(). The arguments
 * can be retrieved using args(). The message with substituted arguments
 * is returned by messageWithArgs().
 *
 * An error code can be set as integer value code() and/or a symbolic value
 * symbol().
 *
 * The error also stores the context it was raised in (context(),
 * setContext()) — usually the class name. toString() renders it together
 * with the symbol as \c Context::SYMBOL.
 *
 * An error can have one or more related errors that caused it. An error is
 * related using addCausingError(). All causing errors can be retrieved using
 * causingErrors().
 *
 * A per thread error can be maintained using lastError() and setLastError().
 *
 * There are two macros avaiable to simplify the error creation. The macro
 * \ref Log4Qt::LOG4QT_ERROR "LOG4QT_ERROR" is used with classes not derived
 * from QObject. The macro \ref Log4Qt::LOG4QT_QCLASS_ERROR "LOG4QT_QCLASS_ERROR"
 * is used with classes derived from QObject.
 */
class LOG4QT_EXPORT LogError
{
public:

    /*!
    * Creates an empty error. The error code is set to 0 and all other
    * members are set to be empty.
    *
    * \sa isEmpty()
    */
    LogError();

    /*!
     * Creates an error with the Message \a message and the error code
     * \a code. The symbol for the error code is set to \a rSymbol and the
     * context to \a rContext.
     *
     * \sa context(), toString()
     */
    explicit LogError(const QString &message,
                      int code = 0,
                      const QString &symbol = QString(),
                      const QString &context = QString());

    /*!
     * Creates an error with the Message \a pMessage and the error code
     * \a code. The symbol for the error code is set to \a pSymbol and the
     * context to \a pContext.
     *
     * \a pMessage is expected to be UTF-8; \a pSymbol and \a pContext are
     * expected to be Latin-1.
     *
     * \note To support the macros \ref Log4Qt::LOG4QT_ERROR "LOG4QT_ERROR"
     *       and \ref Log4Qt::LOG4QT_QCLASS_ERROR "LOG4QT_QCLASS_ERROR"
     *       the function tests, if \a pSymbol is the string representation of
     *       \a code. If it is, the symbol is set to be empty. Otherwise symbol
     *       is set to \a pSymbol.
     *
     * \sa context(), toString()
     */
    explicit LogError(const char *message,
                      int code = 0,
                      const char *symbol = nullptr,
                      const char *context = nullptr);

    /*!
     * Returns the error code.
     *
     * \sa setCode()
     */
    int code() const { return mCode; }

    /*!
     * Returns the context for the error.
     *
     * \sa setContext()
     */
    [[nodiscard]] const QString &context() const { return mContext; }

    /*!
     * Returns the error message.
     *
     * \sa setMessage()
     */
    [[nodiscard]] const QString &message() const { return mMessage; }

    /*!
     * Returns the symbol for the error code.
     *
     * \sa setSymbol()
     */
    [[nodiscard]] const QString &symbol() const { return mSymbol; }

    /*!
     * Sets the error code to \a code.
     *
     * \sa code()
     */
    void setCode(int code) { mCode = code; }

    /*!
     * Sets the context to \a className. The context is usually the name of
     * the class that raised the error; toString() renders it together with
     * the symbol as \c Context::SYMBOL.
     *
     * \sa context(), toString()
     */
    void setContext(const QString &context) { mContext = context; }

    /*!
     * Sets the error message to \a message
     *
     * \sa message()
     */
    void setMessage(const QString &message) { mMessage = cleanMessage(message); }

    /*!
     * Sets the symbol for the error code to \a symbol.
     *
     * \sa symbol()
     */
    void setSymbol(const QString &symbol) { mSymbol = symbol; }

    /*!
     * Returns the last error set for the current thread using
     * setLastError().
     *
     * \note: This function is thread-safe.
     *
     * \sa setLastError()
     */
    static LogError lastError();

    /*!
     * Sets the last error for the current thread to \a logError.
     *
     * \note: This function is thread-safe.
     *
     * \sa lastError()
     */
    static void setLastError(const LogError &logError);

    /*!
     * Appends \a rArg to the list of arguments and returns a reference to
     * this error.
     *
     * \sa operator<<(), args(), clearArgs()
     */
    LogError &addArg(const QVariant &arg)
    {
        mArgs << arg;
        return *this;
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    LogError &addArg(int arg)
    {
        mArgs << QVariant(arg);
        return *this;
    }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    LogError &addArg(const QString &arg)
    {
        mArgs << QVariant(arg);
        return *this;
    }

    /*!
     * Appends \a logError to the list of causing errors and returns a
     * reference to this error.
     *
     * \sa causingErrors(), clearCausingErrors()
     */
    LogError &addCausingError(const LogError &logError)
    {
        mCausingErrors << logError;
        return *this;
    }

    /*!
     * Returns the list of arguments that have been added to this error.
     *
     * \sa addArg(), operator<<(), clearArgs()
     */
    [[nodiscard]] const QList<QVariant> &args() const { return mArgs; }

    /*!
     * Returns the list of causing errors that have been added to this error.
     *
     * \sa addArg(), operator<<(), clearArgs()
     */
    [[nodiscard]] const QList<LogError> &causingErrors() const { return mCausingErrors; }

    /*!
     * Clears the list of arguments that have been added to this error.
     *
     * \sa addArg(), operator<<(), args()
     */
    void clearArgs() { mArgs.clear(); }

    /*!
     * Clears the list of causing errors that have been added to this error.
     *
     * \sa addCausingError(), causingErrors()
     */
    void clearCausingErrors() { mCausingErrors.clear(); }

    /*!
     * Returns true, if the error code is 0 and the message is empty.
     * Otherwise it returns false.
     *
     * \sa code(), message()
     */
    bool isEmpty() const { return (mCode == 0) && mMessage.isEmpty(); }

    /*!
     * Returns the message with arguments. The arguments are incoorporated
     * into the messag using QString::arg().
     *
     * \sa QString::arg(), message(), toString()
     */
    QString messageWithArgs() const { return insertArgs(message()); }

    /*!
     * Appends \a rArg to the list of arguments and returns a reference to
     * this error.
     *
     * \sa addArg()
     */
    LogError &operator<<(const QVariant &arg) { return addArg(arg); }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    LogError &operator<<(int arg) { return addArg(arg); }

    /*!
     * This is an overloaded member function, provided for convenience.
     */
    LogError &operator<<(const QString &arg) { return addArg(arg); }

    /*!
     * Returns a string representation of the error.
     *
     * The string has the following format:
     *
     * <tt>
     * message (context::symbol, code): causing_error, causing_error
     * </tt>
     *
     * If members are empty they are omitted:
     * - Omit context, if empty
     * - Omit symbol, if empty
     * - Omit double colon with context and symbol, if both are empty
     * - Omit code, if 0
     * - Omit bracket with context/symbol and code, if all are empty
     * - Omit colon with causing errors, if no causing errors exist
     */
    QString toString() const;

private:
    QString insertArgs(const QString &message) const;
    QString cleanMessage(const QString &message);

private:
    int mCode;
    QString  mContext;
    QString  mMessage;
    QString  mSymbol;
    QList<QVariant> mArgs;
    QList<LogError> mCausingErrors;

#ifndef QT_NO_DATASTREAM
    // Needs to be friend to stream objects
    friend QDataStream &operator<<(QDataStream &stream,
                                   const LogError &logError);
    friend QDataStream &operator>>(QDataStream &stream,
                                   LogError &logError);
#endif // QT_NO_DATASTREAM
};

#ifndef QT_NO_DATASTREAM
/*!
 * \relates LogError
 *
 * Writes the given error \a logError to the given stream \a rStream,
 * and returns a reference to the stream.
 */
QDataStream &operator<<(QDataStream &stream,
                        const LogError &logError);

/*!
 * \relates LogError
 *
 * Reads an error from the given stream \a rStream into the given
 * error \a logError, and returns a reference to the stream.
 */
QDataStream &operator>>(QDataStream &stream,
                        LogError &logError);
#endif // QT_NO_DATASTREAM

} // namespace Log4Qt

Q_DECLARE_METATYPE(Log4Qt::LogError)
Q_DECLARE_TYPEINFO(Log4Qt::LogError, Q_MOVABLE_TYPE);

#endif // LOG4QT_ERROR_H
