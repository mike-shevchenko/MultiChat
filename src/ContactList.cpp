#include "ContactList.h"

void ContactList::removeUser(const QString &userId, const QString &nick)
{
    contacts.remove(userId);
    emit userLeaves(userId, nick);
}

void ContactList::confirmUser(const QString &userId, const QString &nick)
{
    Contact &contact = contacts[userId];
    // If not found, created and added to the map by operator[].
    contact.id = userId;

    contact.timeLastSeen.start();

    if (contact.nick != nick) {
        if (contact.nick != "") {
            // The user has new nick, e.g. his App has been restarted.
            emit userLeaves(contact.id, contact.nick);
        }
        contact.nick = nick;
        emit userJoins(contact.id, contact.nick);
    }
}

void ContactList::removeExpiredUsers()
{
    QMutableHashIterator<QString, Contact> it(contacts);
    while (it.hasNext()) {
        it.next();
        if (it.value().timeLastSeen.hasExpired(
            settings.expiryPeriodMs)) {

            emit userLeaves(it.value().id, it.value().nick);
            it.remove();
        }
    }
}

QSet<QString> ContactList::buildUserIds()
{
    QSet<QString> userIds;
    foreach (const Contact &contact, contacts) {
        userIds.insert(contact.id);
    }
    return userIds;
}
