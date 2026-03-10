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
#include <QScreen>
#include <QTimer>

// ── Sticker definitions: symbol · x-ratio · y-ratio · font-size (px) ──────
struct StickerDef { QString text; float xr, yr; int size; };
static const QVector<StickerDef> k_stickers = {
    { "\xe2\x98\x85",  0.04f, 0.07f, 56 },
    { "\xe2\x9c\xa6",  0.94f, 0.11f, 40 },
    { "\xe2\x9d\x80",  0.08f, 0.82f, 48 },
    { "\xe2\x9c\xa7",  0.91f, 0.80f, 52 },
    { "\xc2\xb7",      0.14f, 0.44f, 44 },
    { "\xe2\x9c\xa6",  0.85f, 0.48f, 36 },
    { "\xe2\x98\x85",  0.22f, 0.90f, 32 },
    { "\xe2\x99\xa5",  0.78f, 0.12f, 42 },
    { "\xe2\x98\x85",  0.50f, 0.04f, 48 },
    { "\xe2\x9c\xa7",  0.50f, 0.94f, 36 },
    { "\xc2\xb7",      0.35f, 0.20f, 30 },
    { "\xe2\x9c\xa6",  0.65f, 0.75f, 28 },
    { "\xe2\x98\x85",  0.88f, 0.55f, 34 },
    { "\xe2\x99\xa5",  0.12f, 0.60f, 30 },
};

