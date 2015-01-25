#include "AboutDialog.h"

#include <QtWidgets>

AboutDialog::AboutDialog(QWidget *parent, const QString &title)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle(title);
}
