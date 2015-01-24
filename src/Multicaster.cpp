#include "Multicaster.h"

#include <QtNetwork>

// Logs regular datagrams.
#define LOG(ARGS) \
//    qDebug() << ARGS

// Allows to log datagrams "wasted" as requested by debug settings.
#define DEBUG_LOG_WASTED(ARGS) \
    qDebug() << ARGS

const Multicaster::Settings Multicaster::defaultSettings;

Multicaster::Multicaster(QObject *parent, const Settings &settings)
    throw (NetworkEx, NoSuitableInterfaceEx)
    : QObject(parent), settings(settings)

{
    chooseNetworkInterface();

    socket = new QUdpSocket(this);

    if (!socket->bind(ownIp, settings.port,
        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {

        throw NetworkEx("Unable to bind UDP socket to port " +
            QString::number(settings.port) +
            " on iface with own IP " +
            ownIp.toString() + ".");
    }

    if (!socket->joinMulticastGroup(settings.groupAddress, chosenIface)) {
        throw NetworkEx("Unable to join multicast group "
            + settings.groupAddress.toString() + " on iface \""
            + chosenIface.name() + "\".");
    }

    connect(socket, SIGNAL(readyRead()),
        this, SLOT(readyRead()));
}

void Multicaster::sendDatagram(const QByteArray &datagram)
    throw (NetworkEx)
{
    // Debug.
    if (settings.debugWasteEachNthDatagramSent > 0)
    {
        static int count = 0;
        if (++count % settings.debugWasteEachNthDatagramSent == 0)
        {
            DEBUG_LOG_WASTED("-x->" << datagram);
            return;
        }
    }

    LOG("--->" << datagram);

    qint64 r = socket->writeDatagram(
        datagram, settings.groupAddress, settings.port);
    if (r == -1) {
        throw NetworkEx("Unable to send datagram.");
    }
    else if (r != datagram.size()) {
        throw NetworkEx("Unable to send datagram of " +
            QString::number(datagram.size()) + " bytes.");
    }
}

void Multicaster::readyRead()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());

        QHostAddress senderAddr;
        quint16 senderPort;

        // Here it is assumed that if a datagram is fragmented, it can
        // be dismissed and treated as a UDP unreliability
        qint64 r = socket->readDatagram(datagram.data(), datagram.size(),
            &senderAddr, &senderPort);
        if (r != datagram.size())
        {
            // Ignore errors and datagrams of incorrect size.
            qDebug() << "Multicaster::readyRead()"
                << "readDatagram() returned" << r << ", but"
                << datagram.size() << "expected. Ignored.";
            return;
        }

        if (senderPort != settings.port) {
            // Ignore datagrams sent from unknown ports.
            qDebug() << "Multicaster::readyRead()"
                << "Received datagram from port" << senderPort
                << ", but expected port is" << settings.port
                << ". Ignored.";
            return;
        }

        if (senderAddr == ownIp) {
            // Ignore datagrams sent by this host to itself.
            return;
        }

        // Debug.
        if (settings.debugWasteEachNthDatagramReceived > 0)
        {
            static int count = 0;
            if (++count % settings.debugWasteEachNthDatagramReceived == 0)
            {
                DEBUG_LOG_WASTED("    " << datagram << "<-x-"
                    << qUtf8Printable(senderAddr.toString()));
                return;
            }
        }

        LOG("    " << datagram << "<---"
            << qUtf8Printable(senderAddr.toString()));

        emit datagramReceived(datagram, senderAddr.toString());
    }
}

QString Multicaster::getOwnId()
{
    return ownIp.toString();
}

/**
 * Limitations of the current implementation: if there is more than one
 * suitable network interfaces, an exception is thrown. In future, some
 * mean of allowing the user to choose among them can be provided.
 */
void Multicaster::chooseNetworkInterface()
    throw (NoSuitableInterfaceEx)
{
    ownIp = QHostAddress::Null;

    foreach (const QNetworkInterface &iface,
        QNetworkInterface::allInterfaces()) {

        if (iface.flags() & QNetworkInterface::IsUp
            && iface.flags() & QNetworkInterface::IsRunning) {

            foreach (const QNetworkAddressEntry &entry,
                iface.addressEntries()) {

                if (entry.ip().protocol() == QUdpSocket::IPv4Protocol
                    && entry.ip() != QHostAddress::LocalHost) {

                    if (!ownIp.isNull()) {
                        throw NoSuitableInterfaceEx(
                            "More than one suitable network interfaces found.");
                    }
                    chosenIface = iface;
                    ownIp = entry.ip();
                }
            }
        }
    }

    if (ownIp.isNull()) {
        throw NoSuitableInterfaceEx("No suitable networks found.");
    }

    qDebug() << "Multicaster::chooseNetworkInterface() ownIp:"
         << ownIp << "; chosenIface:" << chosenIface;
}