LoginDialog::LoginDialog(QApplication &app, QWidget *parent)
    : QDialog(parent), m_app(app), userId(-1), currentThemeIdx(0)
{
    setWindowTitle("Acadence");
    setModal(true);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    themes = ThemeManager::getAvailableThemes();
    ThemeManager::applyTheme(m_app, themes[currentThemeIdx]);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addStretch(2);

    // Decorative top sparkle row
    QLabel *decoTop = new QLabel(QString::fromUtf8("\xe2\x9c\xbf   \xe2\x9c\xa6   \xcb\x9a \xc2\xb7 \xcb\x9a   \xe2\x9c\xa6   \xe2\x9c\xbf"), this);
    decoTop->setAlignment(Qt::AlignCenter);
    decoTop->setObjectName("decoLabel");
    decoTop->setProperty("baseStyle",
        "font-size:30px; letter-spacing:14px; background:transparent;");
    mainLayout->addWidget(decoTop, 0, Qt::AlignCenter);

    mainLayout->addSpacing(10);

    // App title
    QLabel *welcomeLabel = new QLabel("Acadence", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(
        QString("font-size:%1; font-weight:bold; letter-spacing:4px; background:transparent;")
            .arg(AppFonts::Title));
    mainLayout->addWidget(welcomeLabel, 0, Qt::AlignCenter);

    // Dreamy subtitle
    QLabel *taglineLabel = new QLabel(
        QString::fromUtf8("\xe2\x9c\xa7  your campus  \xc2\xb7  your schedule  \xc2\xb7  your success  \xe2\x9c\xa7"), this);
    taglineLabel->setAlignment(Qt::AlignCenter);
    taglineLabel->setObjectName("decoLabel");
    taglineLabel->setProperty("baseStyle",
        "font-size:17px; font-weight:400; letter-spacing:1.2px; background:transparent;");
    mainLayout->addWidget(taglineLabel, 0, Qt::AlignCenter);

    mainLayout->addSpacing(24);

    // Login card
    QFrame *centerFrame = new QFrame(this);
    centerFrame->setFixedWidth(490);
    centerFrame->setObjectName("loginFrame");

    QVBoxLayout *frameLayout = new QVBoxLayout(centerFrame);
    frameLayout->setContentsMargins(52, 44, 52, 44);
    frameLayout->setSpacing(16);

    // Gradient accent bar at top of card
    QFrame *accentBar = new QFrame(centerFrame);
    accentBar->setFixedHeight(5);
    accentBar->setObjectName("loginAccentBar");
    frameLayout->addWidget(accentBar);

    frameLayout->addSpacing(6);

    // Card heading
    QLabel *cardTitle = new QLabel(QString::fromUtf8("\xe2\x9c\xa6  Sign In  \xe2\x9c\xa6"), centerFrame);
    cardTitle->setAlignment(Qt::AlignCenter);
    cardTitle->setObjectName("cardDecoLabel");
    cardTitle->setProperty("baseStyle",
        "font-size:27px; font-weight:700; letter-spacing:2px; background:transparent;");
    frameLayout->addWidget(cardTitle);

    frameLayout->addSpacing(4);

    // Drop shadow on card
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(centerFrame);
    shadow->setBlurRadius(70);
    shadow->setXOffset(0);
    shadow->setYOffset(22);
    shadow->setColor(QColor(0, 0, 0, 45));
    centerFrame->setGraphicsEffect(shadow);

    // Input fields
    userEdit = new QLineEdit(centerFrame);
    userEdit->setPlaceholderText("  Username");
    userEdit->setAlignment(Qt::AlignCenter);
    userEdit->setMinimumHeight(52);

    passEdit = new QLineEdit(centerFrame);
    passEdit->setPlaceholderText("  Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setAlignment(Qt::AlignCenter);
    passEdit->setMinimumHeight(52);

    frameLayout->addWidget(userEdit);
    frameLayout->addWidget(passEdit);
    frameLayout->addSpacing(6);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, centerFrame);
    buttonBox->button(QDialogButtonBox::Ok)->setText("  Sign In  ");
    buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(52);
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Exit");
    buttonBox->setCenterButtons(true);
    frameLayout->addWidget(buttonBox);

    mainLayout->addWidget(centerFrame, 0, Qt::AlignCenter);
    mainLayout->addStretch(2);

    // Cute footer
    QLabel *footer = new QLabel(QString::fromUtf8("\xcb\x9a \xc2\xb7  made with love  \xc2\xb7 \xcb\x9a"), this);
    footer->setAlignment(Qt::AlignCenter);
    footer->setObjectName("decoLabel");
    footer->setProperty("baseStyle",
        "font-size:15px; letter-spacing:2.5px; background:transparent;");
    mainLayout->addWidget(footer, 0, Qt::AlignCenter);

    mainLayout->addSpacing(8);

    // Theme toggle button
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    themeBtn = new QPushButton(this);
    themeBtn->setFixedSize(46, 46);
    themeBtn->setCursor(Qt::PointingHandCursor);
    themeBtn->setToolTip("Change Theme");
    bottomLayout->addWidget(themeBtn);
    bottomLayout->setContentsMargins(0, 0, 24, 22);
    mainLayout->addLayout(bottomLayout);

    // Floating stickers (created hidden, positioned in showEvent)
    for (const auto &s : k_stickers)
    {
        QLabel *lbl = new QLabel(s.text, this);
        lbl->setObjectName("stickerLabel");
        lbl->setAttribute(Qt::WA_TranslucentBackground);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setProperty("stickerSize", s.size);
        lbl->setProperty("xRatio", s.xr);
        lbl->setProperty("yRatio", s.yr);
        lbl->resize(s.size + 24, s.size + 24);
        lbl->hide();
    }

    // Connections
    connect(themeBtn,  &QPushButton::clicked,       this, &LoginDialog::onThemeClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::onLoginClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(userEdit, &QLineEdit::returnPressed, this, [this]()
        { passEdit->setFocus(); passEdit->selectAll(); });
    connect(passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    userEdit->installEventFilter(this);
    passEdit->installEventFilter(this);
    userEdit->setFocus();

    updateThemeButton(themes[currentThemeIdx]);
}

// ──────────────────────────────────────────────────────────────────────────
bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke  = static_cast<QKeyEvent *>(event);
        const int  key = ke->key();

        if (obj == userEdit && (key == Qt::Key_Return || key == Qt::Key_Enter ||
                                 key == Qt::Key_Down))
        { passEdit->setFocus(); passEdit->selectAll(); return true; }

        if (obj == passEdit && (key == Qt::Key_Return || key == Qt::Key_Enter))
        { onLoginClicked(); return true; }

        if (obj == passEdit && key == Qt::Key_Up)
        { userEdit->setFocus(); userEdit->selectAll(); return true; }
    }
    return QDialog::eventFilter(obj, event);
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QTimer::singleShot(100, this, [this]() {
        int w = width();
        int h = height();
        for (QLabel *lbl : findChildren<QLabel *>("stickerLabel"))
        {
            int sz = lbl->property("stickerSize").toInt();
            float xr = lbl->property("xRatio").toFloat();
            float yr = lbl->property("yRatio").toFloat();
            lbl->setStyleSheet(QString(
                "font-size:%1px; color:%2; background:transparent; "
                "font-family:'Segoe UI Emoji','Segoe UI Symbol',sans-serif;")
                .arg(sz).arg(themes[currentThemeIdx].accent));
            lbl->move(int(w * xr) - (sz + 24) / 2,
                      int(h * yr) - (sz + 24) / 2);
            lbl->show();
            lbl->raise();
        }
    });
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::updateThemeButton(const AppTheme &t)
{
    themeBtn->setStyleSheet(QString(
        "QPushButton {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);"
        "  border:2px solid %2; border-radius:23px; font-size:22px; }"
        "QPushButton:hover { border-color:white; }"
    ).arg(t.accent, t.background));
    themeBtn->setToolTip(QString("Theme: %1").arg(t.name));

    QFrame *frame = findChild<QFrame *>("loginFrame");
    if (frame)
    {
        frame->setStyleSheet(QString(
            ".QFrame#loginFrame {"
            "  border:2px solid %1;"
            "  border-radius:28px;"
            "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "    stop:0 %2, stop:1 %3); }"
        ).arg(t.accent, t.surface, t.background));
    }

    QFrame *bar = findChild<QFrame *>("loginAccentBar");
    if (bar)
    {
        bar->setStyleSheet(QString(
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "  stop:0 %1, stop:0.5 %2, stop:1 %1);"
            "border-radius:3px;"
        ).arg(t.accent, t.surface));
    }

    for (QLabel *lbl : findChildren<QLabel *>("decoLabel"))
    {
        QString base = lbl->property("baseStyle").toString();
        lbl->setStyleSheet(base + QString(" color:%1;").arg(t.accent));
    }

    for (QLabel *lbl : findChildren<QLabel *>("cardDecoLabel"))
    {
        QString base = lbl->property("baseStyle").toString();
        lbl->setStyleSheet(base + QString(" color:%1;").arg(t.text));
    }

    for (QLabel *lbl : findChildren<QLabel *>("stickerLabel"))
    {
        int sz = lbl->property("stickerSize").toInt();
        lbl->setStyleSheet(QString(
            "font-size:%1px; color:%2; background:transparent;")
            .arg(sz).arg(t.accent));
    }
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::onThemeClicked()
{
    currentThemeIdx = (currentThemeIdx + 1) % themes.size();
    ThemeManager::applyTheme(m_app, themes[currentThemeIdx]);
    updateThemeButton(themes[currentThemeIdx]);
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::onLoginClicked()
{
    QString inputUsername = userEdit->text().trimmed();
    QString inputPassword = passEdit->text();

    AcadenceManager manager;
    try
    {
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
                { name = row[3]; break; }
            }
        }
        else if (role == Constants::Role::Student)
        {
            std::unique_ptr<Student> s(manager.getStudent(userId));
            if (s) name = s->getName();
        }
        else if (role == Constants::Role::Teacher)
        {
            std::unique_ptr<Teacher> t(manager.getTeacher(userId));
            if (t) name = t->getName();
        }
        accept();
    }
    else
    {
        QMessageBox::warning(this, "Login Failed", "Invalid credentials.");
        passEdit->clear();
        passEdit->setFocus();
    }
}

// ──────────────────────────────────────────────────────────────────────────
QString LoginDialog::getRole()   const { return role;   }
int     LoginDialog::getUserId() const { return userId; }
QString LoginDialog::getName()   const { return name;   }
