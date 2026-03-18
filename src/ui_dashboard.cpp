#include "../include/ui_dashboard.hpp"
#include "../include/student.hpp"
#include "../include/chart_widget.hpp"
#include "../include/utils.hpp"
#include "../include/exceptions.hpp"
#include "../include/notifications.hpp"
#include "../include/commands.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QScrollArea>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <memory>

namespace
{
    constexpr int NoticeHeaderRole = Qt::UserRole + 20;
    constexpr int NoticeBodyRole = Qt::UserRole + 21;
    constexpr int NoticeExpandedRole = Qt::UserRole + 22;
    constexpr int NoticeDateRole = Qt::UserRole + 23;
    constexpr int NoticeAuthorRole = Qt::UserRole + 24;
    constexpr int NoticeAudienceRole = Qt::UserRole + 25;
    constexpr int NoticeSubjectRole = Qt::UserRole + 26;
    constexpr int NoticeCourseIdsRole = Qt::UserRole + 27;
    constexpr int NoticeRawAuthorRole = Qt::UserRole + 28;
    constexpr int NoticeRawContentRole = Qt::UserRole + 29;
    // Decorator-pattern roles
    constexpr int NoticeIsUrgentRole = Qt::UserRole + 30;
    constexpr int NoticeIsPinnedRole = Qt::UserRole + 31;
    constexpr int NoticeExpiresOnRole = Qt::UserRole + 32;
    constexpr int NoticeBadgesRole = Qt::UserRole + 33;
    constexpr int NoticeHighlightColorRole = Qt::UserRole + 34;

    QString normalizeAudience(const QString &raw)
    {
        const QString key = raw.trimmed().toUpper();
        if (key == "ALL" || key == "STUDENTS" || key == "TEACHERS" || key == "ADMINS")
            return key;
        return "";
    }

    bool isAdminLikeAuthor(const QString &author)
    {
        const QString key = author.trimmed().toLower();
        return key == "admin" || key == "system admin" || key == "system administrator" || key == "register of the campus";
    }
}

UIDashboard::UIDashboard(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QString name, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userRole(role), userId(uid), userName(name)
{
    ui->noticeListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->noticeListWidget, &QListWidget::customContextMenuRequested,
            this, &UIDashboard::onNoticeListContextMenuRequested);
}

QFrame *UIDashboard::buildStatsCard(const QString &value, const QString &label, const QString &bgColor) const
{
    QFrame *card = new QFrame();
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    card->setObjectName("statsCard");
    card->setStyleSheet(QString("QFrame { background: %1; border-radius: 16px; }").arg(bgColor));

    QVBoxLayout *l = new QVBoxLayout(card);
    l->setContentsMargins(14, 8, 14, 8);
    l->setSpacing(2);

    QLabel *valLbl = new QLabel(value, card);
    valLbl->setAlignment(Qt::AlignCenter);
    valLbl->setStyleSheet("font-size:24px; font-weight:bold; color:white; background:transparent;");

    QLabel *descLbl = new QLabel(label, card);
    descLbl->setAlignment(Qt::AlignCenter);
    descLbl->setStyleSheet("font-size:12px; color:rgba(255,255,255,0.85); background:transparent;");

    l->addWidget(valLbl);
    l->addWidget(descLbl);
    return card;
}

void UIDashboard::refreshStatsCards()
{
    QVBoxLayout *dashLayout = qobject_cast<QVBoxLayout *>(ui->tab_dashboard->layout());
    if (!dashLayout)
        return;

    if (m_statsFrame)
    {
        dashLayout->removeWidget(m_statsFrame);
        delete m_statsFrame;
        m_statsFrame = nullptr;
    }

    m_statsFrame = new QFrame();
    m_statsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *outerL = new QVBoxLayout(m_statsFrame);
    outerL->setContentsMargins(0, 4, 0, 4);
    outerL->setSpacing(6);

    QHBoxLayout *cardsL = new QHBoxLayout();
    cardsL->setSpacing(10);

    if (userRole == Constants::Role::Student)
    {
        // GPA
        QString statsStr = myManager->getDashboardStats(userId, userRole);
        QString gpa = "N/A";
        if (statsStr.contains("GPA:"))
            gpa = statsStr.split("GPA:").last().trimmed();

        // Attendance average
        double avgAtt = 0;
        int attCnt = 0;
        try
        {
            for (const auto &rec : myManager->getStudentAttendance(userId))
            {
                if (rec.getTotalClasses() > 0)
                {
                    avgAtt += 100.0 * rec.getAttendedClasses() / rec.getTotalClasses();
                    ++attCnt;
                }
            }
        }
        catch (...)
        {
        }
        const QString attStr = attCnt > 0 ? QString::number(avgAtt / attCnt, 'f', 1) + "%" : "N/A";

        // Pending tasks
        int pending = 0;
        try
        {
            for (const auto &t : myManager->getTasks(userId))
                if (!t.getIsCompleted())
                    ++pending;
        }
        catch (...)
        {
        }

        cardsL->addWidget(buildStatsCard(gpa, "Current GPA", "#7c3aed"));
        cardsL->addWidget(buildStatsCard(attStr, "Avg Attendance", "#0891b2"));
        cardsL->addWidget(buildStatsCard(QString::number(pending), "Pending Tasks", "#d97706"));

        // View Charts button
        QPushButton *chartBtn = new QPushButton("View Attendance Chart");
        connect(chartBtn, &QPushButton::clicked, this, &UIDashboard::onViewChartsClicked);
        outerL->addLayout(cardsL);
        outerL->addWidget(chartBtn, 0, Qt::AlignRight);
    }
    else if (userRole == Constants::Role::Teacher)
    {
        int courseCount = 0;
        try
        {
            QVector<Course *> courses = myManager->getTeacherCourses(userId);
            courseCount = courses.size();
            qDeleteAll(courses);
        }
        catch (...)
        {
        }

        cardsL->addWidget(buildStatsCard(QString::number(courseCount), "Active Courses", "#7c3aed"));
        outerL->addLayout(cardsL);
    }
    else
    {
        cardsL->addWidget(buildStatsCard("Active", "System Status", "#059669"));
        outerL->addLayout(cardsL);
    }

    dashLayout->insertWidget(1, m_statsFrame);
}

