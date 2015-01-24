#ifndef CHATMESSAGESTEST_H
#define CHATMESSAGESTEST_H

#include <QtTest>

#include "ChatMessages.h"

using namespace Chat;

class ChatMessageTest : public QObject
{
    Q_OBJECT
private:
    static void testMessageInvalid(const QString &s)
    {
        try
        {
            QScopedPointer<Message>(
                Message::createFromUtf8(s.toUtf8(), "TEST_senderId"));
        }
        catch (ParseEx &)
        {
            // OK.
            return;
        }

        QFAIL(("Invalid message string parsed successfully:\n"
            + s).toUtf8().constData());
    }

    template<class T>
    static void testMessageValid(const QString &s)
    {
        try
        {
            QScopedPointer<Message> m(
                Message::createFromUtf8(s.toUtf8(), "TEST_senderId"));

            QVERIFY(m->getSenderId() == "TEST_senderId");

            // Test that the message is of the expected type.
            QVERIFY(dynamic_cast<T *>(m.data()) != 0);

            QCOMPARE(m->toUtf8(), s.toUtf8());
        }
        catch (ParseEx &e)
        {
            QFAIL(("Failed parsing message:\n" +
                 s + "\n" +
                 e.what()).toUtf8().constData());
        }
    }

private slots:
    void testGenericMessageInvalid()
    {
        QFETCH(QString, s);
        testMessageInvalid(s);
    }

    void testUserMessageInvalid()
    {
        QFETCH(QString, s);
        testMessageInvalid(s);
    }

    void testLeaveMessageInvalid()
    {
        QFETCH(QString, s);
        testMessageInvalid(s);
    }

    void testTextMessageInvalid()
    {
        QFETCH(QString, s);
        testMessageInvalid(s);
    }

    void testAckMessageInvalid()
    {
        QFETCH(QString, s);
        testMessageInvalid(s);
    }

    void testUserMessageValid()
    {
        QFETCH(QString, s);
        testMessageValid<UserMessage>(s);
    }

    void testLeaveMessageValid()
    {
        QFETCH(QString, s);
        testMessageValid<LeaveMessage>(s);
    }

    void testTextMessageValid()
    {
        QFETCH(QString, s);
        testMessageValid<TextMessage>(s);
    }

    void testAckMessageValid()
    {
        QFETCH(QString, s);
        testMessageValid<AckMessage>(s);
    }

    ///////////////////////////////////////////////////////////////////////

    void testGenericMessageInvalid_data()
    {
        QTest::addColumn<QString>("s");

        // Generic unparsable strings.
        QTest::newRow("1") << "unknown|message";
        QTest::newRow("2") << "incomplete";
        QTest::newRow("3") << "";
        QTest::newRow("4") << "\n";
        QTest::newRow("5") << "|";
        QTest::newRow("6") << "|1";
    }

    void testUserMessageInvalid_data()
    {
        QTest::addColumn<QString>("s");

        // user|<sender.nick>

        QTest::newRow("user: no fields")
            << "user";
        QTest::newRow("user: empty sender.nick")
            << "user|";
        QTest::newRow("user: extra empty field")
            << "user|nick|";
        QTest::newRow("user: extra field")
            << "user|nick|1";
    }

    void testLeaveMessageInvalid_data()
    {
        QTest::addColumn<QString>("s");

        // leave|<sender.nick>

        QTest::newRow("leave: no fields")
            << "leave";
        QTest::newRow("leave: empty sender.nick")
            << "leave|";
        QTest::newRow("leave: extra empty field")
            << "leave|nick|";
        QTest::newRow("leave: extra field")
            << "leave|nick|1";
    }

    void testTextMessageInvalid_data()
    {
        QTest::addColumn<QString>("s");

        // text|<sender.nick>|<text.id>|<text>

        QTest::newRow("text: no fields")
            << "text";
        QTest::newRow("text: bad text.id")
            << "text|nick|xxx|text";
        QTest::newRow("text: too large text.id")
            << "text|nick|9223372036854775808|text";
        QTest::newRow("text: too low negative text.id")
            << "text|nick|-9223372036854775809|text";
        QTest::newRow("text: only 1 field")
            << "text|1";
        QTest::newRow("text: only 2 fields")
            << "text|1|2";
        QTest::newRow("text: empty nick")
            << "text||1|text";
        QTest::newRow("text: empty text.id")
            << "text|nick||text";
        QTest::newRow("text: empty nick and text.id")
            << "text|||1";
        QTest::newRow("text: empty nick, text.id and text")
            << "text|||";
    }

    void testAckMessageInvalid_data()
    {
        QTest::addColumn<QString>("s");

        // ack|<text.sender.id>|<text.id>

        QTest::newRow("ack: empty text.sender.id")
            << "ack||1";
        QTest::newRow("ack: empty text.id")
            << "ack|1|";
        QTest::newRow("ack: empty text.sender.id and text.id")
            << "ack||";
        QTest::newRow("ack: extra field")
            << "ack|1|2|3";
        QTest::newRow("ack: extra empty field")
            << "ack|1|2|";
        QTest::newRow("ack: bad text.id")
            << "ack|1|xxx";
        QTest::newRow("ack: too large text.id")
            << "ack|1.1.1.1|9223372036854775808";
        QTest::newRow("ack: too low negative text.id")
            << "ack|1.1.1.1|-9223372036854775809";
    }

    void testUserMessageValid_data()
    {
        QTest::addColumn<QString>("s");

        // user|<sender.nick>
        QTest::newRow("user: typical") << "user|Bob Marley";
    }

    void testLeaveMessageValid_data()
    {
        QTest::addColumn<QString>("s");

        // leave|<sender.nick>
        QTest::newRow("leave: typical") << "leave|Jane J. Doe";
    }

    void testTextMessageValid_data()
    {
        QTest::addColumn<QString>("s");

        // text|<sender.nick>|<text.id>|<text>

        QTest::newRow("text: typical")
            << "text|John Doe|113326|some text";
        QTest::newRow("text: zero text.id")
            << "text|nick|0|text";
        QTest::newRow("text: max text.id")
            << "text|nick|9223372036854775807|text";
        QTest::newRow("text: min negative text.id")
            << "text|nick|-9223372036854775808|text";
        QTest::newRow("text: text with '|'")
            << "text|nick|1|some text with '|' char";
        QTest::newRow("text: empty text")
            << "text|nick|1|";
        QTest::newRow("text: text with new-line")
            << "text|nick|1|a\nb";
    }

    void testAckMessageValid_data()
    {
        QTest::addColumn<QString>("s");

        // ack|<text.sender.id>|<text.id>

        QTest::newRow("ack: typical")
            << "ack|192.168.1.100|113326";
        QTest::newRow("ack: zero text.id")
            << "ack|1.1.1.1|0";
        QTest::newRow("ack: max text.id")
            << "ack|1.1.1.1|9223372036854775807";
        QTest::newRow("ack: min negative text.id")
            << "ack|1.1.1.1|-9223372036854775808";
    }
};

#endif // CHATMESSAGESTEST_H

