#ifndef RELIABLETEXTRECEIVERTEST_H
#define RELIABLETEXTRECEIVERTEST_H

#include <QtTest>

#include "ReliableTextReceiver.h"

class ReliableTextReceiverTest : public QObject
{
    Q_OBJECT
private:
    void allows(ReliableTextReceiver &r,
        const QString &senderId, qint64 messageId,
        const char *lineName)
    {
        QVERIFY2(r.handleMessage(senderId, messageId), lineName);
    }

    void rejects(ReliableTextReceiver &r,
        const QString &senderId, qint64 messageId,
        const char *lineName)
    {
        QVERIFY2(!r.handleMessage(senderId, messageId), lineName);
    }

private slots:

    void testSimpleCase()
    {
        ReliableTextReceiver r(ReliableTextReceiver::Settings{3}, "ID");
        allows(r, "a", 10, "orig");
        allows(r, "f", 10, "fill10");
        rejects(r, "a", -10, "rejects dup");
    }

    void testExpiration()
    {
        ReliableTextReceiver r(ReliableTextReceiver::Settings{3}, "ID");
        allows(r, "a", 10, "orig");
        allows(r, "f", 10, "fill10");
        rejects(r, "a", -10, "rejects dup1");
        allows(r, "f", 11, "fill11");
        allows(r, "f", 12, "fill12");
        allows(r, "f", 13, "fill13");
        allows(r, "a", -10, "allows dup2: dup1 expired");
    }

    void testAllowedDuplicatesAreRegistered()
    {
        ReliableTextReceiver r(ReliableTextReceiver::Settings{3}, "ID");
        allows(r, "f", 10, "fill10");
        allows(r, "a", -10, "allows dup1");
        allows(r, "f", 11, "fill11");
        rejects(r, "a", -10, "rejects dup2");
    }
};

#endif // RELIABLETEXTRECEIVERTEST_H

