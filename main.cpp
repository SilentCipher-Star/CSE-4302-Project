#include "include/mainwindow.hpp"
#include "include/logindialog.hpp"
#include "include/theme.hpp"
#include "include/csvhandler.hpp"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QDir>

void initializeDataFiles()
{
    QString dataDir = CsvHandler::getDataDirectory();
    QFile adminsFile(dataDir + "admins.csv");
    if (!adminsFile.exists())
    {
        if (adminsFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&adminsFile);
            out << "1,admin,admin,System Admin,admin@school.edu\n";
            adminsFile.close();
        }
    }

    QVector<QString> otherFiles = {
        "students.csv", "teachers.csv", "courses.csv", "enrollments.csv",
        "routine.csv", "attendance.csv", "grades.csv", "notices.csv",
        "tasks.csv", "habits.csv", "queries.csv", "assessments.csv", "prayers.csv"};

    for (const QString &fileName : otherFiles)
    {
        QFile file(dataDir + fileName);
        if (!file.exists() && !file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
        }
        else
        {
            file.close();
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Acadence");
    a.setOrganizationName("MyOrganization");

    QString fontDir = QCoreApplication::applicationDirPath() + "/../fonts";
    QDir dir(fontDir);
    if (dir.exists())
    {
        QStringList filters;
        filters << "*.ttf" << "*.otf";
        dir.setNameFilters(filters);
        for (const QFileInfo &info : dir.entryInfoList())
        {
            QFontDatabase::addApplicationFont(info.absoluteFilePath());
        }
    }

    initializeDataFiles();

    int exitCode = 0;

    do
    {
        LoginDialog loginDialog(a);
        if (loginDialog.exec() != QDialog::Accepted)
        {
            return 0;
        }

        MainWindow w(loginDialog.getRole(), loginDialog.getUserId(), loginDialog.getName());
        w.show();
        exitCode = a.exec();
    } while (exitCode == 99);

    return exitCode;
}