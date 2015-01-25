#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "ui_AboutDialog.h"

class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget *parent, const QString &title);

    static void showModal(QWidget *parent, const QString &title)
    {
        AboutDialog *dialog = new AboutDialog(parent, title);
        connect(dialog, SIGNAL(finished(int)),
            dialog, SLOT(deleteLater()));
        dialog->setModal(true);
        dialog->show();
    }
};

#endif // ABOUTDIALOG_H