// ── Upcoming Events Widget ────────────────────────────────────────────────────
QFrame *UIDashboard::buildUpcomingSection(const QString &title, const QString &icon,
                                           const QVector<QPair<QString,QString>> &rows,
                                           const QString &accentColor) const
{
    QFrame *section = new QFrame();
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    section->setStyleSheet(QString(
        "QFrame { border: 1.5px solid %1; border-radius: 12px; background: palette(base); }"
    ).arg(accentColor));

    QVBoxLayout *vl = new QVBoxLayout(section);
    vl->setContentsMargins(12, 10, 12, 10);
    vl->setSpacing(6);

    // Section header
    QLabel *hdr = new QLabel(icon + "  " + title);
    hdr->setStyleSheet(QString(
        "font-size:13px; font-weight:bold; color:%1; background:transparent; border:none;"
    ).arg(accentColor));
    vl->addWidget(hdr);

    // Divider
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background:%1; border:none; max-height:1px;").arg(accentColor));
    vl->addWidget(line);

    if (rows.isEmpty())
    {
        QLabel *empty = new QLabel("Nothing scheduled");
        empty->setStyleSheet("font-size:11px; color:palette(mid); background:transparent; border:none;");
        vl->addWidget(empty);
    }
    else
    {
        for (const auto &row : rows)
        {
            QLabel *mainLbl = new QLabel(row.first);
            mainLbl->setStyleSheet("font-size:12px; font-weight:600; background:transparent; border:none;");
            mainLbl->setWordWrap(true);
            vl->addWidget(mainLbl);

            if (!row.second.isEmpty())
            {
                QLabel *subLbl = new QLabel(row.second);
                subLbl->setStyleSheet("font-size:11px; color:palette(mid); background:transparent; border:none;");
                subLbl->setWordWrap(true);
                vl->addWidget(subLbl);
            }
        }
    }

    vl->addStretch();
    return section;
}

void UIDashboard::refreshUpcomingEvents()
{
    if (userRole != Constants::Role::Student)
        return;

    QVBoxLayout *dashLayout = qobject_cast<QVBoxLayout *>(ui->tab_dashboard->layout());
    if (!dashLayout) return;

    if (m_upcomingFrame)
    {
        dashLayout->removeWidget(m_upcomingFrame);
        delete m_upcomingFrame;
        m_upcomingFrame = nullptr;
    }

    m_upcomingFrame = new QFrame();
    m_upcomingFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *hl = new QHBoxLayout(m_upcomingFrame);
    hl->setContentsMargins(0, 4, 0, 4);
    hl->setSpacing(10);

    QDate today = QDate::currentDate();
    QString todayDow = today.toString("dddd"); // e.g. "Monday"

    // ── Section 1: Next Exam ──────────────────────────────────────────────
    QVector<QPair<QString,QString>> examRows;
    QString examAccent = "#6d28d9";
    try
    {
        QVector<Assessment> all = myManager->getStudentAssessments(userId);
        int bestDays = INT_MAX;
        Assessment *best = nullptr;
        for (auto &a : all)
        {
            QDate d = QDate::fromString(a.getDate(), "yyyy-MM-dd");
            if (!d.isValid()) continue;
            int days = today.daysTo(d);
            if (days >= 0 && days < bestDays)
            {
                bestDays = days;
                best = &a;
            }
        }
        if (best)
        {
            QString countdown;
            if (bestDays == 0)       { countdown = "TODAY!";           examAccent = "#dc2626"; }
            else if (bestDays == 1)  { countdown = "Tomorrow";         examAccent = "#dc2626"; }
            else if (bestDays <= 3)  { countdown = QString("in %1 days").arg(bestDays); examAccent = "#ea580c"; }
            else if (bestDays <= 7)  { countdown = QString("in %1 days").arg(bestDays); examAccent = "#d97706"; }
            else                     { countdown = QString("in %1 days").arg(bestDays); examAccent = "#059669"; }

            examRows.append({ best->getTitle(), best->getCourseName() + "  •  " + countdown });
        }
    }
    catch (...) {}

    // ── Section 2: Today's Classes ────────────────────────────────────────
    QVector<QPair<QString,QString>> classRows;
    int semester = -1;
    try
    {
        Student *stu = myManager->getStudent(userId);
        if (stu) { semester = stu->getSemester(); delete stu; }
        QVector<RoutineSession> sessions = myManager->getRoutineForDay(todayDow, semester);
        for (const auto &s : sessions)
            classRows.append({ s.getCourseCode() + "  " + s.getCourseName(),
                               s.getStartTime() + " – " + s.getEndTime() + "  •  " + s.getRoom() });
    }
    catch (...) {}

    // ── Section 3: Pending Tasks ──────────────────────────────────────────
    QVector<QPair<QString,QString>> taskRows;
    int extraTasks = 0;
    try
    {
        QVector<Task> tasks = myManager->getTasks(userId);
        int shown = 0;
        for (const auto &t : tasks)
        {
            if (t.getIsCompleted()) continue;
            if (shown < 3)
            {
                taskRows.append({ t.getDescription(), "" });
                ++shown;
            }
            else
                ++extraTasks;
        }
        if (extraTasks > 0)
            taskRows.append({ QString("+ %1 more...").arg(extraTasks), "" });
    }
    catch (...) {}

    hl->addWidget(buildUpcomingSection("Next Exam",        "\xf0\x9f\x93\x9d", examRows,  examAccent));
    hl->addWidget(buildUpcomingSection("Today's Classes",  "\xf0\x9f\x93\x9a", classRows, "#0891b2"));
    hl->addWidget(buildUpcomingSection("Pending Tasks",    "\xe2\x9c\x85",     taskRows,  "#d97706"));

    dashLayout->insertWidget(2, m_upcomingFrame);
}

