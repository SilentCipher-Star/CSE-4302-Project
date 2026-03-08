#pragma once

#include <QWidget>
#include <QListWidgetItem>
#include <QSet>
#include "appmanager.hpp"
#include "notice_decorators.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Dashboard module: handles notices, profile display, and password changes
class UIDashboard : public QObject
{
    Q_OBJECT

public:
    UIDashboard(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QString name, QObject *parent = nullptr);

    void refreshDashboard();

public slots:
    void onAddNoticeClicked();
    void onNoticeItemClicked(QListWidgetItem *item);
    void onNoticeItemDoubleClicked(QListWidgetItem *item);
    void onNoticeListContextMenuRequested(const QPoint &pos);
    void expandAllNotices();
    void collapseAllNotices();
    void onChangePasswordClicked();
    void onLogoutClicked();
    void onSearchNoticesChanged(const QString &searchText);

private:
    void updateNoticeItemDisplay(QListWidgetItem *item);
    QString summarizeNotice(const QString &content) const;

    bool noticeVisibleForCurrentUser(const QString &content) const;
    QString stripAudienceTag(const QString &content) const;
    QString audienceTagForRole(const QString &role) const;
    QString audienceLabelFromContent(const QString &content) const;
    QString composeNoticeStorageContent(const QString &audienceTag, const QStringList &courseIds, const QString &subject, const QString &body,
                                         bool isUrgent = false, bool isPinned = false, const QString &expiresOn = {}) const;
    bool parseStructuredNoticeContent(const QString &raw, QString &audienceTag, QStringList &courseIds, QString &subject, QString &body,
                                      bool *isUrgent = nullptr, bool *isPinned = nullptr, QString *expiresOn = nullptr) const;
    QSet<int> currentStudentCourseIds() const;
    QString courseNameById(int courseId) const;

    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    QString userRole;
    int userId;
    QString userName;
};
