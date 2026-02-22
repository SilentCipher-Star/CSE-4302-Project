#ifndef LOGINDIALOG_HPP
#define LOGINDIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include "academicmanager.hpp"
#include "theme.hpp"

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QApplication &app, QWidget *parent = nullptr);

    QString getRole() const;
    int getUserId() const;
    QString getName() const;

private:
    QApplication &m_app;
    QLineEdit *userEdit;
    QLineEdit *passEdit;
    QPushButton *themeBtn;

    AcadenceManager authManager;

    QString role;
    int userId;
    QString name;

    int currentThemeIdx;
    QVector<AppTheme> themes;

    void updateThemeButton(const AppTheme &t);
    void onLoginClicked();
    void onThemeClicked();
};

#endif // LOGINDIALOG_HPP