void UIDashboard::onViewChartsClicked()
{
    QVector<AttendanceRecord> records;
    try
    {
        records = myManager->getStudentAttendance(userId);
    }
    catch (...)
    {
    }

    QDialog dlg;
    dlg.setWindowTitle("Attendance Chart");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QLabel *title = new QLabel("Attendance % per Course", &dlg);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:15px; font-weight:bold;");
    layout->addWidget(title);

    BarChartWidget *chart = new BarChartWidget(&dlg);
    QVector<QPair<QString, double>> data;
    for (const auto &rec : records)
    {
        if (rec.getTotalClasses() > 0)
        {
            double pct = 100.0 * rec.getAttendedClasses() / rec.getTotalClasses();
            data.append({rec.getCourseName(), pct});
        }
    }
    chart->setData(data, 100.0);

    if (data.isEmpty())
        title->setText("No attendance data available.");

    layout->addWidget(chart, 1);

    QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    layout->addWidget(btns);

    dlg.exec();
}

void UIDashboard::refreshDashboard()
{
    refreshStatsCards();
    refreshUpcomingEvents();
    ui->noticeListWidget->clear();
    QVector<Notice> notices;
    try
    {
        notices = myManager->getNotices();
    }
    catch (const Acadence::Exception &e)
    {
        // Ignore error if notices fail to load
    }
    // Parsed notice info for sorting before inserting into the list widget
    struct ParsedEntry
    {
        Notice raw;
        QString audienceTag, subject, body, authorDisplay, expiresOn, badges;
        QStringList courseIds;
        QColor highlight;
        bool isUrgent = false, isPinned = false, priority = false;
    };

    QVector<ParsedEntry> priorityEntries, regularEntries;

    // Optimization: Cache student course IDs once to avoid reading CSV for every notice
    QSet<int> cachedStudentCourseIds;
    if (userRole == Constants::Role::Student)
    {
        cachedStudentCourseIds = currentStudentCourseIds();
    }

    auto isNoticeVisible = [&](const QString &content) -> bool
    {
        QString audienceTag;
        QStringList courseIds;
        QString subject;
        QString body;
        if (parseStructuredNoticeContent(content, audienceTag, courseIds, subject, body))
        {
            if (userRole == Constants::Role::Admin)
                return true;

            const QString normalized = normalizeAudience(audienceTag);
            if (normalized == "ALL")
                return true;
            if (userRole == Constants::Role::Teacher)
                return normalized == "TEACHERS";

            if (userRole == Constants::Role::Student)
            {
                if (normalized != "STUDENTS")
                    return false;
                if (courseIds.isEmpty())
                    return true;

                for (const QString &cidText : courseIds)
                {
                    if (cachedStudentCourseIds.contains(cidText.toInt()))
                        return true;
                }
                return false;
            }
            return false;
        }

        // Fallback to legacy check if not structured
        return noticeVisibleForCurrentUser(content);
    };

    for (const auto &n : notices)
    {
        if (!isNoticeVisible(n.getContent()))
            continue;

        ParsedEntry e;
        e.raw = n;

        bool isUrgent = false, isPinned = false;
        QString expiresOn;
        const bool structured = parseStructuredNoticeContent(
            n.getContent(), e.audienceTag, e.courseIds, e.subject, e.body,
            &isUrgent, &isPinned, &expiresOn);

        if (!structured)
        {
            e.audienceTag = normalizeAudience(audienceLabelFromContent(n.getContent()));
            e.body = stripAudienceTag(n.getContent());
            e.subject = e.body.left(60).replace('\n', ' ').simplified();
            if (e.subject.isEmpty())
                e.subject = "No Subject";
            if (e.body.size() > 60)
                e.subject += "...";
        }

        e.isUrgent = isUrgent;
        e.isPinned = isPinned;
        e.expiresOn = expiresOn;

        e.authorDisplay = n.getAuthor();
        if (isAdminLikeAuthor(e.authorDisplay))
            e.authorDisplay = "Register of the Campus";

        // ---- Decorator Pattern: build the decorated notice ----
        auto decorated = buildDecoratedNotice(
            e.subject, e.body, e.authorDisplay, n.getDate(),
            e.isUrgent, e.isPinned, e.expiresOn);

        e.badges = decorated->getBadges();
        e.highlight = decorated->getHighlightColor();
        e.priority = decorated->isPriority();
        // -------------------------------------------------------

        if (e.priority)
            priorityEntries.append(e);
        else
            regularEntries.append(e);
    }

    // Helper lambda: build a QListWidgetItem from stored ParsedEntry + preview
    auto makeItem = [&](const ParsedEntry &e) -> QListWidgetItem *
    {
        QString previewLine;
        if (!e.badges.isEmpty())
            previewLine = e.badges + "  " + e.authorDisplay + "  |  " + e.subject;
        else
            previewLine = e.authorDisplay + "  |  " + e.subject;

        if (userRole == Constants::Role::Admin)
        {
            const QString audienceLabel = e.audienceTag.isEmpty() ? "ALL" : e.audienceTag;
            previewLine += "  [" + audienceLabel + "]";
        }
        previewLine += "\n" + summarizeNotice(e.body);

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(NoticeHeaderRole, previewLine);
        item->setData(NoticeBodyRole, e.body);
        item->setData(NoticeExpandedRole, false);
        item->setData(NoticeDateRole, e.raw.getDate());
        item->setData(NoticeAuthorRole, e.authorDisplay);
        item->setData(NoticeAudienceRole, e.audienceTag);
        item->setData(NoticeSubjectRole, e.subject);
        item->setData(NoticeCourseIdsRole, e.courseIds.join(","));
        item->setData(NoticeRawAuthorRole, e.raw.getAuthor());
        item->setData(NoticeRawContentRole, e.raw.getContent());
        item->setData(NoticeIsUrgentRole, e.isUrgent);
        item->setData(NoticeIsPinnedRole, e.isPinned);
        item->setData(NoticeExpiresOnRole, e.expiresOn);
        item->setData(NoticeBadgesRole, e.badges);
        item->setData(NoticeHighlightColorRole, e.highlight);
        updateNoticeItemDisplay(item);
        return item;
    };

    // Add pinned/urgent notices first, then ordinary notices (each group
    // already sorted newest-first by ManagerNotices::getNotices)
    for (const auto &e : priorityEntries)
        ui->noticeListWidget->addItem(makeItem(e));
    for (const auto &e : regularEntries)
        ui->noticeListWidget->addItem(makeItem(e));

    if (userRole == Constants::Role::Student)
        ui->label_notices->setText("Notices For Students:");
    else if (userRole == Constants::Role::Teacher)
        ui->label_notices->setText("Notices For Teachers:");
    else
        ui->label_notices->setText("All Campus Notices:");

    QString stats = myManager->getDashboardStats(userId, userRole);
    ui->label_welcome->setToolTip(stats);

    if (userRole == Constants::Role::Student)
    {
        std::unique_ptr<Student> s(myManager->getStudent(userId));
        if (s)
        {
            ui->val_p_name->setText(s->getName());
            ui->lbl_p_id->setVisible(true);
            ui->val_p_id->setVisible(true);
            ui->val_p_id->setText(QString::number(s->getId()));
            ui->val_p_dept->setText(s->getDepartment());
            ui->val_p_sem->setText(QString::number(s->getSemester()));
            ui->val_p_email->setText(s->getEmail());
        }
    }
    else if (userRole == Constants::Role::Teacher)
    {
        std::unique_ptr<Teacher> t(myManager->getTeacher(userId));
        if (t)
        {
            ui->val_p_name->setText(t->getName());
            ui->lbl_p_id->setVisible(false);
            ui->val_p_id->setVisible(false);
            ui->val_p_dept->setText(t->getDepartment());
            ui->lbl_p_sem->setText("Designation:");
            ui->val_p_sem->setText(t->getDesignation());
            ui->val_p_email->setText(t->getEmail());
        }
    }
    else if (userRole == Constants::Role::Admin)
    {
        auto profile = myManager->getAdminProfile(userId);
        ui->groupBox_profile->setTitle("Administrator");
        ui->val_p_name->setText(profile.first);
        ui->lbl_p_id->setVisible(false);
        ui->val_p_id->setVisible(false);
        ui->val_p_dept->setText("IT / Admin");
        ui->lbl_p_sem->setText("Role:");
        ui->val_p_sem->setText("Super User");
        ui->val_p_email->setText(profile.second);
    }
}

