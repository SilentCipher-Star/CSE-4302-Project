#pragma once

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include "appmanager.hpp"

class UILostFound : public QObject
{
    Q_OBJECT

public:
    explicit UILostFound(AcadenceManager *manager, const QString &role,
                         int userId, const QString &userName, QObject *parent = nullptr);

    QWidget *getWidget() const { return m_container; }
    void refresh();

private slots:
    void onPostClicked();
    void onClaimClicked();
    void onDeleteClicked();
    void onFilterChanged();
    void onPostSelected();

private:
    AcadenceManager *m_mgr;
    QString m_role;
    int m_userId;
    QString m_userName;

    QWidget *m_container;
    QTableWidget *m_table;
    QTextBrowser *m_detailView;
    QComboBox *m_filterCombo;
    QLineEdit *m_searchEdit;
    QLineEdit *m_editItemName;
    QTextEdit *m_editDescription;
    QLineEdit *m_editLocation;
    QComboBox *m_typeCombo;
    QPushButton *m_btnPost;
    QPushButton *m_btnClaim;
    QPushButton *m_btnDelete;

    QVector<LostFoundPost> m_posts;
    QVector<LostFoundPost> m_filteredPosts;

    void buildWidget();
    void applyFilter();
};
