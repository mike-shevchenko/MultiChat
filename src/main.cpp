// To run tests, define a build configuration with the following option:
// QMAKE_CXXFLAGS+=-DRUNTESTS
#ifdef RUNTESTS

#include "RunTests.h"

int main()
{
    return runTests();
}

#else // RUNTESTS
///////////////////////////////////////////////////////////////////////////

#include <QApplication>

#include "WelcomeDialog.h"
#include "MainDialog.h"

#include "ChatEngine.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Mike Shevchenko");
    app.setApplicationName("MultiChat");

    WelcomeDialog welcomeDialog(nullptr, app.applicationName());
    if (welcomeDialog.exec() == QDialog::Rejected) {
        return 1;
    }

    MainDialog dialog(nullptr, app.applicationName(),
        welcomeDialog.getChatEngine());
    dialog.show();

    return app.exec();
}

#endif // RUNTESTS