void UIDashboard::onAddNoticeClicked()
{
    if (userRole != Constants::Role::Teacher && userRole != Constants::Role::Admin)
        return;

    QDialog editor;
    editor.setWindowTitle("Compose Notice");
    editor.setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(&editor);
    QFormLayout *form = new QFormLayout();

    QComboBox *audienceCombo = new QComboBox(&editor);
    if (userRole == Constants::Role::Teacher)
        audienceCombo->addItems({"Students", "All"});
    else
        audienceCombo->addItems({"All", "Students", "Teachers", "Admins"});
    form->addRow("Audience:", audienceCombo);

    QComboBox *courseTargetCombo = new QComboBox(&editor);
    courseTargetCombo->addItem("All My Students", "*");
    if (userRole == Constants::Role::Teacher)
    {
        QVector<Course *> courses = myManager->getTeacherCourses(userId);
        for (Course *c : courses)
            courseTargetCombo->addItem(c->getCode() + " - " + c->getName(), QString::number(c->getId()));
        qDeleteAll(courses);
        form->addRow("Student Target:", courseTargetCombo);
    }

    QLineEdit *subjectEdit = new QLineEdit(&editor);
    subjectEdit->setPlaceholderText("Short subject, e.g. Midterm date update");
    form->addRow("Subject:", subjectEdit);

    QTextEdit *bodyEdit = new QTextEdit(&editor);
    bodyEdit->setPlaceholderText("Write the full notice here...");
    bodyEdit->setAcceptRichText(false);

    QLabel *hintLabel = new QLabel(&editor);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet("color: #666;");
    hintLabel->setText("Tip: Students will see sender, subject, and a preview. Clicking opens full notice.");

    // ---- Decorator Pattern: notice options ----
    QGroupBox *optionsBox = new QGroupBox("Notice Options", &editor);
    QVBoxLayout *optLayout = new QVBoxLayout(optionsBox);

    QCheckBox *urgentCheck = new QCheckBox("Mark as Urgent  (red highlight, sorted to top)", optionsBox);
    QCheckBox *pinnedCheck = new QCheckBox("Pin this Notice  (yellow highlight, sorted to top)", optionsBox);
    optLayout->addWidget(urgentCheck);
    optLayout->addWidget(pinnedCheck);

    QHBoxLayout *expiryRow = new QHBoxLayout();
    QCheckBox *expiryCheck = new QCheckBox("Set Expiry Date:", optionsBox);
    QDateEdit *expiresEdit = new QDateEdit(QDate::currentDate().addDays(7), optionsBox);
    expiresEdit->setCalendarPopup(true);
    expiresEdit->setDisplayFormat("yyyy-MM-dd");
    expiresEdit->setMinimumDate(QDate::currentDate());
    expiresEdit->setEnabled(false);
    expiryRow->addWidget(expiryCheck);
    expiryRow->addWidget(expiresEdit);
    expiryRow->addStretch();
    optLayout->addLayout(expiryRow);
    connect(expiryCheck, &QCheckBox::toggled, expiresEdit, &QDateEdit::setEnabled);
    // -------------------------------------------

    mainLayout->addLayout(form);
    mainLayout->addWidget(new QLabel("Message:", &editor));
    mainLayout->addWidget(bodyEdit, 1);
    mainLayout->addWidget(hintLabel);
    mainLayout->addWidget(optionsBox);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &editor);
    buttons->button(QDialogButtonBox::Ok)->setText("Send Notice");
    mainLayout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &editor, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &editor, &QDialog::reject);

    if (editor.exec() != QDialog::Accepted)
        return;

    const QString subject = subjectEdit->text().trimmed();
    const QString body = bodyEdit->toPlainText().trimmed();
    const QString audienceTag = audienceCombo->currentText().trimmed().toUpper();
    if (subject.isEmpty())
    {
        Notifications::warning(nullptr, "Subject is required.");
        return;
    }
    if (body.isEmpty())
    {
        Notifications::warning(nullptr, "Notice message is required.");
        return;
    }

    const QString expiresOn = expiryCheck->isChecked()
                                  ? expiresEdit->date().toString("yyyy-MM-dd")
                                  : QString{};

    QStringList courseIds;
    if (userRole == Constants::Role::Teacher && audienceTag == "STUDENTS")
    {
        const QString selected = courseTargetCombo->currentData().toString();
        if (!selected.isEmpty() && selected != "*")
            courseIds << selected;
    }

    const bool isUrgent = urgentCheck->isChecked();
    const bool isPinned = pinnedCheck->isChecked();
    const QString author = (userRole == Constants::Role::Admin) ? "Register of the Campus" : userName;
    const QString payload = composeNoticeStorageContent(audienceTag, courseIds, subject, body, isUrgent, isPinned, expiresOn);

    try
    {
        auto cmd = std::make_shared<AddNoticeCommand>(payload, author);
        myManager->executeCommand(cmd);
        Notifications::success(nullptr, "Notice posted successfully.");
        refreshDashboard();
    }
    catch (const Acadence::Exception &e)
    {
        Notifications::error(nullptr, QString::fromUtf8(e.what()));
    }
}

