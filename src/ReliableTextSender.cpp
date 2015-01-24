#include "ReliableTextSender.h"

#include <QTimer>
#include <QDebug>

void ReliableTextSender::start()
{
    QElapsedTimer timeOfFirstAttempt;
    timeOfFirstAttempt.start();
    sentTextId = timeOfFirstAttempt.msecsSinceReference();

    if (userIdsToWaitAck.isEmpty()) {
        // On empty contact list, just send the message once and finish.
        emit needToSendText(text, sentTextId);
        emit finished(userIdsToWaitAck);
        return;
    }

    attemptToSendText();
}

void ReliableTextSender::attemptToSendText()
{
    if (userIdsToWaitAck.isEmpty()) {
        // Already delivered to everyone. Already emitted finish() upon
        // receiving the last ack.
        return;
    }

    ++attempt;
    // Initial attempt is 1.

    if (attempt > settings.maxAttempts) {
        // Not delivered to some users; finish.

        qDebug() << "FAIL"
            << QString::number(sentTextId) + "|" + text
            << qUtf8Printable("#" + QString::number(attempt))
            << ">>>" << userIdsToWaitAck;

        emit finished(userIdsToWaitAck);
        return;
    }

    qint64 textIdToSend = (attempt == 1) ? sentTextId : -sentTextId;

    qDebug() << "SEND"
        << QString::number(textIdToSend) + "|" + text
        << qUtf8Printable("#" + QString::number(attempt))
        << ">>>" << userIdsToWaitAck;

    emit needToSendText(text, textIdToSend);

    QTimer::singleShot(settings.attemptPeriodMs,
        this, SLOT(attemptToSendText()));
}

void ReliableTextSender::handleAck(
    const QString &textSenderId, qint64 textId, const QString &senderId)
{
    if (textSenderId != ownSenderId || abs(textId) != sentTextId
        || userIdsToWaitAck.isEmpty()) {

        return;
    }

    userIdsToWaitAck.remove(senderId);

    if (userIdsToWaitAck.isEmpty()) {
        // Delivered to everyone.
        emit finished(userIdsToWaitAck);
    }
}
