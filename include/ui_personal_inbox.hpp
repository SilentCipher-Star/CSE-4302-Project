#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "appmanager.hpp"

class PersonalInboxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PersonalInboxDialog(AcadenceManager *manager, int userId, const QString &role, QWidget *parent = nullptr);

private slots:
    void onMessageSelected();
    void onDeleteClicked();
    void onRefreshClicked();

private:
    void loadMessages();

    AcadenceManager *myManager;
    int userId;
    QString userRole;

    QLabel *lblTitle;
    QLabel *lblUnread;
    QListWidget *messageList;
    QTextEdit *messageView;
    QPushButton *btnDelete;
    QPushButton *btnRefresh;

    QVector<PersonalMessage> messages;
};
