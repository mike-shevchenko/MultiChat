#include "ReliableTextReceiver.h"

bool ReliableTextReceiver::handleMessage(
    const QString &senderId, qint64 messageId)
{
    if (messageId > 0) {
        // The text is the original message, sent at first attempt.
        addToRegistry(senderId, messageId);
        return true;
    }

    // NOTE: messageId = 0 will automatically be treated as a negative textId.
    if (registryContains(senderId, -messageId)) {
        // This is a duplicate message and should be ignored.
        return false;
    } else {
        // This is a duplicate message, but the original was not received.
        addToRegistry(senderId, -messageId);
        return true;
    }
}

bool ReliableTextReceiver::registryContains(
    const QString &senderId, qint64 messageId)
{
    Q_ASSERT(messageId >= 0);

    // Search backwards because the message is more likely to be found
    // close to the end.
    QListIterator<RegistryEntry> it(registry);
    it.toBack();
    while (it.hasPrevious()) {
        const RegistryEntry &entry = it.previous();
        if (entry.messageId == messageId && entry.senderId == senderId) {
            return true;
        }
    }

    return false;
}

void ReliableTextReceiver::addToRegistry(
    const QString &senderId, qint64 messageId)
{
    Q_ASSERT(messageId >= 0);

    registry.append(RegistryEntry{senderId, messageId});

    if (registry.count() > settings.maxStoredMessageRecords) {
        // Remove the oldest message.
        registry.removeFirst();
    }
}

