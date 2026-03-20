#include "../include/ui_lostfound.hpp"
#include "../include/theme.hpp"
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

    // Add main title for the board
    QLabel *title = new QLabel("Lost & Found Board");
    title->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 4px 0;").arg(AppFonts::Large));
    mainLayout->addWidget(title);

    // Set up filter and search bar layout
    QHBoxLayout *filterBar = new QHBoxLayout();

    QLabel *filterLabel = new QLabel("Filter:");
    filterLabel->setStyleSheet("font-weight: bold;");
    filterBar->addWidget(filterLabel);

    m_filterCombo = new QComboBox();
    m_filterCombo->addItems({"All Posts", "Lost Items", "Found Items", "My Posts", "Open Only", "Claimed"});
    filterBar->addWidget(m_filterCombo);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search by item name or location...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    filterBar->addWidget(m_searchEdit, 1);

    filterBar->addStretch();
    mainLayout->addLayout(filterBar);

    // Create a splitter to separate the post list from the detail view
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // Initialize the table for listing posts
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
    splitter->addWidget(m_table);

    // Setup the detail panel for displaying selected post information
    QWidget *detailPanel = new QWidget();
    QVBoxLayout *detailLayout = new QVBoxLayout(detailPanel);
    detailLayout->setContentsMargins(8, 0, 0, 0);

    QLabel *detailTitle = new QLabel("Post Details");
    detailTitle->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 4px 0; border-bottom: 2px solid palette(mid);").arg(AppFonts::Normal));
    detailLayout->addWidget(detailTitle);

    m_detailView = new QTextBrowser();
    m_detailView->setOpenExternalLinks(false);
    m_detailView->setPlaceholderText("Select a post to view details...");
    detailLayout->addWidget(m_detailView, 1);

    // Provide action buttons for the selected post
    QHBoxLayout *actionBar = new QHBoxLayout();
    m_btnClaim = new QPushButton("Mark as Claimed");
    m_btnClaim->setEnabled(false);
    actionBar->addWidget(m_btnClaim);

    m_btnDelete = new QPushButton("Delete Post");
    m_btnDelete->setEnabled(false);
    actionBar->addWidget(m_btnDelete);

    actionBar->addStretch();
    detailLayout->addLayout(actionBar);

    splitter->addWidget(detailPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter, 1);

    // Create the form for submitting new lost or found posts
    QGroupBox *formBox = new QGroupBox("Report a Lost or Found Item");
    QHBoxLayout *formLayout = new QHBoxLayout(formBox);

    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"LOST", "FOUND"});
    formLayout->addWidget(m_typeCombo);

    m_editItemName = new QLineEdit();
    m_editItemName->setPlaceholderText("Item name...");
    m_editItemName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addWidget(m_editItemName);

    m_editLocation = new QLineEdit();
    m_editLocation->setPlaceholderText("Location...");
    m_editLocation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addWidget(m_editLocation);

    m_editDescription = new QTextEdit();
    m_editDescription->setPlaceholderText("Description (color, brand, etc.)...");
    m_editDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    formLayout->addWidget(m_editDescription, 1);

    m_btnPost = new QPushButton("Post");
    formLayout->addWidget(m_btnPost);

    mainLayout->addWidget(formBox);

    // Connect signals and slots for interactivity
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UILostFound::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &UILostFound::onFilterChanged);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &UILostFound::onPostSelected);
    connect(m_btnPost, &QPushButton::clicked, this, &UILostFound::onPostClicked);
    connect(m_btnClaim, &QPushButton::clicked, this, &UILostFound::onClaimClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &UILostFound::onDeleteClicked);
}

void UILostFound::refresh()
{
    // Retrieve posts and update the view
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
        // Apply category filtering
        if (filter == "Lost Items" && post.getType() != "LOST")
            continue;
        if (filter == "Found Items" && post.getType() != "FOUND")
            continue;
        if (filter == "My Posts" && post.getPosterId() != m_userId)
            continue;
        if (filter == "Open Only" && post.getStatus() != "OPEN")
            continue;
        if (filter == "Claimed" && post.getStatus() != "CLAIMED")
            continue;

        // Apply search keyword filtering
        if (!search.isEmpty())
        {
            bool matches = post.getItemName().toLower().contains(search) || post.getLocation().toLower().contains(search) || post.getDescription().toLower().contains(search);
            if (!matches)
                continue;
        }

        m_filteredPosts.append(post);
    }

    // Populate the table with filtered posts
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

    // Clear detail view on refresh
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
    // Disable actions if no valid row is selected
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

    // Format post details into HTML for presentation
    QString html = QString(
                       "<div style='font-family: \"%1\", sans-serif;'>"
                       "<h3 style='margin-bottom:4px; font-size:%13px;'>%2</h3>"
                       "<p><span style='color:%3; font-weight:bold; font-size:%14px;'>"
                       "%4</span>"
                       " &nbsp; <span style='color:%5; font-weight:bold; font-size:%14px;'>%6</span></p>"
                       "<hr>"
                       "<p style='font-size:%14px;'><b>Description:</b><br>%7</p>"
                       "<p style='font-size:%14px;'><b>Location:</b> %8</p>"
                       "<p style='font-size:%14px;'><b>Posted by:</b> %9 (%10)</p>"
                       "<p style='font-size:%14px;'><b>Date:</b> %11</p>"
                       "<div style='font-size:%14px;'>%12</div>"
                       "</div>")
                       .arg(AppFonts::Family)
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
                                ? QString("<b>Claimed by:</b> %1").arg(p.getClaimedBy().toHtmlEscaped())
                                : "")
                       .arg(AppFonts::Large)
                       .arg(AppFonts::Small);

    m_detailView->setHtml(html);

    // Enable claim button only if post is open and the user is not the author
    m_btnClaim->setEnabled(p.getStatus() == "OPEN" && p.getPosterId() != m_userId);

    // Enable delete button only if the user is the author or an admin
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

    // Clear form fields
    m_editItemName->clear();
    m_editLocation->clear();
    m_editDescription->clear();
    m_typeCombo->setCurrentIndex(0);
}

void UILostFound::onClaimClicked()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_filteredPosts.size())
        return;

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
    if (row < 0 || row >= m_filteredPosts.size())
        return;

    const auto &p = m_filteredPosts[row];
    auto reply = QMessageBox::question(m_container, "Confirm Delete",
                                       QString("Delete the post for \"%1\"? This cannot be undone.").arg(p.getItemName()),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
        m_mgr->deleteLostFoundPost(p.getId());
}
