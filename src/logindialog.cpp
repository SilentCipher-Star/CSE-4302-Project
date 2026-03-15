#include "../include/logindialog.hpp"
#include "../include/theme.hpp"
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
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QtMath>
#include <QSettings>

// ── Sticker definitions: symbol · x-ratio · y-ratio · font-size (px) ──────
struct StickerDef
{
    QString text;
    float xr, yr;
    int size;
};
static const QVector<StickerDef> k_stickers = {
    {"★", 0.04f, 0.07f, 56},
    {"✦", 0.94f, 0.11f, 40},
    {"❀", 0.08f, 0.82f, 48},
    {"✧", 0.91f, 0.80f, 52},
    {"✦", 0.85f, 0.48f, 36},
    {"★", 0.22f, 0.90f, 32},
    {"♥", 0.78f, 0.12f, 42},
    {"★", 0.50f, 0.04f, 48},
    {"✧", 0.50f, 0.94f, 36},
    {"✦", 0.65f, 0.75f, 28},
    {"★", 0.88f, 0.55f, 34},
    {"♥", 0.12f, 0.60f, 30},
};

LoginDialog::LoginDialog(QApplication &app, QWidget *parent)
    : QDialog(parent), m_app(app), userId(-1), currentThemeIdx(0)
{
    setWindowTitle("Acadence");
    setModal(true);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    QSettings settings("Acadence", "Acadence");
    bool isDark = settings.value("theme/darkMode", false).toBool();

    if (isDark)
    {
        themes = ThemeManager::getDarkThemes();
        int savedIdx = settings.value("theme/darkIndex", 0).toInt();
        currentThemeIdx = qBound(0, savedIdx, themes.size() - 1);
    }
    else
    {
        themes = ThemeManager::getAvailableThemes();
        QString savedName = settings.value("theme/name", "").toString().trimmed();
        for (int i = 0; i < themes.size(); ++i)
            if (themes[i].name.trimmed() == savedName)
            {
                currentThemeIdx = i;
                break;
            }
    }

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
    decoTop->setProperty("baseStyle", QString("font-size:%1px; letter-spacing:14px; background:transparent;").arg(AppFonts::Title));
    mainLayout->addWidget(decoTop, 0, Qt::AlignCenter);

    mainLayout->addSpacing(10);

    // App title
    m_welcomeLabel = new QLabel("ACADENCE", this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet(
        QString("font-size:%1px; font-weight:bold; letter-spacing:4px; background:transparent;")
            .arg(AppFonts::Title));
    mainLayout->addWidget(m_welcomeLabel, 0, Qt::AlignCenter);

    // Dreamy subtitle
    m_taglineLabel = new QLabel(
        QString::fromUtf8("\xe2\x9c\xa7  your campus  \xc2\xb7  your schedule  \xc2\xb7  your success  \xe2\x9c\xa7"), this);
    m_taglineLabel->setAlignment(Qt::AlignCenter);
    m_taglineLabel->setObjectName("decoLabel");
    m_taglineLabel->setProperty("baseStyle", QString("font-size:%1px; font-weight:400; letter-spacing:1.2px; background:transparent;").arg(AppFonts::Small));
    mainLayout->addWidget(m_taglineLabel, 0, Qt::AlignCenter);

    mainLayout->addSpacing(24);

    // Login card
    QFrame *centerFrame = new QFrame(this);
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
    cardTitle->setProperty("baseStyle", QString("font-size:%1px; font-weight:700; letter-spacing:2px; background:transparent;").arg(AppFonts::Large));
    frameLayout->addWidget(cardTitle);

    frameLayout->addSpacing(8);

    // ── Role selector ──────────────────────────────────────────────────────
    QFrame *roleSelector = new QFrame(centerFrame);
    roleSelector->setObjectName("roleSelector");
    roleSelector->setFixedHeight(44);
    QHBoxLayout *roleLayout = new QHBoxLayout(roleSelector);
    roleLayout->setContentsMargins(0, 0, 0, 0);
    roleLayout->setSpacing(8);

    struct RoleDef
    {
        const char *label;
        const char *role;
    };
    const RoleDef roleDefs[3] = {
        {"\xf0\x9f\x8e\x93  Student", "Student"},
        {"\xf0\x9f\x93\x96  Teacher", "Teacher"},
        {"\xf0\x9f\x94\x91  Admin", "Admin"}};

    for (int i = 0; i < 3; ++i)
    {
        m_roleButtons[i] = new QPushButton(QString::fromUtf8(roleDefs[i].label), roleSelector);
        m_roleButtons[i]->setProperty("roleValue", QString(roleDefs[i].role));
        m_roleButtons[i]->setCursor(Qt::PointingHandCursor);
        m_roleButtons[i]->setFixedHeight(38);
        m_roleButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(m_roleButtons[i], &QPushButton::clicked, this, [this, i]()
                {
            m_selectedRole = m_roleButtons[i]->property("roleValue").toString();
            updateRoleButtons(); });
        roleLayout->addWidget(m_roleButtons[i]);
    }
    frameLayout->addWidget(roleSelector);
    frameLayout->addSpacing(10);

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

    passEdit = new QLineEdit(centerFrame);
    passEdit->setPlaceholderText("  Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(userEdit);
    frameLayout->addWidget(passEdit);
    frameLayout->addSpacing(6);

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, centerFrame);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Sign In");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Exit");
    buttonBox->setCenterButtons(true);
    frameLayout->addWidget(buttonBox);

    mainLayout->addWidget(centerFrame, 0, Qt::AlignCenter);
    mainLayout->addStretch(2);

    // Theme toggle button
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    themeBtn = new QPushButton(this);
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
    connect(themeBtn, &QPushButton::clicked, this, &LoginDialog::onThemeClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::onLoginClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(userEdit, &QLineEdit::returnPressed, this, [this]()
            { passEdit->setFocus(); passEdit->selectAll(); });
    connect(passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    userEdit->installEventFilter(this);
    passEdit->installEventFilter(this);
    userEdit->setFocus();

    updateThemeButton(themes[currentThemeIdx]);
    updateRoleButtons();
}

// ──────────────────────────────────────────────────────────────────────────
bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        const int key = ke->key();

        if (obj == userEdit && (key == Qt::Key_Return || key == Qt::Key_Enter ||
                                key == Qt::Key_Down))
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

        if (obj == passEdit && key == Qt::Key_Up)
        {
            userEdit->setFocus();
            userEdit->selectAll();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QTimer::singleShot(100, this, [this]()
                       {
        int w = width();
        int h = height();

        // ── Position stickers and store base coords ──
        m_stickerBase.clear();
        for (QLabel *lbl : findChildren<QLabel *>("stickerLabel"))
        {
            int   sz = lbl->property("stickerSize").toInt();
            float xr = lbl->property("xRatio").toFloat();
            float yr = lbl->property("yRatio").toFloat();
            lbl->setStyleSheet(QString(
                "font-size:%1px; color:%2; background:transparent; "
                "font-family:'%3','Segoe UI Emoji','Segoe UI Symbol',sans-serif;")
                .arg(sz).arg(themes[currentThemeIdx].accent).arg(AppFonts::Family));
            QPoint base(int(w * xr) - (sz + 24) / 2,
                        int(h * yr) - (sz + 24) / 2);
            lbl->move(base);
            lbl->show();
            lbl->raise();
            m_stickerBase.append(base);
        }

        // ── Start floating animation ──
        if (!m_floatTimer)
        {
            m_floatTimer = new QTimer(this);
            connect(m_floatTimer, &QTimer::timeout, this, &LoginDialog::onFloatTick);
        }
        m_floatFrame = 0;
        m_floatTimer->start(33); // ~30 fps

        // ── Entrance: title fade in ──
        auto *titleEff = new QGraphicsOpacityEffect(m_welcomeLabel);
        m_welcomeLabel->setGraphicsEffect(titleEff);
        titleEff->setOpacity(0.0);
        auto *titleAnim = new QPropertyAnimation(titleEff, "opacity", this);
        titleAnim->setDuration(900);
        titleAnim->setStartValue(0.0);
        titleAnim->setEndValue(1.0);
        titleAnim->setEasingCurve(QEasingCurve::OutCubic);
        titleAnim->start(QAbstractAnimation::DeleteWhenStopped);

        // ── Entrance: tagline fade in (delayed) ──
        auto *tagEff = new QGraphicsOpacityEffect(m_taglineLabel);
        m_taglineLabel->setGraphicsEffect(tagEff);
        tagEff->setOpacity(0.0);
        QTimer::singleShot(300, this, [tagEff, this]()
        {
            auto *tagAnim = new QPropertyAnimation(tagEff, "opacity", this);
            tagAnim->setDuration(800);
            tagAnim->setStartValue(0.0);
            tagAnim->setEndValue(1.0);
            tagAnim->setEasingCurve(QEasingCurve::OutCubic);
            tagAnim->start(QAbstractAnimation::DeleteWhenStopped);
        }); });
}

void LoginDialog::onFloatTick()
{
    ++m_floatFrame;
    const QList<QLabel *> stickers = findChildren<QLabel *>("stickerLabel");
    for (int i = 0; i < stickers.size() && i < m_stickerBase.size(); ++i)
    {
        float phase = i * 0.65f;
        int yOff = (int)(qSin(m_floatFrame * 0.045f + phase) * 7.0f);
        int xOff = (int)(qCos(m_floatFrame * 0.028f + phase * 0.5f) * 4.0f);
        stickers[i]->move(m_stickerBase[i] + QPoint(xOff, yOff));
    }
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::updateRoleButtons()
{
    const AppTheme &t = themes[currentThemeIdx];
    QFrame *roleSelector = findChild<QFrame *>("roleSelector");
    if (roleSelector)
    {
        roleSelector->setStyleSheet("background: transparent; border: none;");
    }

    for (int i = 0; i < 3; ++i)
    {
        if (!m_roleButtons[i])
            continue;
        bool selected = m_roleButtons[i]->property("roleValue").toString() == m_selectedRole;

        QString radius = "border-radius:12px;";

        if (selected)
            m_roleButtons[i]->setStyleSheet(QString(
                                                "QPushButton { background: %1;"
                                                " color:%2; border:none; font-weight:700;"
                                                " font-size:%4px; padding:0 10px; %3 }"
                                                "QPushButton:hover { border: 1px solid %2; }")
                                                .arg(t.accent, t.background, radius)
                                                .arg(AppFonts::Normal));
        else
            m_roleButtons[i]->setStyleSheet(QString(
                                                "QPushButton { background:transparent; color:%1; border:1px solid %1; font-weight:500;"
                                                " font-size:%3px; padding:0 10px; %2 }"
                                                "QPushButton:hover { background:%4; color:%1; }")
                                                .arg(t.accent, radius)
                                                .arg(AppFonts::Normal)
                                                .arg(t.surface));
    }
}

void LoginDialog::updateThemeButton(const AppTheme &t)
{
    themeBtn->setStyleSheet(QString(
                                "QPushButton {"
                                "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);"
                                "  border:2px solid %2; border-radius:23px; font-size:%3px; }"
                                "QPushButton:hover { border-color:white; }")
                                .arg(t.accent, t.background)
                                .arg(AppFonts::Large));
    themeBtn->setToolTip(QString("Theme: %1").arg(t.name));

    QFrame *frame = findChild<QFrame *>("loginFrame");
    if (frame)
    {
        frame->setStyleSheet(QString(
                                 ".QFrame#loginFrame {"
                                 "  border:2px solid %1;"
                                 "  border-radius:28px;"
                                 "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                                 "    stop:0 %2, stop:1 %3); }")
                                 .arg(t.accent, t.surface, t.background));
    }

    QFrame *bar = findChild<QFrame *>("loginAccentBar");
    if (bar)
    {
        bar->setStyleSheet(QString(
                               "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                               "  stop:0 %1, stop:0.5 %2, stop:1 %1);"
                               "border-radius:3px;")
                               .arg(t.accent, t.surface));
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
                               .arg(sz)
                               .arg(t.accent));
    }

    updateRoleButtons();
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::onThemeClicked()
{
    currentThemeIdx = (currentThemeIdx + 1) % themes.size();
    ThemeManager::applyTheme(m_app, themes[currentThemeIdx]);
    updateThemeButton(themes[currentThemeIdx]);

    QSettings settings("Acadence", "Acadence");
    bool isDark = settings.value("theme/darkMode", false).toBool();
    if (isDark)
        settings.setValue("theme/darkIndex", currentThemeIdx);
    else
        settings.setValue("theme/name", themes[currentThemeIdx].name.trimmed());
}

// ──────────────────────────────────────────────────────────────────────────
void LoginDialog::onLoginClicked()
{
    QString inputUsername = userEdit->text().trimmed();
    QString inputPassword = passEdit->text();

    AcadenceManager manager;
    try
    {
        role = manager.login(inputUsername, inputPassword, userId, m_selectedRole);
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
                name = s->getName();
        }
        else if (role == Constants::Role::Teacher)
        {
            std::unique_ptr<Teacher> t(manager.getTeacher(userId));
            if (t)
                name = t->getName();
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
QString LoginDialog::getRole() const { return role; }
int LoginDialog::getUserId() const { return userId; }
QString LoginDialog::getName() const { return name; }
