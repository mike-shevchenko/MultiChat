#include "ChatMessages.h"

using namespace Chat;

const Message::Type UserMessage::cType("user");
const Message::Type LeaveMessage::cType("leave");
const Message::Type TextMessage::cType("text");
const Message::Type AckMessage::cType("ack");

///////////////////////////////////////////////////////////////////////////
// Parsing utils.

static qint64 parseTextId(const QStringRef &s)
    throw (ParseEx)
{
    bool success = false;
    qint64 result = s.toLongLong(&success);
    if (!success) {
        throw ParseEx("\"" + s.toString() + "\" " +
            "is not a valid Text Id, int64 expected.");
    }

    return result;
}

/**
 * Parse next (non-last) field of a '|'-separated string. The field value
 * can not be empty, otherwise ParseEx is thrown.
 * @param pRest The string to parse; after parsing, is set to the rest of
 * the string.
 */
static QStringRef parseNextField(
    QStringRef *pRest, const QString &fieldName)
    throw (ParseEx)
{
    int pos = pRest->indexOf('|');
    if (pos == -1) {
        throw ParseEx("<" + fieldName + "> should not be the last field.");
    }

    QStringRef result = pRest->left(pos);
    *pRest = pRest->right(pRest->length() - pos - 1);

    if (result.isEmpty()) {
        throw ParseEx("<" + fieldName + "> should not be empty.");
    }

    return result;
}

/**
 * Parse the last field of a '|'-separated string. Thus, the field can not
 * contain '|' chars. The field value can not be empty, otherwise ParseEx
 * is thrown.
 * @param pRest The string to parse; after parsing, is set empty.
 */
static QStringRef parseLastField(
    QStringRef *pRest, const QString &fieldName)
    throw (ParseEx)
{
    int pos = pRest->indexOf('|');
    if (pos != -1) {
        throw ParseEx("Unexpected trailing fields found after <"
            + fieldName + ">: \"" + pRest->toString() + "\".");
    }

    QStringRef result = *pRest;
    *pRest = QStringRef();

    if (result.isEmpty()) {
        throw ParseEx("<" + fieldName + "> should not be empty.");
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////

// user|<sender.nick>

QByteArray UserMessage::toUtf8() const
{
    return QByteArray(type) + "|" + senderNick.toUtf8();
}

static UserMessage *createUserMessageFromString(
    const QStringRef &body, const QString &senderId)
    throw (ParseEx)
{
    QStringRef rest = body;
    QStringRef senderNick = parseLastField(&rest, "sender.nick");

    return new UserMessage(senderNick.toString(), senderId);
}

// leave|<sender.nick>

QByteArray LeaveMessage::toUtf8() const
{
    return QByteArray(cType) + "|" + senderNick.toUtf8();
}

static LeaveMessage *createLeaveMessageFromString(
    const QStringRef &body, const QString &senderId)
    throw (ParseEx)
{
    QStringRef rest = body;
    QStringRef senderNick = parseLastField(&rest, "sender.nick");

    return new LeaveMessage(senderNick.toString(), senderId);
}

// text|<sender.nick>|<text.id>|<text>

QByteArray TextMessage::toUtf8() const
{
    return QByteArray(cType) + "|" + senderNick.toUtf8() + "|"
        + QByteArray::number(textId) + "|" + text.toUtf8();
}

static TextMessage *createTextMessageFromString(
    const QStringRef &body, const QString &senderId)
    throw (ParseEx)
{
    QStringRef rest = body;
    QStringRef senderNick = parseNextField(&rest, "sender.nick");
    QStringRef textId = parseNextField(&rest, "text.id");

    return new TextMessage(senderNick.toString(), parseTextId(textId),
        rest.toString(), senderId);
}

// ack|<text.sender.id>|<text.id>

QByteArray AckMessage::toUtf8() const
{
    return QByteArray(cType) + "|" + textSenderId.toUtf8() + "|"
        + QByteArray::number(textId);
}

AckMessage *createAckMessageFromString(
    const QStringRef &body, const QString &senderId)
    throw (ParseEx)
{
    QStringRef rest = body;
    QStringRef textSenderId = parseNextField(&rest, "text.sender.id");
    QStringRef textId = parseLastField(&rest, "text.id");

    return new AckMessage(textSenderId.toString(), parseTextId(textId),
        senderId);
}

///////////////////////////////////////////////////////////////////////////

/**
 * ATTENTION: All message types should be registered in this function.
 */
static Message *createMessageByType(
    const QStringRef &messageType, const QStringRef &body,
    const QString &senderId)
    throw (ParseEx)
{
    if (messageType == UserMessage::cType) {
        return createUserMessageFromString(body, senderId);
    } else if (messageType == LeaveMessage::cType) {
        return createLeaveMessageFromString(body, senderId);
    } else if (messageType == TextMessage::cType) {
        return createTextMessageFromString(body, senderId);
    } else if (messageType == AckMessage::cType) {
        return createAckMessageFromString(body, senderId);
    } else {
        throw ParseEx("Unknown message type \"" +
            messageType.toString() + "\".");
    }
}

Message *Message::createFromUtf8(
    const QByteArray &utf8, const QString &senderId)
    throw (ParseEx)
{
    QString s(utf8);
    QStringRef body(&s);
    QStringRef messageType = parseNextField(&body, "message.type");

    try {
        return createMessageByType(messageType, body, senderId);
    } catch (ParseEx &e) {
        throw ParseEx("Unable to parse message: "
            + QString(e.what()) + " Message text:\n"
            + utf8);
    }
}
