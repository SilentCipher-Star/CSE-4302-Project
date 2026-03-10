#include "../include/ui_admin.hpp"
#include "../include/csvdelegate.hpp"
#include "../include/csvhandler.hpp"
#include "../include/utils.hpp"
#include "../include/exceptions.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <algorithm>

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
    adminProxyModel->setFilterFixedString(arg1);
}
