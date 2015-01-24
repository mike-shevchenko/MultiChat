#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "ui_AboutDialog.h"

class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = 0);

    static void showModal(QWidget *parent = 0)
    {
        AboutDialog *dialog = new AboutDialog(parent);
        connect(dialog, SIGNAL(finished(int)),
            dialog, SLOT(deleteLater()));
        dialog->setModal(true);
        dialog->show();
    }
};

#endif // ABOUTDIALOG_H
