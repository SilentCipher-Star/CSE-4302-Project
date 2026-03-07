#include "../include/ui_dashboard.hpp"
#include "../include/utils.hpp"
#include "../include/exceptions.hpp"
#include "../include/notifications.hpp"
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
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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

void UIDashboard::refreshDashboard()
{
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
    for (const auto &n : notices)
    {
        if (!noticeVisibleForCurrentUser(n.getContent()))
            continue;

        QString audienceTag;
        QStringList courseIds;
        QString subject;
        QString body;
        const bool structured = parseStructuredNoticeContent(n.getContent(), audienceTag, courseIds, subject, body);

        if (!structured)
        {
            audienceTag = normalizeAudience(audienceLabelFromContent(n.getContent()));
            body = stripAudienceTag(n.getContent());
            subject = body.left(60).replace('\n', ' ').simplified();
            if (subject.isEmpty())
                subject = "No Subject";
            if (body.size() > 60)
                subject += "...";
        }

        QString authorDisplay = n.getAuthor();
        if (isAdminLikeAuthor(authorDisplay))
            authorDisplay = "Register of the Campus";

        QString previewLine = authorDisplay + "  |  " + subject;
        if (userRole == Constants::Role::Admin)
        {
            const QString audienceLabel = audienceTag.isEmpty() ? "ALL" : audienceTag;
            previewLine += "  [" + audienceLabel + "]";
        }
        previewLine += "\n" + summarizeNotice(body);

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(NoticeHeaderRole, previewLine);
        item->setData(NoticeBodyRole, body);
        item->setData(NoticeExpandedRole, false);
        item->setData(NoticeDateRole, n.getDate());
        item->setData(NoticeAuthorRole, authorDisplay);
        item->setData(NoticeAudienceRole, audienceTag);
        item->setData(NoticeSubjectRole, subject);
        item->setData(NoticeCourseIdsRole, courseIds.join(","));
        item->setData(NoticeRawAuthorRole, n.getAuthor());
        item->setData(NoticeRawContentRole, n.getContent());
        updateNoticeItemDisplay(item);
        ui->noticeListWidget->addItem(item);
    }

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
    editor.resize(620, 460);

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

    mainLayout->addLayout(form);
    mainLayout->addWidget(new QLabel("Message:", &editor));
    mainLayout->addWidget(bodyEdit, 1);
    mainLayout->addWidget(hintLabel);

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

    QStringList courseIds;
    if (userRole == Constants::Role::Teacher && audienceTag == "STUDENTS")
    {
        const QString selected = courseTargetCombo->currentData().toString();
        if (!selected.isEmpty() && selected != "*")
            courseIds << selected;
    }

    const QString author = (userRole == Constants::Role::Admin) ? "Register of the Campus" : userName;
    const QString payload = composeNoticeStorageContent(audienceTag, courseIds, subject, body);

    try
    {
        myManager->addNotice(payload, author);
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
    viewer.resize(700, 500);

    QVBoxLayout *layout = new QVBoxLayout(&viewer);
    const QString subject = item->data(NoticeSubjectRole).toString();
    const QString author = item->data(NoticeAuthorRole).toString();
    const QString date = item->data(NoticeDateRole).toString();
    const QString audience = item->data(NoticeAudienceRole).toString();
    const QString body = item->data(NoticeBodyRole).toString();

    QLabel *subjectLabel = new QLabel("<b>" + subject.toHtmlEscaped() + "</b>", &viewer);
    subjectLabel->setWordWrap(true);
    QLabel *fromLabel = new QLabel("From: " + author, &viewer);
    QLabel *dateLabel = new QLabel("Date: " + date, &viewer);
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
        editor.resize(620, 460);

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

        mainLayout->addLayout(form);
        mainLayout->addWidget(new QLabel("Message:", &editor));
        mainLayout->addWidget(bodyEdit, 1);

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

        QStringList newCourseIds;
        if (userRole == Constants::Role::Teacher && newAudience == "STUDENTS")
        {
            const QString selectedCourse = courseTargetCombo->currentData().toString();
            if (!selectedCourse.isEmpty() && selectedCourse != "*")
                newCourseIds << selectedCourse;
        }

        const QString newContent = composeNoticeStorageContent(newAudience, newCourseIds, newSubject, newBody);
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

        const bool ok = myManager->deleteNotice(oldDate, oldAuthor, oldContent);
        if (!ok)
        {
            Notifications::error(nullptr, "Failed to delete notice. It may have been removed already.");
            return;
        }

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

    const QString header = item->data(NoticeHeaderRole).toString();
    item->setText(header);
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

QString UIDashboard::composeNoticeStorageContent(const QString &audienceTag, const QStringList &courseIds, const QString &subject, const QString &body) const
{
    QJsonObject obj;
    obj.insert("audience", audienceTag.trimmed().toUpper());
    obj.insert("subject", subject.trimmed());
    obj.insert("body", body.trimmed());

    QJsonArray ids;
    for (const QString &courseId : courseIds)
        ids.append(courseId.trimmed());
    obj.insert("courseIds", ids);

    const QString json = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return "@notice:" + json;
}

bool UIDashboard::parseStructuredNoticeContent(const QString &raw, QString &audienceTag, QStringList &courseIds, QString &subject, QString &body) const
{
    audienceTag.clear();
    courseIds.clear();
    subject.clear();
    body.clear();

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