void UIDashboard::onNoticeItemClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    QDialog viewer;
    viewer.setWindowTitle("Notice");
    viewer.setModal(true);
    viewer.resize(900, 700);
    viewer.setMinimumSize(700, 500);

    QVBoxLayout *layout = new QVBoxLayout(&viewer);
    const QString subject = item->data(NoticeSubjectRole).toString();
    const QString author = item->data(NoticeAuthorRole).toString();
    const QString date = item->data(NoticeDateRole).toString();
    const QString audience = item->data(NoticeAudienceRole).toString();
    const QString body = item->data(NoticeBodyRole).toString();

    // Decorator Pattern: show badges if the notice has any decorator applied
    const QString badges = item->data(NoticeBadgesRole).toString();
    if (!badges.isEmpty())
    {
        QLabel *badgesLabel = new QLabel(badges, &viewer);
        badgesLabel->setStyleSheet("font-weight: bold; color: #8B0000; font-size: 11px;");
        layout->addWidget(badgesLabel);
    }

    QLabel *subjectLabel = new QLabel("<b>" + subject.toHtmlEscaped() + "</b>", &viewer);
    subjectLabel->setWordWrap(true);
    QLabel *fromLabel = new QLabel("From: " + author, &viewer);
    QLabel *dateLabel = new QLabel("Date: " + date, &viewer);

    const QString expiresOn = item->data(NoticeExpiresOnRole).toString();
    if (!expiresOn.isEmpty())
        dateLabel->setText(dateLabel->text() + "   |   Expires: " + expiresOn);

    QLabel *audienceLabel = new QLabel("Audience: " + (audience.isEmpty() ? "ALL" : audience), &viewer);

    const QString courseIds = item->data(NoticeCourseIdsRole).toString();
    if (!courseIds.isEmpty())
    {
        QStringList labels;
        for (const QString &idText : courseIds.split(',', Qt::SkipEmptyParts))
        {
            const int cid = idText.toInt();
            if (cid > 0)
                labels << courseNameById(cid);
        }
        if (!labels.isEmpty())
            audienceLabel->setText(audienceLabel->text() + " | Courses: " + labels.join(", "));
    }

    QTextEdit *bodyView = new QTextEdit(&viewer);
    bodyView->setReadOnly(true);
    bodyView->setPlainText(body);

    layout->addWidget(subjectLabel);
    layout->addWidget(fromLabel);
    layout->addWidget(dateLabel);
    layout->addWidget(audienceLabel);
    layout->addWidget(bodyView, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &viewer);
    connect(buttons, &QDialogButtonBox::rejected, &viewer, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &viewer, &QDialog::accept);
    layout->addWidget(buttons);

    viewer.exec();
}

void UIDashboard::onNoticeItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
}

