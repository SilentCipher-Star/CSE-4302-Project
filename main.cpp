#include "include/mainwindow.hpp"
#include "include/logindialog.hpp"
#include "include/theme.hpp"
#include "include/csvhandler.hpp"
#include "include/databasemanager.hpp"
#include "include/utils.hpp"
#include "include/assetmanager.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Acadence");
    app.setOrganizationName("Tutu_Ali");

    // Prepare heavy assets locally to speed up active UI render loops
    AssetManager::load();

    int exitCode = 0;

    do
    {
        LoginDialog login(app);
        if (login.exec() != QDialog::Accepted)
        {
            return 0;
        }

        // Mount central dashboard using returned credentials
        MainWindow window(login.getRole(), login.getUserId(), login.getName());
        window.show();
        exitCode = app.exec();
    } while (exitCode == 99); // Relaunch if soft logout triggers app termination hook

    AssetManager::unload();
    return exitCode;
}