#ifndef RELIABLETEXTRECEIVER_H
#define RELIABLETEXTRECEIVER_H

#include <QString>
#include <QList>

/**
 * Component which filters out messages received using an unreliable
 * receiving mechanism which can produce duplicates.
 */
class ReliableTextReceiver
{
public:
    struct Settings
    {
        // Old messages are forgot after the registry exceeds this length.
        int maxStoredMessageRecords;
    };

    ReliableTextReceiver(
        const Settings &settings, const QString &ownSenderId)
        : settings(settings), ownSenderId(ownSenderId)
    {}

    /**
     * Should be called each time a message is received from a user.
     * Duplicate message has messageId < 0, abs-equal to the original
     * (sent at the first attempt) messageId.
     * @return Whether the message is considered to be received for the
     * first time and thus needs to be handled (otherwise, should be
     * skipped).
     */
    bool handleMessage(const QString &senderId, qint64 messageId);

private:
    const Settings settings;
    const QString ownSenderId;

    struct RegistryEntry
    {
        QString senderId;
        qint64 messageId;
    };

    QList<RegistryEntry> registry;

    bool registryContains(const QString &senderId, qint64 messageId);
    void addToRegistry(const QString &senderId, qint64 messageId);
};

#endif // RELIABLETEXTRECEIVER_H
