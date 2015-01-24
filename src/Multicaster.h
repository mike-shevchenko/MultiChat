#ifndef MULTICASTER_H
#define MULTICASTER_H

#include <QObject>
#include <QByteArray>
#include <QString>

// private:
#include <QHostAddress>
#include <QNetworkInterface>
class QUdpSocket;

/**
 * Mechanism which sends and receives multicast messages (unreliably).
 */
class Multicaster : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        // Should be in range 224.0.0.0 to 239.255.255.255.
        QHostAddress groupAddress = QHostAddress("239.255.42.42");

        quint16 port = 42424;

        // Debug: Non-zero values allow testing against UDP unreliability.
        int debugWasteEachNthDatagramSent = 7;
        int debugWasteEachNthDatagramReceived = 7;
    };

    static const Settings defaultSettings;

    class NetworkEx : public std::runtime_error
    {
    public:
        NetworkEx(const QString &what)
            : std::runtime_error(what.toStdString())
        {}
    };

    class NoSuitableInterfaceEx : public std::runtime_error
    {
    public:
        NoSuitableInterfaceEx(const QString &what)
            : std::runtime_error(what.toStdString())
        {}
    };

    /**
     * @throw NoSuitableInterfaceEx if no network interface is found
     * suitable for multicast communication.
     * @throw NetworkEx if any network error has occurred.
     */
    Multicaster(QObject *parent, const Settings &settings)
        throw (NetworkEx, NoSuitableInterfaceEx);

    /**
     * @return Id which other instances receive as senderId.
     */
    QString getOwnId();

    void sendDatagram(const QByteArray &datagram)
        throw (NetworkEx);

signals:
    /**
     * Receive datagrams from _other_ instances of the application, thus,
     * filtering out datagrams from a host with the same address and port.
     */
    void datagramReceived(QByteArray datagram, QString senderId);

private slots:
    void readyRead();

private:
    const Settings settings;
    QHostAddress ownIp = QHostAddress::Null;
    QUdpSocket *socket = nullptr;
    QNetworkInterface chosenIface;

    void chooseNetworkInterface()
        throw (NoSuitableInterfaceEx);
};

#endif // MULTICASTER_H
