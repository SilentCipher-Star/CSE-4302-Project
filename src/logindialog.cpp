#include "../include/logindialog.hpp"
#include "../include/csvhandler.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QDialogButtonBox>
#include <QMessageBox>

LoginDialog::LoginDialog(QApplication &app, QWidget *parent)
    : QDialog(parent), m_app(app), userId(-1), currentThemeIdx(0)
{
    setWindowTitle("Acadence");
    setModal(true);
    setFixedSize(1280, 720);

    themes = ThemeManager::getAvailableThemes();

    ThemeManager::applyTheme(m_app, themes[currentThemeIdx]);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();

    QLabel *welcomeLabel = new QLabel("Welcome to Acadence", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 48pt; font-weight: bold; margin-bottom: 20px;");
    mainLayout->addWidget(welcomeLabel, 0, Qt::AlignCenter);

    QFrame *centerFrame = new QFrame(this);
    centerFrame->setFixedWidth(360);
    centerFrame->setObjectName("loginFrame");

    QVBoxLayout *frameLayout = new QVBoxLayout(centerFrame);
    frameLayout->setContentsMargins(40, 40, 40, 40);
    frameLayout->setSpacing(20);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(centerFrame);
    shadow->setBlurRadius(30);
    shadow->setXOffset(0);
    shadow->setYOffset(10);
    shadow->setColor(QColor(0, 0, 0, 60));
    centerFrame->setGraphicsEffect(shadow);

    userEdit = new QLineEdit(centerFrame);
    userEdit->setPlaceholderText("Username");
    userEdit->setAlignment(Qt::AlignCenter);

    passEdit = new QLineEdit(centerFrame);
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(userEdit);
    frameLayout->addWidget(passEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, centerFrame);
    buttonBox->setCenterButtons(true);
    frameLayout->addWidget(buttonBox);

    mainLayout->addWidget(centerFrame, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    themeBtn = new QPushButton(this);
    themeBtn->setFixedSize(40, 40);
    themeBtn->setCursor(Qt::PointingHandCursor);

    bottomLayout->addWidget(themeBtn);
    bottomLayout->setContentsMargins(0, 0, 20, 20);
    mainLayout->addLayout(bottomLayout);

    connect(themeBtn, &QPushButton::clicked, this, &LoginDialog::onThemeClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::onLoginClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateThemeButton(themes[currentThemeIdx]);
}

void LoginDialog::updateThemeButton(const AppTheme &t)
{
    themeBtn->setStyleSheet(QString("QPushButton { background-color: %1; border: 2px solid %2; border-radius: 20px; } QPushButton:hover { border-color: white; }").arg(t.accent).arg(t.background));
    themeBtn->setToolTip("Change Theme: " + t.name);

    QFrame *frame = findChild<QFrame *>("loginFrame");
    if (frame)
    {
        frame->setStyleSheet(QString(".QFrame { border: 2px solid %1; border-radius: 16px; background-color: palette(base); }").arg(t.accent));
    }
}

void LoginDialog::onThemeClicked()
{
    currentThemeIdx = (currentThemeIdx + 1) % themes.size();
    ThemeManager::applyTheme(m_app, themes[currentThemeIdx]);
    updateThemeButton(themes[currentThemeIdx]);
}

void LoginDialog::onLoginClicked()
{
    QString inputUsername = userEdit->text();
    QString inputPassword = passEdit->text();

    try
    {
        role = authManager.login(inputUsername, inputPassword, userId);
    }
    catch (const Acadence::Exception &e)
    {
        QMessageBox::critical(this, "System Error", e.what());
        return;
    }

    if (!role.isEmpty())
    {
        if (role == "Admin")
        {
            QVector<QStringList> admins = CsvHandler::readCsv("admins.csv");
            for (const auto &row : admins)
            {
                if (row.size() >= 4 && row[0].toInt() == userId)
                {
                    name = row[3];
                    break;
                }
            }
        }
        else if (role == "Student")
        {
            Student *s = authManager.getStudent(userId);
            if (s)
            {
                name = s->getName();
                delete s;
            }
        }
        else if (role == "Teacher")
        {
            Teacher *t = authManager.getTeacher(userId);
            if (t)
            {
                name = t->getName();
                delete t;
            }
        }
        accept();
    }
    else
    {
        QMessageBox::warning(this, "Login Failed", "Invalid credentials.\n");
        passEdit->clear();
        passEdit->setFocus();
    }
}

QString LoginDialog::getRole() const { return role; }
int LoginDialog::getUserId() const { return userId; }
QString LoginDialog::getName() const { return name; }