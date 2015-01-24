#include "ChatEngine.h"

#include "ContactList.h"
#include "Multicaster.h"
#include "ReliableTextSender.h"
#include "ReliableTextReceiver.h"
#include "ChatMessages.h"

using namespace Chat;

static const QVector<Message::Type> cMessageTypesToLog{
    TextMessage::cType, AckMessage::cType};

const Engine::Settings Engine::defaultSettings;

static const int cMaxNickUtf8Size = 64;
static const int cMaxTextUtf8Size = 255;

///////////////////////////////////////////////////////////////////////////
// Utils.

static const QString &validateNick(const QString &nick)
    throw (BadValueEx)
{
    if (nick.isEmpty()) {
        throw BadValueEx("Nick should not be empty.");
    }

    if (nick.indexOf('|') != -1) {
        throw BadValueEx("Nick should not contain '|' chars.");
    }

    if (nick.toUtf8().length() > cMaxNickUtf8Size) {
        throw BadValueEx("Nick is too long.");
    }

    return nick;
}

static const QString &validateText(const QString &text)
    throw (BadValueEx)
{
    if (text.toUtf8().length() > cMaxTextUtf8Size) {
        throw BadValueEx("Text is too long.");
    }

    return text;
}

static ReliableTextSender::Settings buildSenderSettings(
    const Engine::Settings &settings)
{
    ReliableTextSender::Settings result;
    result.maxAttempts = settings.textMaxAttempts;
    result.attemptPeriodMs = settings.textAttemptPeriodMs;
    return result;
}

static ReliableTextReceiver::Settings buildReceiverSettings(
    const Engine::Settings &settings)
{
    ReliableTextReceiver::Settings result;
    result.maxStoredMessageRecords = settings.textMaxStoredRecords;
    return result;
}

static ContactList::Settings buildContactListSettings(
    const Engine::Settings &settings)
{
    ContactList::Settings result;
    result.expiryPeriodMs = settings.contactExpiryPeriodMs;
    return result;
}

/**
 * Should be called before sending a message.
 */
static QByteArray toUtf8AndLogIfNeeded(const Message &message)
{
    QByteArray utf8 = message.toUtf8();

    if (cMessageTypesToLog.contains(message.type)) {
        qDebug() << "===>" << utf8;
    }

    return utf8;
}

///////////////////////////////////////////////////////////////////////////

class Engine::MessageHandler : public Message::Handler
{
private:
    Engine *const engine;

public:
    MessageHandler(Engine *engine)
        : engine(engine)
    {}

    virtual void handleUserMessage(const UserMessage &message) override
    {
        engine->handleUserMessage(message);
    }

    virtual void handleLeaveMessage(const LeaveMessage &message) override
    {
        engine->handleLeaveMessage(message);
    }

    virtual void handleTextMessage(const TextMessage &message) override
    {
        engine->handleTextMessage(message);
    }

    virtual void handleAckMessage(const AckMessage &message) override
    {
        engine->handleAckMessage(message);
    }
};

///////////////////////////////////////////////////////////////////////////

Engine::Engine(QObject *parent, const Settings &settings,
    const QString &ownNick, Multicaster *multicaster)
    throw (BadValueEx)
    : QObject(parent), settings(settings), ownNick(validateNick(ownNick)),
        multicaster(multicaster), messageHandler(new MessageHandler(this))
{
    connect(multicaster, SIGNAL(datagramReceived(QByteArray,QString)),
        this, SLOT(datagramReceived(QByteArray,QString)));

    contactList = new ContactList(this,
        buildContactListSettings(settings));
    connect(contactList, SIGNAL(userLeaves(QString,QString)),
        this, SIGNAL(userLeaves(QString,QString)));
    connect(contactList, SIGNAL(userJoins(QString,QString)),
        this, SIGNAL(userJoins(QString,QString)));

    connect(&advertisingTimer, SIGNAL(timeout()),
        this, SLOT(sendAdvertising()));
    advertisingTimer.setInterval(settings.advertisingPeriodMs);

    receiver.reset(new ReliableTextReceiver(
        buildReceiverSettings(settings), multicaster->getOwnId()));
}

Engine::~Engine()
{
    // Do nothing: needed by this class because it contains QScopedPointer.
}

void Engine::start()
{
    sendAdvertising();
    advertisingTimer.start();
}

void Engine::leaveChat()
{
    sendMessageIgnoringError(LeaveMessage(ownNick));
}

void Engine::sendText(const QString &text)
    throw (BadValueEx)
{
    validateText(text);

    if (sender != nullptr) {
        throw InvalidCallEx("Sending the text is not finished yet.");
    }

    sender = new ReliableTextSender(this, buildSenderSettings(settings),
        multicaster->getOwnId(), text, contactList->buildUserIds());

    connect(sender, SIGNAL(finished(QSet<QString>)),
            this, SLOT(senderFinished(QSet<QString>)));
    connect(sender, SIGNAL(needToSendText(QString,qint64)),
        this, SLOT(senderNeedToSendText(QString,qint64)));

    sender->start();
}

void Engine::senderNeedToSendText(QString text, qint64 textId)
{
    sendMessageReportingError(TextMessage(ownNick, textId, text));
}

void Engine::datagramReceived(QByteArray datagram, QString senderId)
{
    QScopedPointer<Message> pMessage;
    try {
        pMessage.reset(Message::createFromUtf8(datagram, senderId));
    } catch (ParseEx &e) {
        // Ignore unparsable datagrams.
        qDebug() << "Chat::Engine: Unable to parse received datagram:\n"
            << e.what();
        return;
    }

    if (cMessageTypesToLog.contains(pMessage->type)) {
        qDebug() << "    " << pMessage->toUtf8() << "<==="
            << qUtf8Printable(senderId);
    }

    pMessage->handleBy(messageHandler.data());
}

void Engine::sendAdvertising()
{
    sendMessageIgnoringError(UserMessage(ownNick));

    // It looks reasonable to perform this as frequently as advertising.
    contactList->removeExpiredUsers();
}

void Engine::senderFinished(QSet<QString> failedUserIds)
{
    sender->deleteLater();
    sender = nullptr;
    emit textSent(QStringList::fromSet(failedUserIds));
}

void Engine::handleUserMessage(const UserMessage &message)
{
    contactList->confirmUser(
        message.getSenderId(), message.getSenderNick());
}

void Engine::handleLeaveMessage(const LeaveMessage &message)
{
    contactList->removeUser(
        message.getSenderId(), message.getSenderNick());
}

void Engine::handleTextMessage(const TextMessage &message)
{
    sendMessageIgnoringError(
        AckMessage(message.getSenderId(), message.getTextId()));

    if (receiver->handleMessage(
        message.getSenderId(), message.getTextId())) {

        emit textReceived(message.getText(), message.getSenderNick());
    }
}

void Engine::handleAckMessage(const AckMessage &message)
{
    if (sender != nullptr) {
        sender->handleAck(message.getTextSenderId(), message.getTextId(),
            message.getSenderId());
    }
}

void Engine::sendMessageIgnoringError(const Message &message)
{
    try {
        multicaster->sendDatagram(toUtf8AndLogIfNeeded(message));
    } catch (Multicaster::NetworkEx &e) {
        // Ignore error.
        qDebug() << "Chat::Engine: Error sending datagram: " << e.what();
    }
}

void Engine::sendMessageReportingError(const Message &message)
{
    try {
        multicaster->sendDatagram(toUtf8AndLogIfNeeded(message));
    } catch (Multicaster::NetworkEx &e) {
        emit networkError(e.what());
    }
}
