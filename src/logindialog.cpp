#include "../include/logindialog.hpp"
#include "../include/csvhandler.hpp"
#include "../include/appmanager.hpp"
#include "../include/utils.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>

LoginDialog::LoginDialog(QApplication &app, QWidget *parent)
    : QDialog(parent), m_app(app), userId(-1), currentThemeIdx(0)
{
    setWindowTitle("Acadence");
    setModal(true);

    // Start in Full Screen
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    themes = ThemeManager::getAvailableThemes();

    ThemeManager::applyTheme(m_app, themes[currentThemeIdx]);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();

    QLabel *welcomeLabel = new QLabel("Welcome to Acadence", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(QString("font-size: %1; font-weight: bold; margin-bottom: 40px;").arg(AppFonts::Title));
    mainLayout->addWidget(welcomeLabel, 0, Qt::AlignCenter);

    QFrame *centerFrame = new QFrame(this);
    centerFrame->setFixedWidth(360);
    centerFrame->setObjectName("loginFrame");

    QVBoxLayout *frameLayout = new QVBoxLayout(centerFrame);
    frameLayout->setContentsMargins(50, 50, 50, 50);
    frameLayout->setSpacing(20);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(centerFrame);
    shadow->setBlurRadius(30);
    shadow->setXOffset(0);
    shadow->setYOffset(10);
    shadow->setColor(QColor(0, 0, 0, 60));
    centerFrame->setGraphicsEffect(shadow);

    // Define username and password fields
    userEdit = new QLineEdit(centerFrame);
    userEdit->setPlaceholderText("Username");
    userEdit->setAlignment(Qt::AlignCenter);

    passEdit = new QLineEdit(centerFrame);
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setAlignment(Qt::AlignCenter);

    // Add the username and password layouts
    frameLayout->addWidget(userEdit);
    frameLayout->addWidget(passEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, centerFrame);
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Exit");
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

    // Keyboard UX:
    // - Enter on username -> move to password
    // - Enter on password -> attempt login
    connect(userEdit, &QLineEdit::returnPressed, this, [this]()
            { passEdit->setFocus(); passEdit->selectAll(); });
    connect(passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    // Arrow-key navigation between username/password inputs
    userEdit->installEventFilter(this);
    passEdit->installEventFilter(this);

    userEdit->setFocus();

    updateThemeButton(themes[currentThemeIdx]);
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();

        if (obj == userEdit && (key == Qt::Key_Return || key == Qt::Key_Enter))
        {
            passEdit->setFocus();
            passEdit->selectAll();
            return true;
        }

        if (obj == passEdit && (key == Qt::Key_Return || key == Qt::Key_Enter))
        {
            onLoginClicked();
            return true;
        }

        if (obj == userEdit && key == Qt::Key_Down)
        {
            passEdit->setFocus();
            passEdit->selectAll();
            return true;
        }

        if (obj == passEdit && key == Qt::Key_Up)
        {
            userEdit->setFocus();
            userEdit->selectAll();
            return true;
        }
    }

    return QDialog::eventFilter(obj, event);
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

    AcadenceManager manager;
    // Exception Handling
    try
    {
        // Gets role (Admin, Student or Teacher) from user
        role = manager.login(inputUsername, inputPassword, userId);
    }
    catch (const Acadence::Exception &e)
    {
        QMessageBox::critical(this, "System Error", e.what());
        return;
    }

    if (!role.isEmpty())
    {
        if (role == Constants::Role::Admin)
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
        else if (role == Constants::Role::Student)
        {
            std::unique_ptr<Student> s(manager.getStudent(userId));
            if (s)
            {
                name = s->getName();
            }
        }
        else if (role == Constants::Role::Teacher)
        {
            std::unique_ptr<Teacher> t(manager.getTeacher(userId));
            if (t)
            {
                name = t->getName();
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

// Returns Information
QString LoginDialog::getRole() const { return role; }
int LoginDialog::getUserId() const { return userId; }
QString LoginDialog::getName() const { return name; }