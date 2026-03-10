#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QApplication>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include "appmanager.hpp"
#include "theme.hpp"

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QApplication &app, QWidget *parent = nullptr);

    QString getRole() const;
    int getUserId() const;
    QString getName() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onFloatTick();

private:
    QApplication &m_app;
    QLineEdit   *userEdit;
    QLineEdit   *passEdit;
    QPushButton *themeBtn;

    // Animation members
    QLabel  *m_welcomeLabel = nullptr;
    QLabel  *m_taglineLabel = nullptr;
    QTimer  *m_floatTimer   = nullptr;
    int      m_floatFrame   = 0;
    QVector<QPoint> m_stickerBase;

    AcadenceManager Manager;

    QString role;
    int userId;
    QString name;

    int currentThemeIdx;
    QVector<AppTheme> themes;

    void updateThemeButton(const AppTheme &t);
    void onLoginClicked();
    void onThemeClicked();
};