void UIDashboard::onNoticeListContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->noticeListWidget->itemAt(pos);
    if (!item)
        return;

    QMenu menu;
    QAction *openAction = menu.addAction("Open Notice");
    QAction *editAction = nullptr;
    QAction *deleteAction = nullptr;

    const QString rawAuthor = item->data(NoticeRawAuthorRole).toString();
    const bool canManage = (userRole == Constants::Role::Admin) ||
                           (rawAuthor.compare(userName, Qt::CaseInsensitive) == 0);

    if (canManage)
    {
        menu.addSeparator();
        editAction = menu.addAction("Edit Notice");
        deleteAction = menu.addAction("Delete Notice");
    }

    QAction *selected = menu.exec(ui->noticeListWidget->viewport()->mapToGlobal(pos));
    if (selected == openAction && item)
    {
        onNoticeItemClicked(item);
        return;
    }

    if (selected == editAction && item)
    {
        const QString oldDate = item->data(NoticeDateRole).toString();
        const QString oldAuthor = item->data(NoticeRawAuthorRole).toString();
        const QString oldContent = item->data(NoticeRawContentRole).toString();

        QString audienceTag;
        QStringList courseIds;
        QString subject;
        QString body;
        const bool structured = parseStructuredNoticeContent(oldContent, audienceTag, courseIds, subject, body);
        if (!structured)
        {
            audienceTag = normalizeAudience(audienceLabelFromContent(oldContent));
            if (audienceTag.isEmpty())
                audienceTag = "ALL";
            body = stripAudienceTag(oldContent);
            subject = item->data(NoticeSubjectRole).toString().trimmed();
            if (subject.isEmpty())
                subject = "No Subject";
        }

        QDialog editor;
        editor.setWindowTitle("Edit Notice");
        editor.setModal(true);

        QVBoxLayout *mainLayout = new QVBoxLayout(&editor);
        QFormLayout *form = new QFormLayout();

        QComboBox *audienceCombo = new QComboBox(&editor);
        if (userRole == Constants::Role::Teacher)
            audienceCombo->addItems({"Students", "All"});
        else
            audienceCombo->addItems({"All", "Students", "Teachers", "Admins"});
        const int audienceIdx = audienceCombo->findText(audienceTag, Qt::MatchFixedString);
        audienceCombo->setCurrentIndex(audienceIdx >= 0 ? audienceIdx : 0);
        form->addRow("Audience:", audienceCombo);

        QComboBox *courseTargetCombo = new QComboBox(&editor);
        courseTargetCombo->addItem("All My Students", "*");
        if (userRole == Constants::Role::Teacher)
        {
            QVector<Course *> courses = myManager->getTeacherCourses(userId);
            for (Course *c : courses)
                courseTargetCombo->addItem(c->getCode() + " - " + c->getName(), QString::number(c->getId()));
            qDeleteAll(courses);
            if (courseIds.size() == 1)
            {
                const int idx = courseTargetCombo->findData(courseIds.first());
                if (idx >= 0)
                    courseTargetCombo->setCurrentIndex(idx);
            }
            form->addRow("Student Target:", courseTargetCombo);
        }

        QLineEdit *subjectEdit = new QLineEdit(&editor);
        subjectEdit->setPlaceholderText("Short subject, e.g. Midterm date update");
        subjectEdit->setText(subject);
        form->addRow("Subject:", subjectEdit);

        QTextEdit *bodyEdit = new QTextEdit(&editor);
        bodyEdit->setAcceptRichText(false);
        bodyEdit->setPlainText(body);

        // ---- Decorator Pattern: pre-populate existing decorator options ----
        QGroupBox *editOptionsBox = new QGroupBox("Notice Options", &editor);
        QVBoxLayout *editOptLayout = new QVBoxLayout(editOptionsBox);

        QCheckBox *editUrgentCheck = new QCheckBox("Mark as Urgent", editOptionsBox);
        QCheckBox *editPinnedCheck = new QCheckBox("Pin this Notice", editOptionsBox);
        editUrgentCheck->setChecked(item->data(NoticeIsUrgentRole).toBool());
        editPinnedCheck->setChecked(item->data(NoticeIsPinnedRole).toBool());
        editOptLayout->addWidget(editUrgentCheck);
        editOptLayout->addWidget(editPinnedCheck);

        const QString existingExpiry = item->data(NoticeExpiresOnRole).toString();
        const QDate existingExpiryDate = QDate::fromString(existingExpiry, "yyyy-MM-dd");

        QHBoxLayout *editExpiryRow = new QHBoxLayout();
        QCheckBox *editExpiryCheck = new QCheckBox("Set Expiry Date:", editOptionsBox);
        QDateEdit *editExpiresEdit = new QDateEdit(editOptionsBox);
        editExpiresEdit->setCalendarPopup(true);
        editExpiresEdit->setDisplayFormat("yyyy-MM-dd");
        editExpiresEdit->setMinimumDate(QDate::currentDate());
        if (existingExpiryDate.isValid())
        {
            editExpiryCheck->setChecked(true);
            editExpiresEdit->setDate(existingExpiryDate);
        }
        else
        {
            editExpiresEdit->setDate(QDate::currentDate().addDays(7));
            editExpiresEdit->setEnabled(false);
        }
        editExpiryRow->addWidget(editExpiryCheck);
        editExpiryRow->addWidget(editExpiresEdit);
        editExpiryRow->addStretch();
        editOptLayout->addLayout(editExpiryRow);
        connect(editExpiryCheck, &QCheckBox::toggled, editExpiresEdit, &QDateEdit::setEnabled);
        // -------------------------------------------------------------------

        mainLayout->addLayout(form);
        mainLayout->addWidget(new QLabel("Message:", &editor));
        mainLayout->addWidget(bodyEdit, 1);
        mainLayout->addWidget(editOptionsBox);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &editor);
        buttons->button(QDialogButtonBox::Ok)->setText("Save Changes");
        mainLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &editor, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &editor, &QDialog::reject);

        if (editor.exec() != QDialog::Accepted)
            return;

        const QString newSubject = subjectEdit->text().trimmed();
        const QString newBody = bodyEdit->toPlainText().trimmed();
        const QString newAudience = audienceCombo->currentText().trimmed().toUpper();
        if (newSubject.isEmpty())
        {
            Notifications::warning(nullptr, "Subject is required.");
            return;
        }
        if (newBody.isEmpty())
        {
            Notifications::warning(nullptr, "Notice message is required.");
            return;
        }

        const QString newExpiresOn = editExpiryCheck->isChecked()
                                         ? editExpiresEdit->date().toString("yyyy-MM-dd")
                                         : QString{};

        QStringList newCourseIds;
        if (userRole == Constants::Role::Teacher && newAudience == "STUDENTS")
        {
            const QString selectedCourse = courseTargetCombo->currentData().toString();
            if (!selectedCourse.isEmpty() && selectedCourse != "*")
                newCourseIds << selectedCourse;
        }

        const bool newIsUrgent = editUrgentCheck->isChecked();
        const bool newIsPinned = editPinnedCheck->isChecked();
        const QString newContent = composeNoticeStorageContent(newAudience, newCourseIds, newSubject, newBody,
                                                               newIsUrgent, newIsPinned, newExpiresOn);
        const bool ok = myManager->updateNotice(oldDate, oldAuthor, oldContent, newContent);
        if (!ok)
        {
            Notifications::error(nullptr, "Failed to edit notice. It may have been changed already.");
            return;
        }

        Notifications::success(nullptr, "Notice updated successfully.");
        refreshDashboard();
        return;
    }

    if (selected == deleteAction && item)
    {
        const QString oldDate = item->data(NoticeDateRole).toString();
        const QString oldAuthor = item->data(NoticeRawAuthorRole).toString();
        const QString oldContent = item->data(NoticeRawContentRole).toString();
        const QString subject = item->data(NoticeSubjectRole).toString();

        if (!Notifications::confirmDelete(nullptr, "notice \"" + subject + "\""))
            return;

        auto cmd = std::make_shared<DeleteNoticeCommand>(oldDate, oldAuthor, oldContent);
        myManager->executeCommand(cmd);

        Notifications::success(nullptr, "Notice deleted successfully.");
        refreshDashboard();
    }
}

