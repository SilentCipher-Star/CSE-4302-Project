#include "../include/ui_lostfound.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QScrollArea>
#include <QDateTime>
#include <QFont>
#include <QFrame>

UILostFound::UILostFound(AcadenceManager *manager, const QString &role,
                         int userId, const QString &userName, QObject *parent)
    : QObject(parent), m_mgr(manager), m_role(role), m_userId(userId), m_userName(userName)
{
    buildWidget();
    refresh();
}

void UILostFound::buildWidget()
{
    m_container = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_container);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // ── Title ──
    QLabel *title = new QLabel("Lost & Found Board");
    title->setStyleSheet("font-size: 20px; font-weight: bold; padding: 4px 0;");
    mainLayout->addWidget(title);

    // ── Filter bar ──
    QHBoxLayout *filterBar = new QHBoxLayout();
    filterBar->setSpacing(10);

    QLabel *filterLabel = new QLabel("Filter:");
    filterLabel->setStyleSheet("font-weight: bold;");
    filterBar->addWidget(filterLabel);

    m_filterCombo = new QComboBox();
    m_filterCombo->addItems({"All Posts", "Lost Items", "Found Items", "My Posts", "Open Only", "Claimed"});
    m_filterCombo->setMinimumWidth(140);
    filterBar->addWidget(m_filterCombo);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search by item name or location...");
    m_searchEdit->setClearButtonEnabled(true);
    filterBar->addWidget(m_searchEdit, 1);

    filterBar->addStretch();
    mainLayout->addLayout(filterBar);

    // ── Content: splitter with table on left, detail on right ──
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // Left: post table
    m_table = new QTableWidget();
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"Type", "Item", "Location", "Posted By", "Date", "Status"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setMinimumWidth(500);
    splitter->addWidget(m_table);

    // Right: detail panel
    QWidget *detailPanel = new QWidget();
    QVBoxLayout *detailLayout = new QVBoxLayout(detailPanel);
    detailLayout->setContentsMargins(8, 0, 0, 0);
    detailLayout->setSpacing(8);

    QLabel *detailTitle = new QLabel("Post Details");
    detailTitle->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px 0;"
                               "border-bottom: 2px solid palette(mid);");
    detailLayout->addWidget(detailTitle);

    m_detailView = new QTextBrowser();
    m_detailView->setOpenExternalLinks(false);
    m_detailView->setMinimumWidth(260);
    m_detailView->setPlaceholderText("Select a post to view details...");
    detailLayout->addWidget(m_detailView, 1);

    // Action buttons
    QHBoxLayout *actionBar = new QHBoxLayout();
    m_btnClaim = new QPushButton("Mark as Claimed");
    m_btnClaim->setEnabled(false);
    m_btnClaim->setStyleSheet("QPushButton { padding: 6px 16px; font-weight: bold; }");
    actionBar->addWidget(m_btnClaim);

    m_btnDelete = new QPushButton("Delete Post");
    m_btnDelete->setEnabled(false);
    m_btnDelete->setStyleSheet("QPushButton { padding: 6px 16px; color: #c0392b; font-weight: bold; }");
    actionBar->addWidget(m_btnDelete);

    actionBar->addStretch();
    detailLayout->addLayout(actionBar);

    detailPanel->setMinimumWidth(280);
    splitter->addWidget(detailPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter, 1);

    // ── New post form ──
    QGroupBox *formBox = new QGroupBox("Report a Lost or Found Item");
    formBox->setStyleSheet("QGroupBox { font-weight: bold; padding-top: 16px; margin-top: 8px; }");
    QHBoxLayout *formLayout = new QHBoxLayout(formBox);
    formLayout->setSpacing(10);

    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"LOST", "FOUND"});
    m_typeCombo->setMinimumWidth(90);
    formLayout->addWidget(m_typeCombo);

    m_editItemName = new QLineEdit();
    m_editItemName->setPlaceholderText("Item name...");
    m_editItemName->setMinimumWidth(150);
    formLayout->addWidget(m_editItemName);

    m_editLocation = new QLineEdit();
    m_editLocation->setPlaceholderText("Location...");
    m_editLocation->setMinimumWidth(150);
    formLayout->addWidget(m_editLocation);

    m_editDescription = new QTextEdit();
    m_editDescription->setPlaceholderText("Description (color, brand, etc.)...");
    m_editDescription->setMaximumHeight(60);
    m_editDescription->setMinimumWidth(200);
    formLayout->addWidget(m_editDescription, 1);

    m_btnPost = new QPushButton("Post");
    m_btnPost->setStyleSheet("QPushButton { padding: 8px 24px; font-weight: bold; font-size: 13px; }");
    formLayout->addWidget(m_btnPost);

    mainLayout->addWidget(formBox);

    // ── Connections ──
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UILostFound::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &UILostFound::onFilterChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &UILostFound::onPostSelected);
    connect(m_btnPost, &QPushButton::clicked, this, &UILostFound::onPostClicked);
    connect(m_btnClaim, &QPushButton::clicked, this, &UILostFound::onClaimClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &UILostFound::onDeleteClicked);
}

