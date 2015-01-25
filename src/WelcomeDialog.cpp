#include "WelcomeDialog.h"

#include <QtWidgets>

#include "AboutDialog.h"

WelcomeDialog::WelcomeDialog(QWidget *parent, const QString &title)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle(title);

    try {
        multicaster = new Multicaster(this, Multicaster::defaultSettings);
    } catch (Multicaster::NetworkEx &e) {
        handleMulticasterError(e.what());
        return;
    } catch (Multicaster::NoSuitableInterfaceEx &e) {
        handleMulticasterError(e.what());
        return;
    }

    startButton->setEnabled(true);
    nickEdit->setEnabled(true);
    ipValueLabel->setStyleSheet(
        "QLabel { color: darkgreen; font-weight: bold; }");
    ipValueLabel->setText(multicaster->getOwnId());
}

void WelcomeDialog::handleMulticasterError(const QString &message)
{
    ipValueLabel->setStyleSheet("QLabel { color: red; }");
    ipValueLabel->setText(tr("NETWORK ERROR: ") + message);

    startButton->setEnabled(false);
    nickEdit->setEnabled(false);

    delete multicaster;
    multicaster = nullptr;
}

void WelcomeDialog::aboutButtonClicked()
{
    AboutDialog::showModal(this, windowTitle());
}

void WelcomeDialog::startButtonClicked()
{
    try {
        chatEngine = new Chat::Engine(this, Chat::Engine::defaultSettings,
            nickEdit->text(), multicaster);
    } catch (Chat::BadValueEx &) {
        QMessageBox::critical(this, windowTitle(),
            tr("Your nick should not be empty, too long or contain '|' characters."));
        nickEdit->selectAll();
        return;
    }

    emit accept();
}
