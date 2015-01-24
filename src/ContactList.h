#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <QObject>
#include <QString>
#include <QSet>

// private:
#include <QHash>
#include <QElapsedTimer>

/**
 * Component which keeps contact list of user id and nick (QStrings). Each
 * entry is required to be periodically confirmed, otherwise, it is removed
 * on timeout.
 */
class ContactList : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        int expiryPeriodMs;
    };

    ContactList(QObject *parent, const Settings &settings)
        : QObject(parent), settings(settings)
    {}

    /**
     * Remove the user from the contact list.
     */
    void removeUser(const QString &userId, const QString &nick);

    /**
     * Should be called periodically. Confirm that the user is active.
     */
    void confirmUser(const QString &userId, const QString &nick);

    /**
     * Should be called periodically to remove users which did not confirm
     * their presence for more that the configured period.
     */
    void removeExpiredUsers();

    QSet<QString> buildUserIds();

signals:
    /**
     * A user leaves the contact list, including when a user is considered
     * left on timeout.
     */
    void userLeaves(QString userId, QString nick);

    /**
     * A new user joins the contact list.
     */
    void userJoins(QString userId, QString nick);

private:
    Settings settings;

    struct Contact
    {
        QString nick;
        QString id;
        QElapsedTimer timeLastSeen;
    };

    // userId -> contact.
    QHash<QString, Contact> contacts;
};

#endif // CONTACTLIST_H