void UILostFound::refresh()
{
    m_posts = m_mgr->getLostFoundPosts();
    applyFilter();
}

void UILostFound::applyFilter()
{
    m_filteredPosts.clear();
    QString filter = m_filterCombo->currentText();
    QString search = m_searchEdit->text().trimmed().toLower();

    for (const auto &post : m_posts)
    {
        // Category filter
        if (filter == "Lost Items" && post.getType() != "LOST") continue;
        if (filter == "Found Items" && post.getType() != "FOUND") continue;
        if (filter == "My Posts" && post.getPosterId() != m_userId) continue;
        if (filter == "Open Only" && post.getStatus() != "OPEN") continue;
        if (filter == "Claimed" && post.getStatus() != "CLAIMED") continue;

        // Search filter
        if (!search.isEmpty())
        {
            bool matches = post.getItemName().toLower().contains(search)
                        || post.getLocation().toLower().contains(search)
                        || post.getDescription().toLower().contains(search);
            if (!matches) continue;
        }

        m_filteredPosts.append(post);
    }

    // Populate table
    m_table->setRowCount(m_filteredPosts.size());
    for (int i = 0; i < m_filteredPosts.size(); ++i)
    {
        const auto &p = m_filteredPosts[i];

        QString typeLabel = p.getType() == "LOST" ? "LOST" : "FOUND";
        QString statusLabel = p.getStatus() == "OPEN" ? "Open" : "Claimed";

        auto *typeItem = new QTableWidgetItem(typeLabel);
        typeItem->setTextAlignment(Qt::AlignCenter);
        if (p.getType() == "LOST")
            typeItem->setForeground(QColor("#e74c3c"));
        else
            typeItem->setForeground(QColor("#27ae60"));

        auto *itemItem = new QTableWidgetItem(p.getItemName());
        auto *locItem = new QTableWidgetItem(p.getLocation());
        auto *posterItem = new QTableWidgetItem(p.getPosterName() + " (" + p.getPosterRole() + ")");
        auto *dateItem = new QTableWidgetItem(p.getDate());
        auto *statusItem = new QTableWidgetItem(statusLabel);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (p.getStatus() == "CLAIMED")
            statusItem->setForeground(QColor("#7f8c8d"));

        m_table->setItem(i, 0, typeItem);
        m_table->setItem(i, 1, itemItem);
        m_table->setItem(i, 2, locItem);
        m_table->setItem(i, 3, posterItem);
        m_table->setItem(i, 4, dateItem);
        m_table->setItem(i, 5, statusItem);
    }

    m_table->resizeColumnsToContents();
    m_detailView->clear();
    m_btnClaim->setEnabled(false);
    m_btnDelete->setEnabled(false);
}

