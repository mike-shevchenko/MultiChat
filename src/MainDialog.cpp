#include "MainDialog.h"

#include <QtWidgets>

#include "AboutDialog.h"
#include "Multicaster.h"
#include "ChatEngine.h"

using namespace Chat;

///////////////////////////////////////////////////////////////////////////
// Style sheet for the Chat log.

static const QString styleOwnNick = "ownNick";
static const QString styleSenderNick = "senderNick";
static const QString styleIncomingText = "incomingText";
static const QString styleOutgoingText = "outgoingText";
static const QString styleNotification = "notification";
static const QString styleError = "error";

static const QString chatLogStyleSheet =
    "." + styleOwnNick + " { color: rgb(128, 128, 128); } "
    "." + styleOutgoingText + "{ color: rgb(0, 0, 0); }"
    "." + styleSenderNick + " { color: rgb(128, 128, 224); } "
    "." + styleIncomingText + "{ color: rgb(0, 0, 192); }"
    "." + styleError + "{ color: rgb(192, 0, 0); }"
    "." + styleNotification + "{ color: rgb(64, 128, 64); }"
    "* { color: black; } ";

///////////////////////////////////////////////////////////////////////////
// Contact list.

static QString buildUserCaption(const QString &userId, const QString& nick)
{
    return nick + "@" + userId;
}

class ContactListWidgetItem : public QListWidgetItem
{
private:
    const QString userId;
    const QString nick;

public:
    ContactListWidgetItem(const QString &userId, const QString &nick)
        : QListWidgetItem(buildUserCaption(userId, nick)),
            userId(userId), nick(nick)
    {}

    QString getUserId()
    {
        return userId;
    }

    QString getNick()
    {
        return nick;
    }
};

/**
 * @return nullptr if not found.
 */
ContactListWidgetItem *MainDialog::findContactListItem(
    const QString &userId)
{
    for (int i = 0; i < contactListWidget->count(); ++i) {
        ContactListWidgetItem *item = static_cast<ContactListWidgetItem *>(
            contactListWidget->item(i));
        if (item->getUserId() == userId) {
            return item;
        }
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////

MainDialog::MainDialog(QWidget *parent, Chat::Engine *chatEngine)
    : QDialog(parent), chatEngine(chatEngine)
{
    setupUi(this);

    sendPromptLabel->setText(chatEngine->getOwnNick() + ">");

    textEdit->setFocusPolicy(Qt::StrongFocus);

    chatLogTextEdit->setFocusPolicy(Qt::NoFocus);
    chatLogTextEdit->setReadOnly(true);
    chatLogTextEdit->document()->setDefaultStyleSheet(chatLogStyleSheet);

    contactListWidget->setFocusPolicy(Qt::NoFocus);

    connect(textEdit, SIGNAL(returnPressed()),
        this, SLOT(returnPressed()));

    connect(chatEngine, SIGNAL(textReceived(QString,QString)),
        this, SLOT(textReceived(QString,QString)));
    connect(chatEngine, SIGNAL(textSent(QStringList)),
        this, SLOT(textSent(QStringList)));
    connect(chatEngine, SIGNAL(userJoins(QString,QString)),
        this, SLOT(userJoins(QString,QString)));
    connect(chatEngine, SIGNAL(userLeaves(QString,QString)),
        this, SLOT(userLeaves(QString,QString)));
    connect(chatEngine, SIGNAL(networkError(QString)),
        this, SLOT(handleError(QString)));

    connect(this, SIGNAL(finished(int)),
        chatEngine, SLOT(leaveChat()));

    chatEngine->start();
}

void MainDialog::textReceived(QString text, QString senderNick)
{
    appendText(senderNick + "> ", styleSenderNick);
    appendText(text, styleIncomingText);
    appendNewLine();
}

void MainDialog::returnPressed()
{
    QString text = textEdit->text();
    if (text.isEmpty()) {
        return;
    }

    appendText(chatEngine->getOwnNick() + "> ", styleOwnNick);
    appendText(text, styleOutgoingText);
    appendNewLine();

    textEdit->setEnabled(false);

    try {
        chatEngine->sendText(text);
    } catch (BadValueEx &e) {
        QMessageBox::critical(this, "MultiChat", tr("Text is too long."));
        textEdit->selectAll();
    }
}

void MainDialog::userJoins(QString userId, QString nick)
{
    appendLine(tr("%1 has joined.").arg(buildUserCaption(userId, nick)),
        styleNotification);
    appendNewLine();

    delete findContactListItem(userId);
    contactListWidget->addItem(new ContactListWidgetItem(userId, nick));
}

void MainDialog::userLeaves(QString userId, QString nick)
{
    appendLine(tr("%1 has left.").arg(buildUserCaption(userId, nick)),
        styleNotification);
    appendNewLine();

    delete findContactListItem(userId);
}

void MainDialog::textSent(QStringList failedUserIds)
{
    textEdit->setEnabled(true);
    textEdit->setFocus();

    if (failedUserIds.isEmpty()) {
        textEdit->clear();
    } else {
        failedUserIds.sort(Qt::CaseInsensitive);
        appendText(tr("Failed delivery to: "), styleError);
        foreach (const QString &userId, failedUserIds) {
            auto item = findContactListItem(userId);
            QString caption = buildUserCaption(userId,
                item ? item->getNick() : tr("<somebody>"));
            appendText(caption + tr("; "), styleError);
        }
        appendNewLine();

        textEdit->selectAll();
    }

    appendNewLine();
}

void MainDialog::handleError(QString errorMessage)
{
    appendLine("ERROR: " + errorMessage, styleError);
}

void MainDialog::appendNewLine()
{
    chatLogTextEdit->append("");
}

void MainDialog::appendText(const QString &text, const QString &style)
{
    chatLogTextEdit->moveCursor(QTextCursor::End);

    if (style.isEmpty()) {
        chatLogTextEdit->insertPlainText(text);
    } else {
        chatLogTextEdit->insertHtml("<span class='" + style + "'>"
            + text.toHtmlEscaped() + "</span>");
    }

    auto scrollBar = chatLogTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainDialog::appendLine(const QString &text, const QString &style)
{
    appendText(text, style);
    appendNewLine();
}

void MainDialog::aboutButtonClicked()
{
    AboutDialog::showModal(this);
}
