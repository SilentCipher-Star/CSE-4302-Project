#include "../include/ui_personal_inbox.hpp"
#include "../include/theme.hpp"
#include <QFont>
#include <QFrame>
#include <QMessageBox>
#include <QDateTime>

PersonalInboxDialog::PersonalInboxDialog(AcadenceManager *manager, int userId, const QString &role, QWidget *parent)
    : QDialog(parent), myManager(manager), userId(userId), userRole(role)
{
    setWindowTitle("Personal Inbox");
    setMinimumSize(750, 500);
    resize(850, 560);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ── Title bar ──
    QHBoxLayout *titleBar = new QHBoxLayout();

    lblTitle = new QLabel("Personal Inbox");
    lblTitle->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 2px 0;").arg(AppFonts::Large));
    titleBar->addWidget(lblTitle);

    titleBar->addStretch();

    lblUnread = new QLabel();
    lblUnread->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 4px 10px; border-radius: 10px;").arg(AppFonts::Small));
    titleBar->addWidget(lblUnread);

    btnRefresh = new QPushButton("Refresh");
    btnRefresh->setStyleSheet("padding: 5px 12px;");
    titleBar->addWidget(btnRefresh);

    mainLayout->addLayout(titleBar);

    // ── Separator ──
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #444;");
    mainLayout->addWidget(sep);

    // ── Splitter: message list | message view ──
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // Left: message list
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    messageList = new QListWidget();
    messageList->setStyleSheet(QString(
                                   "QListWidget { font-size: %1px; }"
                                   "QListWidget::item { padding: 8px 6px; border-bottom: 1px solid palette(mid); }"
                                   "QListWidget::item:selected { background-color: palette(highlight); color: palette(highlightedText); }")
                                   .arg(AppFonts::Small));
    leftLayout->addWidget(messageList);

    splitter->addWidget(leftPanel);

    // Right: message content viewer
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    messageView = new QTextEdit();
    messageView->setReadOnly(true);
    messageView->setStyleSheet(QString("font-size: %1px; padding: 8px;").arg(AppFonts::Small));
    messageView->setPlaceholderText("Select a message to read...");
    rightLayout->addWidget(messageView);

    // Delete button below content
    QHBoxLayout *actionBar = new QHBoxLayout();
    actionBar->addStretch();
    btnDelete = new QPushButton("Delete Message");
    btnDelete->setStyleSheet("padding: 5px 14px; color: #dc143c;");
    btnDelete->setEnabled(false);
    actionBar->addWidget(btnDelete);
    rightLayout->addLayout(actionBar);

    splitter->addWidget(rightPanel);

    // Set splitter sizes (40% list, 60% content)
    splitter->setSizes({320, 480});

    mainLayout->addWidget(splitter, 1);

    // ── Bottom close ──
    QHBoxLayout *bottomBar = new QHBoxLayout();
    bottomBar->addStretch();
    QPushButton *btnClose = new QPushButton("Close");
    btnClose->setFixedWidth(100);
    btnClose->setStyleSheet("padding: 6px 14px;");
    bottomBar->addWidget(btnClose);
    mainLayout->addLayout(bottomBar);

    // ── Connections ──
    connect(messageList, &QListWidget::currentRowChanged, this, &PersonalInboxDialog::onMessageSelected);
    connect(btnDelete, &QPushButton::clicked, this, &PersonalInboxDialog::onDeleteClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &PersonalInboxDialog::onRefreshClicked);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    // ── Initial load ──
    loadMessages();
}

void PersonalInboxDialog::loadMessages()
{
    messageList->clear();
    messageView->clear();
    btnDelete->setEnabled(false);
    messages = myManager->getMessages(userId);

    int unreadCount = 0;
    for (int i = 0; i < messages.size(); ++i)
    {
        const auto &msg = messages[i];

        QString preview = msg.getSubject();
        if (preview.length() > 50)
            preview = preview.left(47) + "...";

        QString displayText = QString("%1\n%2  -  %3")
                                  .arg(preview)
                                  .arg(msg.getSenderName())
                                  .arg(msg.getTimestamp());

        QListWidgetItem *item = new QListWidgetItem(displayText);

        if (!msg.getIsRead())
        {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            unreadCount++;
        }

        item->setData(Qt::UserRole, i); // store index
        messageList->addItem(item);
    }

    // Update unread badge
    if (unreadCount > 0)
    {
        lblUnread->setText(QString(" %1 unread ").arg(unreadCount));
        lblUnread->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 4px 10px; border-radius: 10px; background-color: #1a73e8; color: white;").arg(AppFonts::Small));
    }
    else
    {
        lblUnread->setText("All read");
        lblUnread->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 4px 10px; border-radius: 10px; color: #22a85a;").arg(AppFonts::Small));
    }

    lblTitle->setText(QString("Personal Inbox (%1 messages)").arg(messages.size()));
}

void PersonalInboxDialog::onMessageSelected()
{
    int row = messageList->currentRow();
    if (row < 0 || row >= messages.size())
    {
        messageView->clear();
        btnDelete->setEnabled(false);
        return;
    }

    const auto &msg = messages[row];

    // Mark as read
    if (!msg.getIsRead())
    {
        myManager->markMessageRead(msg.getId());

        // Update list item style
        QListWidgetItem *item = messageList->item(row);
        if (item)
        {
            QFont f = item->font();
            f.setBold(false);
            item->setFont(f);
        }
    }

    // Display the message
    QString html;
    html += "<div style='padding: 8px;'>";
    html += QString("<h2 style='margin-bottom: 4px; font-size: %1px;'>").arg(AppFonts::Normal) + msg.getSubject().toHtmlEscaped() + "</h2>";
    html += QString("<p style='color: palette(text); font-size: %1px; margin-top: 0;'>").arg(AppFonts::Small);
    html += "From: <b>" + msg.getSenderName().toHtmlEscaped() + "</b> (" + msg.getSenderRole() + ")";
    html += "  |  " + msg.getTimestamp();
    html += "</p>";
    html += "<hr style='border-color: palette(mid);'>";

    // Preserve line breaks in content
    QString body = msg.getContent().toHtmlEscaped();
    body.replace("\n", "<br>");
    html += QString("<div style='font-size: %1px; line-height: 1.6;'>").arg(AppFonts::Small) + body + "</div>";
    html += "</div>";

    messageView->setHtml(html);
    btnDelete->setEnabled(true);
}

void PersonalInboxDialog::onDeleteClicked()
{
    int row = messageList->currentRow();
    if (row < 0 || row >= messages.size())
        return;

    auto reply = QMessageBox::question(this, "Delete Message",
                                       "Are you sure you want to delete this message?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        myManager->deleteMessage(messages[row].getId());
        loadMessages();
    }
}

void PersonalInboxDialog::onRefreshClicked()
{
    loadMessages();
}