void UIDashboard::expandAllNotices()
{
    // Kept for compatibility: Gmail-style view opens full notice in dialog.
}

void UIDashboard::collapseAllNotices()
{
    // Kept for compatibility: Gmail-style view opens full notice in dialog.
}

void UIDashboard::onChangePasswordClicked()
{
    QDialog dlg;
    dlg.setWindowTitle("Change Password");
    dlg.setModal(true);
    dlg.setMinimumWidth(350);
    QFormLayout *layout = new QFormLayout(&dlg);

    QLabel *strengthLabel = new QLabel("Password Strength: ", &dlg);
    layout->addRow(strengthLabel);

    QLineEdit *oldPass = new QLineEdit(&dlg);
    oldPass->setEchoMode(QLineEdit::Password);
    layout->addRow("Old Password:", oldPass);

    QLineEdit *newPass = new QLineEdit(&dlg);
    newPass->setEchoMode(QLineEdit::Password);
    layout->addRow("New Password:", newPass);

    QLineEdit *confirmPass = new QLineEdit(&dlg);
    confirmPass->setEchoMode(QLineEdit::Password);
    layout->addRow("Confirm Password:", confirmPass);

    QLabel *requirementsLabel = new QLabel("Requirements: Min 6 chars, 1 letter, 1 number", &dlg);
    requirementsLabel->setStyleSheet("color: gray; font-style: italic;");
    layout->addRow(requirementsLabel);

    QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addRow(btns);

    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted)
    {
        if (newPass->text().isEmpty())
        {
            Notifications::warning(nullptr, "New password cannot be empty.");
            return;
        }

        if (newPass->text() != confirmPass->text())
        {
            Notifications::error(nullptr, "New passwords do not match.");
            return;
        }

        QString error = Utils::validatePassword(newPass->text());
        if (!error.isEmpty())
        {
            Notifications::warning(nullptr, error);
            return;
        }

        try
        {
            if (myManager->changePassword(userId, userRole, oldPass->text(), newPass->text()))
            {
                Notifications::success(nullptr, "Password changed successfully.");
            }
        }
        catch (const Acadence::Exception &e)
        {
            Notifications::error(nullptr, QString::fromUtf8(e.what()));
        }
    }
}

void UIDashboard::onLogoutClicked()
{
    if (Notifications::confirm(nullptr, "Logout", "Are you sure you want to logout?"))
        QApplication::exit(99);
}

void UIDashboard::onSearchNoticesChanged(const QString &searchText)
{
    const QString lowerSearch = searchText.toLower();
    for (int i = 0; i < ui->noticeListWidget->count(); ++i)
    {
        QListWidgetItem *item = ui->noticeListWidget->item(i);
        if (!item)
            continue;

        const QString header = item->data(NoticeHeaderRole).toString();
        const QString body = item->data(NoticeBodyRole).toString();
        const bool matches = searchText.isEmpty() || header.toLower().contains(lowerSearch) || body.toLower().contains(lowerSearch);
        item->setHidden(!matches);
    }
}

bool UIDashboard::noticeVisibleForCurrentUser(const QString &content) const
{
    QString audienceTag;
    QStringList courseIds;
    QString subject;
    QString body;
    if (parseStructuredNoticeContent(content, audienceTag, courseIds, subject, body))
    {
        if (userRole == Constants::Role::Admin)
            return true;

        const QString normalized = normalizeAudience(audienceTag);
        if (normalized == "ALL")
            return true;

        if (userRole == Constants::Role::Teacher)
            return normalized == "TEACHERS";

        if (userRole == Constants::Role::Student)
        {
            if (normalized != "STUDENTS")
                return false;
            if (courseIds.isEmpty())
                return true;

            const QSet<int> mine = currentStudentCourseIds();
            for (const QString &cidText : courseIds)
            {
                if (mine.contains(cidText.toInt()))
                    return true;
            }
            return false;
        }

        return false;
    }

    const QRegularExpression re("^\\s*\\[([^\\]]+)\\]");
    const QRegularExpressionMatch m = re.match(content);
    if (!m.hasMatch())
        return true;

    const QString audience = normalizeAudience(m.captured(1));
    if (audience.isEmpty() || audience == "ALL")
        return true;

    return audience == audienceTagForRole(userRole);
}