void UILostFound::onFilterChanged()
{
    applyFilter();
}

void UILostFound::onPostSelected()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_filteredPosts.size())
    {
        m_detailView->clear();
        m_btnClaim->setEnabled(false);
        m_btnDelete->setEnabled(false);
        return;
    }

    const auto &p = m_filteredPosts[row];

    QString typeColor = p.getType() == "LOST" ? "#e74c3c" : "#27ae60";
    QString statusColor = p.getStatus() == "OPEN" ? "#2980b9" : "#7f8c8d";

    QString html = QString(
        "<div style='font-family: sans-serif;'>"
        "<h3 style='margin-bottom:4px;'>%1</h3>"
        "<p><span style='color:%2; font-weight:bold; font-size:13px;'>"
        "%3</span>"
        " &nbsp; <span style='color:%4; font-weight:bold;'>%5</span></p>"
        "<hr>"
        "<p><b>Description:</b><br>%6</p>"
        "<p><b>Location:</b> %7</p>"
        "<p><b>Posted by:</b> %8 (%9)</p>"
        "<p><b>Date:</b> %10</p>"
        "%11"
        "</div>")
        .arg(p.getItemName())
        .arg(typeColor)
        .arg(p.getType())
        .arg(statusColor)
        .arg(p.getStatus())
        .arg(p.getDescription().isEmpty() ? "<i>No description</i>" : p.getDescription().toHtmlEscaped().replace("\n", "<br>"))
        .arg(p.getLocation().toHtmlEscaped())
        .arg(p.getPosterName().toHtmlEscaped())
        .arg(p.getPosterRole())
        .arg(p.getDate())
        .arg(p.getStatus() == "CLAIMED"
             ? QString("<p><b>Claimed by:</b> %1</p>").arg(p.getClaimedBy().toHtmlEscaped())
             : "");

    m_detailView->setHtml(html);

    // Enable claim only if post is open and user is not the poster
    m_btnClaim->setEnabled(p.getStatus() == "OPEN" && p.getPosterId() != m_userId);

    // Enable delete only if user is the poster or admin
    m_btnDelete->setEnabled(p.getPosterId() == m_userId || m_role == "Admin");
}

void UILostFound::onPostClicked()
{
    QString itemName = m_editItemName->text().trimmed();
    QString location = m_editLocation->text().trimmed();
    QString description = m_editDescription->toPlainText().trimmed();
    QString type = m_typeCombo->currentText();

    if (itemName.isEmpty())
    {
        QMessageBox::warning(m_container, "Missing Info", "Please enter an item name.");
        return;
    }
    if (location.isEmpty())
    {
        QMessageBox::warning(m_container, "Missing Info", "Please enter a location.");
        return;
    }

    // Sanitize newlines from description for CSV storage
    description.replace("\n", " | ");

    m_mgr->addLostFoundPost(m_userId, m_userName, m_role, type, itemName, description, location);

    // Clear form
    m_editItemName->clear();
    m_editLocation->clear();
    m_editDescription->clear();
    m_typeCombo->setCurrentIndex(0);
}

void UILostFound::onClaimClicked()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_filteredPosts.size()) return;

    const auto &p = m_filteredPosts[row];
    auto reply = QMessageBox::question(m_container, "Confirm Claim",
        QString("Mark \"%1\" as claimed by you (%2)?").arg(p.getItemName(), m_userName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
        m_mgr->claimLostFoundPost(p.getId(), m_userName);
}

void UILostFound::onDeleteClicked()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_filteredPosts.size()) return;

    const auto &p = m_filteredPosts[row];
    auto reply = QMessageBox::question(m_container, "Confirm Delete",
        QString("Delete the post for \"%1\"? This cannot be undone.").arg(p.getItemName()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
        m_mgr->deleteLostFoundPost(p.getId());
}
