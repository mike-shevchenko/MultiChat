#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include "ui_WelcomeDialog.h"

#include "Multicaster.h"
#include "ChatEngine.h"

/**
 * Create Multicaster and Chat::Manager, asking the user for nick and
 * reporting possible errors. These components are owned by this dialog.
 *
 * After the dialog is accepted, the owned Chat::Manager is created but not
 * started.
 */
class WelcomeDialog : public QDialog, private Ui::WelcomeDialog
{
    Q_OBJECT
public:
    explicit WelcomeDialog(QWidget *parent = 0);

    Chat::Engine *getChatEngine()
    {
        return chatEngine;
    }

private slots:
    void aboutButtonClicked();
    void startButtonClicked();

private:
    // Created and owned here as QObjects.
    Multicaster *multicaster = nullptr;
    Chat::Engine *chatEngine = nullptr;

    void handleMulticasterError(const QString &message);
};

#endif // WELCOMEDIALOG_H
