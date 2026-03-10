#include "../include/ui_queries.hpp"
#include "../include/utils.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>

UIQueries::UIQueries(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userRole(role), userId(uid)
{
}

void UIQueries::refreshQueries()
{
    ui->listQueries->clear();
    QVector<Query> queries = myManager->getQueries(userId, userRole);

    for (const auto &q : queries)
    {
        QString label;
        if (userRole == Constants::Role::Student)
        {
            label = QString("To: %1 [%2]\nQ: %3\nA: %4").arg(q.getTeacherName(), q.getTimestamp(), q.getQuestion(), q.getAnswer().isEmpty() ? "(Waiting...)" : q.getAnswer());
        }
        else
        {
            label = QString("From: %1 [%2]\nQ: %3\nA: %4").arg(q.getStudentName(), q.getTimestamp(), q.getQuestion(), q.getAnswer().isEmpty() ? "(Select to Reply)" : q.getAnswer());
        }

        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, q.getId());
        if (q.getAnswer().isEmpty())
            item->setForeground(Qt::red);
        ui->listQueries->addItem(item);
    }
}

void UIQueries::onQueryActionClicked()
{
    QString text = ui->editQueryInput->text();
    if (text.isEmpty())
        return;

    if (userRole == Constants::Role::Student)
    {
        QVector<QPair<int, QString>> teachers = myManager->getTeacherList();
        if (teachers.isEmpty())
        {
            QMessageBox::warning(nullptr, "Error", "No teachers found to ask.");
            return;
        }

        QStringList teacherNames;
        for (const auto &t : teachers)
            teacherNames << (t.second + " (ID: " + QString::number(t.first) + ")");

        bool ok;
        QString selected = QInputDialog::getItem(nullptr, "Ask Question", "Select Teacher:", teacherNames, 0, false, &ok);
        if (ok && !selected.isEmpty())
        {
            int teacherId = teachers[teacherNames.indexOf(selected)].first;
            myManager->addQuery(userId, teacherId, text);
            ui->editQueryInput->clear();
        }
    }
    else
    {
        QListWidgetItem *item = ui->listQueries->currentItem();
        if (!item)
        {
            QMessageBox::warning(nullptr, "Selection", "Select a query to reply to.");
            return;
        }
        int qid = item->data(Qt::UserRole).toInt();
        myManager->answerQuery(qid, text);
        ui->editQueryInput->clear();
    }
}
