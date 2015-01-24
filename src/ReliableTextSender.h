#ifndef RELIABLETEXTSENDER_H
#define RELIABLETEXTSENDER_H

// Component which reliably sends a text message.

#include <QObject>
#include <QString>
#include <QSet>
#include <QElapsedTimer>

/**
 * Component which reliably sends a text message using an unreliable
 * sending mechanism: the text is sent possibly several times until it is
 * acked by all of the users.
 *
 * This component does not perform actual text sending and ack receiving:
 * it rather emits sendText() signal and offers handleAck() method to
 * delegate these actions to its user.
 *
 * A new object of this class should be created for sending new text, and
 * can be deleted via deleteLater() after it emits finished().
 */
class ReliableTextSender : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        int maxAttempts;
        int attemptPeriodMs;
    };

    ReliableTextSender(QObject *parent,
        const Settings &settings,
        const QString &ownSenderId, const QString &text,
        const QSet<QString> &userIdsToWaitAck)
        : QObject(parent),
            settings(settings), ownSenderId(ownSenderId), text(text),
            userIdsToWaitAck(userIdsToWaitAck)
    {}

    /**
     * Should be called once, after signals are connected.
     */
    void start();

    /**
     * Should be called each time an ack is received from a user.
     */
    void handleAck(const QString &textSenderId, qint64 textId,
        const QString &senderId);

signals:
    /**
     * Emitted when an attempt to send the text should be performed.
     * @param textId is assigned by this component.
     */
    void needToSendText(QString text, qint64 textId);

    /**
     * Emitted when the text is acked by all users (then failedUserIds is
     * empty), or the timeout has expired (then failedUserIds contains Ids
     * of users which have not sent an ack). Upon handling of this signal,
     * the object can be safely deleted.
     */
    void finished(QSet<QString> failedUserIds);

private slots:
    void attemptToSendText();

private:
    const Settings settings;
    const QString ownSenderId;
    const QString text;

    QSet<QString> userIdsToWaitAck;

    int attempt = 0;

    // Time stamp of first sending attempt is used as textId for the first
    // attempt, and further attempts use its negated value as textId.
    qint64 sentTextId = 0;
};

#endif // RELIABLETEXTSENDER_H
