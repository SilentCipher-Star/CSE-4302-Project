#include <QGraphicsDropShadowEffect>
#include "../include/ui_admin.hpp"
#include "../include/csvdelegate.hpp"
#include "../include/csvhandler.hpp"
#include "../include/utils.hpp"
#include "../include/exceptions.hpp"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <algorithm>
#include <QListWidget>

UIAdmin::UIAdmin(Ui::MainWindow *ui, AcadenceManager *manager, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager)
{
    adminModel = new QStandardItemModel(this);

    adminProxyModel = new QSortFilterProxyModel(this);
    adminProxyModel->setSourceModel(adminModel);
    adminProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    adminProxyModel->setFilterKeyColumn(-1);
    ui->adminTableView->setModel(adminProxyModel);

    csvDelegate = new CsvDelegate(this);
    ui->adminTableView->setItemDelegate(csvDelegate);

    suggestionList = new QListWidget(nullptr);

    // 2. Style it to look like a clean popup
    suggestionList->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(15);
    shadow->setXOffset(0);
    shadow->setYOffset(5);
    shadow->setColor(QColor(0, 0, 0, 80));
    suggestionList->setGraphicsEffect(shadow);

    suggestionList->setStyleSheet(
        "QListWidget {"
        "   border: 2px solid #ff007f;" // Your signature pink color
        "   background-color: #ffffff;"
        "   color: #333333;"
        "   font-size: 13px;"
        "   border-radius: 8px;"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   padding: 10px;"
        "   border-bottom: 1px solid #fce4ec;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #ff007f;"
        "   color: white;"
        "}");

    suggestionList->hide();

    adminProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // 3. Connect the click event so clicking a suggestion works
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &UIAdmin::onSearchTextChanged);
    // 3. Connect the click event so clicking a suggestion works
    connect(suggestionList, &QListWidget::itemClicked, this, &UIAdmin::onSuggestionClicked);
}

UIAdmin::~UIAdmin()
{
}

void UIAdmin::initialize()
{
    QStringList tables = {
        Constants::Table::Admins, Constants::Table::Students, Constants::Table::Teachers,
        Constants::Table::Courses, Constants::Table::Routine, Constants::Table::RoutineAdj,
        "grades", "assessments", "attendance", Constants::Table::Notices};
    ui->tableComboBox->addItems(tables);

    if (!tables.isEmpty())
        onTableComboBoxChanged(tables.first());
}

void UIAdmin::onTableComboBoxChanged(const QString &tableName)
{
    adminModel->clear();

    disconnect(adminModel, nullptr, this, nullptr);
    csvDelegate->currentTable = tableName;

    QVector<QStringList> data;
    try
    {
        data = CsvHandler::readCsv(tableName + ".csv");
    }
    catch (const Acadence::Exception &e)
    {
        QMessageBox::warning(nullptr, "Load Error", e.what());
    }
    for (const auto &row : data)
    {
        QList<QStandardItem *> items;
        for (const QString &field : row)
            items.append(new QStandardItem(field));
        adminModel->appendRow(items);
    }

    QStringList headers;
    if (tableName == Constants::Table::Admins)
        headers << "ID" << "Username" << "Password" << "Name" << "Email";
    else if (tableName == Constants::Table::Students)
        headers << "ID" << "Name" << "Email" << "Username" << "Password" << "Dept" << "Batch" << "Sem" << "Admission Date" << "CGPA";
    else if (tableName == Constants::Table::Teachers)
        headers << "ID" << "Name" << "Email" << "Username" << "Password" << "Dept" << "Designation" << "Salary";
    else if (tableName == Constants::Table::Courses)
        headers << "ID" << "Code" << "Name" << "Teacher ID" << "Semester" << "Credits";
    else if (tableName == Constants::Table::Routine)
        headers << "Day" << "Serial" << "Code" << "Name" << "Room" << "Instructor" << "Semester";
    else if (tableName == Constants::Table::RoutineAdj)
        headers << "Orig Date" << "Orig Serial" << "Type" << "New Date" << "New Serial" << "Code" << "Name" << "Room" << "Instructor" << "Semester";
    else if (tableName == "grades")
        headers << "Student ID" << "Assessment ID" << "Marks";
    else if (tableName == "assessments")
        headers << "ID" << "Course ID" << "Title" << "Type" << "Date" << "Max Marks";
    else if (tableName == "attendance")
        headers << "Student ID" << "Course ID" << "Date" << "Status";
    else if (tableName == "notices")
        headers << "Date" << "Author" << "Content";
    adminModel->setHorizontalHeaderLabels(headers);

    connect(adminModel, &QStandardItemModel::itemChanged, this, [this, tableName]()
            { Utils::saveTableData(adminModel, tableName); });

    QTimer::singleShot(10, this, [this]()
                       { Utils::adjustColumnWidths(ui->adminTableView); });
}

