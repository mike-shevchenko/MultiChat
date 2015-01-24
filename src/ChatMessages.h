#ifndef CHATMESSAGES_H
#define CHATMESSAGES_H

// Classes for messages sent between Apps to implement the chat protocol.

#include <QString>
#include <stdexcept>

namespace Chat {

/**
 * Error parsing the string with a serialized Message.
 */
class ParseEx : public std::invalid_argument
{
public:
    ParseEx(const QString &what)
        : std::invalid_argument(what.toStdString())
    {}
};

// forward:
class UserMessage;
class LeaveMessage;
class TextMessage;
class AckMessage;

/**
 * Abstract base for messages sent via multicast.
 *
 * Each message is sent via multicast as a single UDP datagram. Each
 * datagram payload is a message serialized as a UTF-8 string.
 *
 * The following message types are supported:
 *
 * user|<sender.nick>
 *     Sent regularly by each App. Populates contact list.
 *
 * leave|<sender.nick>
 *     Sent what an App exits. Depopulates contact list.
 *
 * text|<sender.nick>|<text.id>|<text>
 *     Carries a chat text message. Leads to sending "ack".
 *
 * ack|<text.sender.id>|<text.id>
 *     Sent when the App receives a "text" message.
 *
 * NOTES:
 * - The '|' char is used as a field delimiter, thus, ony the last field of
 *   a message is allowed to contain this char.
 * - <text.id> is used only as a unique id of a text sent by an App among
 *   other texts sent by the same App. It is a 64-bit signed integer, its
 *   semantics is not defined by the message class.
 * - <text.sender.id> is used to identify the sender of the text being
 *   acknowledged, its semantics it not defined by the message class.
 */
class Message
{
public:
    // Each derived class provides static cType constant for comparison.
    typedef const char *Type;
    const Type type;

private:
    const QString senderId;

protected:
    Message(Type type, const QString& senderId)
        : type(type), senderId(senderId)
    {}

public:
    virtual ~Message()
    {}

    /**
     * The senderId property is not part of serialized message, and is
     * supplied by the message receiving mechanism.
     * @return Empty string for messages created from a string (e.g.
     * incoming).
     */
    const QString& getSenderId() const
    {
        return senderId;
    }

    /**
     * Factory: parse the string to create a message of the proper type.
     */
    static Message *createFromUtf8(
        const QByteArray &utf8, const QString &senderId)
        throw (ParseEx);

    /**
     * Visitor: handles all message types.
     */
    class Handler // interface
    {
    public:
        virtual ~Handler()
        {}

        virtual void handleUserMessage(const UserMessage &message) = 0;
        virtual void handleLeaveMessage(const LeaveMessage &message) = 0;
        virtual void handleTextMessage(const TextMessage &message) = 0;
        virtual void handleAckMessage(const AckMessage &message) = 0;
    };

    virtual void handleBy(Handler *pHandler) const = 0;

    virtual QByteArray toUtf8() const = 0;
};

class UserMessage : public Message
{
private:
    const QString senderNick;

public:
    static const Type cType;

    UserMessage(const QString &senderNick,
        const QString &senderId = "")
        : Message(cType, senderId), senderNick(senderNick)
    {}

    virtual ~UserMessage() override
    {}

    QString getSenderNick() const
    {
        return senderNick;
    }

    virtual void handleBy(Handler *pHandler) const override
    {
        pHandler->handleUserMessage(*this);
    }

    virtual QByteArray toUtf8() const override;
};

class LeaveMessage : public Message
{
private:
    const QString senderNick;

public:
    static const Type cType;

    LeaveMessage(const QString &senderNick,
        const QString &senderId = "")
        : Message(cType, senderId), senderNick(senderNick)
    {}

    virtual ~LeaveMessage() override
    {}

    QString getSenderNick() const
    {
        return senderNick;
    }

    virtual void handleBy(Handler *pHandler) const override
    {
        pHandler->handleLeaveMessage(*this);
    }

    virtual QByteArray toUtf8() const override;
};

class TextMessage : public Message
{
private:
    const QString senderNick;
    const qint64 textId;
    const QString text;

public:
    static const Type cType;

    TextMessage(const QString &senderNick, qint64 textId,
        const QString &text, const QString &senderId = "")
        : Message(cType, senderId), senderNick(senderNick), textId(textId),
            text(text)
    {}

    virtual ~TextMessage() override
    {}

    QString getSenderNick() const
    {
        return senderNick;
    }

    qint64 getTextId() const
    {
        return textId;
    }

    QString getText() const
    {
        return text;
    }

    virtual void handleBy(Handler *pHandler) const override
    {
        pHandler->handleTextMessage(*this);
    }

    virtual QByteArray toUtf8() const override;
};

class AckMessage : public Message
{
private:
    const QString textSenderId;
    const qint64 textId;

public:
    static const Type cType;

    AckMessage(const QString &textSenderId, qint64 textId,
        const QString &senderId = "")
        : Message(cType, senderId), textSenderId(textSenderId),
            textId(textId)
    {}

    virtual ~AckMessage() override
    {}

    QString getTextSenderId() const
    {
        return textSenderId;
    }

    qint64 getTextId() const
    {
        return textId;
    }

    virtual void handleBy(Handler *pHandler) const override
    {
        pHandler->handleAckMessage(*this);
    }

    virtual QByteArray toUtf8() const override;
};

} // namespace Chat

#endif // CHATMESSAGES_H
