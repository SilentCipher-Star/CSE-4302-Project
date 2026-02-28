#include "include/mainwindow.hpp"
#include "include/logindialog.hpp"
#include "include/theme.hpp"
#include "include/csvhandler.hpp"
#include "include/utils.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Acadence");
    app.setOrganizationName("Tutu_Ali");

    Utils::loadFonts();
    CsvHandler::initialize();

    int exitCode = 0;

    do
    {
        // Login Screen
        LoginDialog login(app);
        if (login.exec() != QDialog::Accepted)
        {
            return 0;
        }

        // Main Window
        MainWindow window(login.getRole(), login.getUserId(), login.getName());
        window.show();
        exitCode = app.exec();
    } while (exitCode == 99); // the logout button is programmed to exit the application with code 99

    return exitCode;
}