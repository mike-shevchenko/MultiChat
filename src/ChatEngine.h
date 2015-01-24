#ifndef CHATENGINE_H
#define CHATENGINE_H

// Component which implements the business logic of a local network chat.

#include <stdexcept>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
class Multicaster;

// private:
class ContactList;
class ReliableTextSender;
class ReliableTextReceiver;

namespace Chat {

// private:
class Message;
class UserMessage;
class LeaveMessage;
class TextMessage;
class AckMessage;

class InvalidCallEx : public std::logic_error
{
public:
    InvalidCallEx(const QString &what)
        : std::logic_error(what.toStdString())
    {}
};

class BadValueEx : public std::invalid_argument
{
public:
    BadValueEx(const QString &what)
        : std::invalid_argument(what.toStdString())
    {}
};

/**
 * The principles of the chat are as follows:
 * - A number of identical instances of this application is running in a
 *   LAN segment which supports multicast; there is no dedicated server
 *   software. All such instances (hereby called Apps) configured with the
 *   same multicast group address form a single and only chat channel.
 * - Each App periodically announces its presence via multicast, and
 *   populates its contact list with such announcements received from other
 *   Apps.
 * - When an App sends a message, it is delivered to all other Apps.
 *   Sender's nick and IP address are included with the message.
 * - Messages are guaranteed to be delivered (via waiting for an
 *   acknowledgement and resending on timeout) to the Apps which were
 *   on the contact list of the sender at the moment of sending.
 *
 * Here are the current implementation limitations:
 * - Nick length in UTF-8 should not exceed 64 bytes, and it should neither
 *   contain ASCII control codes, nor '|'. Nicks need not be unique.
 * - Message length in UTF-8 should not exceed 255 bytes.
 */
class Engine : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        int textMaxAttempts = 3;
        int textAttemptPeriodMs = 1000;
        int textMaxStoredRecords = 10;
        int advertisingPeriodMs = 5000;
        int contactExpiryPeriodMs = 11000;
    };

    static const Settings defaultSettings;

    /**
     * @param multicaster Should be started before Manager::start().
     * @throw BadValueEx if ownNick is empty, too long, or contains '|'.
     */
    Engine(QObject *parent, const Settings &settings,
        const QString &ownNick, Multicaster *multicaster)
        throw (BadValueEx);

    virtual ~Engine() override;

    QString getOwnNick() const
    {
        return ownNick;
    }

    /**
     * Should be called once after the signals are connected.
     */
    void start();

    /**
     * Asynchronously send the text to all possible recepients (users).
     * After the delivery is finished (either successfully or maybe having
     * failed for certain recepients), textSent() event is called. Before
     * this signal is emitted, subsequent calls to sendText() should not
     * be attempted (throw InvalidCallEx).
     * @throw BadValueEx if text is too long.
     */
    void sendText(const QString &text)
        throw (BadValueEx);

public slots:
    /**
     * Should be called before the App is closed.
     */
    void leaveChat();

signals:   
    void textReceived(QString text, QString senderNick);

    /**
     * Sending the text is considered finished: all known recepients have
     * acknowledged the reception, or a timeout has passed.
     * ATTENTION: Any of the failedUsers may be missing in the contact list
     * at the time this signal is handled.
     */
    void textSent(QStringList failedUserIds);

    /**
     * A user leaves the chat, including when a user is considered left on
     * unavailability timeout.
     */
    void userLeaves(QString userId, QString nick);

    void userJoins(QString userId, QString nick);

    void networkError(QString errorMessage);

private slots:
    void datagramReceived(QByteArray datagram, QString senderId);
    void sendAdvertising();
    void senderNeedToSendText(QString text, qint64 textId);
    void senderFinished(QSet<QString> failedUserIds);

private:    
    const Settings settings;
    const QString ownNick;

    // Neither created nor owned here.
    Multicaster *multicaster = nullptr;

    // Created and owned here, is QObject.
    ContactList *contactList = nullptr;

    // Created for each message to send; deleted after sending finishes.
    ReliableTextSender *sender = nullptr;

    // Created and owned here.
    QScopedPointer<ReliableTextReceiver> receiver;

    QTimer advertisingTimer;

    // Handling messages using a Visitor-pattern adapter.
    class MessageHandler;
    QScopedPointer<MessageHandler> messageHandler;
    void handleUserMessage(const UserMessage &message);
    void handleLeaveMessage(const LeaveMessage &message);
    void handleTextMessage(const TextMessage &message);
    void handleAckMessage(const AckMessage &message);

    void sendMessageIgnoringError(const Message &message);
    void sendMessageReportingError(const Message &message);
};

} // namespace Chat

#endif // CHATENGINE_H
