#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include "ui_MainDialog.h"

// private:
namespace Chat { class Engine; }
class ContactListWidgetItem;

class MainDialog : public QDialog, private Ui::MainDialog
{
    Q_OBJECT
public:
    MainDialog(QWidget *parent, Chat::Engine *chatEngine);

private slots:
    void textReceived(QString text, QString senderNick);
    void textSent(QStringList failedUserIds);
    void userLeaves(QString userId, QString nick);
    void userJoins(QString userId, QString nick);
    void handleError(QString errorMessage);

    void returnPressed();
    void aboutButtonClicked();

private:
    Chat::Engine *chatEngine;

    ContactListWidgetItem *findContactListItem(const QString &userId);
    void appendLine(const QString &text, const QString &style);
    void appendText(const QString &text, const QString &style);
    void appendNewLine();
};

#endif // MAINDIALOG_H