void UIAdmin::onAddRowClicked()
{
    int row = adminModel->rowCount();
    adminModel->insertRow(row);

    QString currentTable = ui->tableComboBox->currentText();
    QString nextIdStr = "";

    if (currentTable == Constants::Table::Admins || currentTable == Constants::Table::Students ||
        currentTable == Constants::Table::Teachers || currentTable == Constants::Table::Courses)
    {
        int maxId = 0;
        for (int i = 0; i < row; ++i)
        {
            QStandardItem *item = adminModel->item(i, 0);
            if (item && item->text().toInt() > maxId)
                maxId = item->text().toInt();
        }
        nextIdStr = QString::number(maxId + 1);
    }

    for (int j = 0; j < adminModel->columnCount(); ++j)
    {
        if (j == 0 && !nextIdStr.isEmpty())
            adminModel->setItem(row, j, new QStandardItem(nextIdStr));
        else
            adminModel->setItem(row, j, new QStandardItem(""));
    }

    Utils::saveTableData(adminModel, currentTable);
}

void UIAdmin::onDeleteRowClicked()
{
    QModelIndexList selected = ui->adminTableView->selectionModel()->selectedRows();
    if (selected.isEmpty())
    {
        QMessageBox::warning(nullptr, "Selection", "Please select a row to delete.");
        return;
    }

    QList<int> sourceRows;
    for (const auto &index : selected)
    {
        QModelIndex sourceIndex = adminProxyModel->mapToSource(index);
        if (sourceIndex.isValid())
            sourceRows.append(sourceIndex.row());
    }

    std::sort(sourceRows.begin(), sourceRows.end());
    for (int i = sourceRows.size() - 1; i >= 0; --i)
    {
        adminModel->removeRow(sourceRows[i]);
    }

    Utils::saveTableData(adminModel, ui->tableComboBox->currentText());
}

void UIAdmin::onSearchTextChanged(const QString &arg1)
{
    // Keep the table filtering active
    adminProxyModel->setFilterFixedString(arg1);

    // 1. If text is too short, hide the list and stop
    if (arg1.length() < 2)
    {
        suggestionList->hide();
        return;
    }

    // 2. Get suggestions from the Trie via the manager
    // In UIAdmin::onSearchTextChanged
    auto suggestions = myManager->getGlobalSearchSuggestions(arg1.toLower());

    // 3. If no suggestions found, hide the list
    if (suggestions.isEmpty())
    {
        suggestionList->hide();
    }
    else
    {
        // 4. Clear the old list and add new ones
        qDebug() << "Trie found" << suggestions.size() << "matches for" << arg1;
        suggestionList->clear();
        for (const auto &pair : suggestions)
        {
            // Format: "Name (Category)"
            suggestionList->addItem(pair.first + " (" + pair.second + ")");
        }

        // 5. Positioning Logic: Place the list exactly under the search bar
        // mapToGlobal converts the local search bar position to screen coordinates
        QPoint pos = ui->searchLineEdit->mapToGlobal(QPoint(0, ui->searchLineEdit->height()));
        suggestionList->move(pos);
        suggestionList->setFixedWidth(ui->searchLineEdit->width());

        // 6. Show the list and bring it to the front
        suggestionList->show();
        suggestionList->raise();
    }
}

void UIAdmin::onSuggestionClicked(QListWidgetItem *item)
{
    // 1. Get the full text (e.g., "John Doe (Student)")
    QString selectedText = item->text();

    // 2. Extract just the name before the parenthesis
    QString nameOnly = selectedText.split(" (").first();

    // 3. Put the name into the search bar
    ui->searchLineEdit->setText(nameOnly);

    // 4. Hide the suggestion list now that we're done
    suggestionList->hide();

    // 5. Logic to automatically switch to the correct table
    if (selectedText.contains("(Student)"))
    {
        ui->tableComboBox->setCurrentText("students");
    }
    else if (selectedText.contains("(Course)"))
    {
        ui->tableComboBox->setCurrentText("courses");
    }
    else if (selectedText.contains("(Teacher)"))
    {
        ui->tableComboBox->setCurrentText("teachers");
    }

    qDebug() << "User selected suggestion:" << nameOnly;
}