QString UIDashboard::stripAudienceTag(const QString &content) const
{
    QString audienceTag;
    QStringList courseIds;
    QString subject;
    QString body;
    if (parseStructuredNoticeContent(content, audienceTag, courseIds, subject, body))
        return body;

    const QRegularExpression re("^\\s*\\[([^\\]]+)\\]\\s*");
    const QRegularExpressionMatch m = re.match(content);
    if (!m.hasMatch())
        return content;

    const QString audience = normalizeAudience(m.captured(1));
    if (audience.isEmpty())
        return content;

    return content.mid(m.capturedLength()).trimmed();
}

QString UIDashboard::audienceTagForRole(const QString &role) const
{
    if (role == Constants::Role::Student)
        return "STUDENTS";
    if (role == Constants::Role::Teacher)
        return "TEACHERS";
    if (role == Constants::Role::Admin)
        return "ADMINS";
    return "";
}

QString UIDashboard::audienceLabelFromContent(const QString &content) const
{
    QString audienceTag;
    QStringList courseIds;
    QString subject;
    QString body;
    if (parseStructuredNoticeContent(content, audienceTag, courseIds, subject, body))
    {
        const QString normalized = normalizeAudience(audienceTag);
        if (!normalized.isEmpty())
            return normalized.left(1) + normalized.mid(1).toLower();
    }

    const QRegularExpression re("^\\s*\\[([^\\]]+)\\]");
    const QRegularExpressionMatch m = re.match(content);
    if (!m.hasMatch())
        return "All";

    const QString audience = normalizeAudience(m.captured(1));
    if (audience.isEmpty())
        return "All";
    return audience.left(1) + audience.mid(1).toLower();
}

void UIDashboard::updateNoticeItemDisplay(QListWidgetItem *item)
{
    if (!item)
        return;

    item->setText(item->data(NoticeHeaderRole).toString());

    // Decorator pattern: apply the highlight colour computed by the decorator chain
    const QColor highlight = item->data(NoticeHighlightColorRole).value<QColor>();
    if (highlight.isValid())
        item->setBackground(highlight);
    else
        item->setBackground(QBrush()); // reset to default
}

QString UIDashboard::summarizeNotice(const QString &content) const
{
    QString singleLine = content;
    singleLine.replace('\n', ' ');
    singleLine = singleLine.simplified();
    if (singleLine.size() <= 90)
        return singleLine;
    return singleLine.left(90) + "...";
}

QString UIDashboard::composeNoticeStorageContent(const QString &audienceTag, const QStringList &courseIds,
                                                 const QString &subject, const QString &body,
                                                 bool isUrgent, bool isPinned, const QString &expiresOn) const
{
    QJsonObject obj;
    obj.insert("audience", audienceTag.trimmed().toUpper());
    obj.insert("subject", subject.trimmed());
    obj.insert("body", body.trimmed());

    // Decorator flags – only persist when set (keeps CSV compact for plain notices)
    if (isUrgent)
        obj.insert("isUrgent", true);
    if (isPinned)
        obj.insert("isPinned", true);
    if (!expiresOn.trimmed().isEmpty())
        obj.insert("expiresOn", expiresOn.trimmed());

    QJsonArray ids;
    for (const QString &courseId : courseIds)
        ids.append(courseId.trimmed());
    obj.insert("courseIds", ids);

    const QString json = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return "@notice:" + json;
}

bool UIDashboard::parseStructuredNoticeContent(const QString &raw, QString &audienceTag, QStringList &courseIds,
                                               QString &subject, QString &body,
                                               bool *isUrgent, bool *isPinned, QString *expiresOn) const
{
    audienceTag.clear();
    courseIds.clear();
    subject.clear();
    body.clear();
    if (isUrgent)
        *isUrgent = false;
    if (isPinned)
        *isPinned = false;
    if (expiresOn)
        expiresOn->clear();

    if (!raw.startsWith("@notice:"))
        return false;

    const QByteArray jsonRaw = raw.mid(QString("@notice:").size()).toUtf8();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonRaw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    const QJsonObject obj = doc.object();
    audienceTag = obj.value("audience").toString().trimmed().toUpper();
    subject = obj.value("subject").toString().trimmed();
    body = obj.value("body").toString().trimmed();

    // Decorator flags
    if (isUrgent)
        *isUrgent = obj.value("isUrgent").toBool(false);
    if (isPinned)
        *isPinned = obj.value("isPinned").toBool(false);
    if (expiresOn)
        *expiresOn = obj.value("expiresOn").toString().trimmed();

    const QJsonArray ids = obj.value("courseIds").toArray();
    for (const QJsonValue &v : ids)
    {
        const QString idText = v.toString().trimmed();
        if (!idText.isEmpty())
            courseIds.append(idText);
    }

    return true;
}

QSet<int> UIDashboard::currentStudentCourseIds() const
{
    QSet<int> ids;
    if (userRole != Constants::Role::Student)
        return ids;

    const QVector<QStringList> enrollments = CsvHandler::readCsv("enrollments.csv");
    for (const QStringList &row : enrollments)
    {
        if (row.size() >= 2 && row[0].toInt() == userId)
            ids.insert(row[1].toInt());
    }
    return ids;
}

QString UIDashboard::courseNameById(int courseId) const
{
    std::unique_ptr<Course> c(myManager->getCourse(courseId));
    if (!c)
        return QString("Course #%1").arg(courseId);
    return c->getCode() + " - " + c->getName();
}
