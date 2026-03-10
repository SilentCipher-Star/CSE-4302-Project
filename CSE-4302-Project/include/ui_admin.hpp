#pragma once
#include <QListWidget>
#include <QListWidgetItem>
#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class CsvDelegate;

// Admin module: manages CSV table editing
class UIAdmin : public QObject
{
    Q_OBJECT

public:
    UIAdmin(Ui::MainWindow *ui, AcadenceManager *manager, QObject *parent = nullptr);
    ~UIAdmin();

    void initialize();

public slots:
    void onTableComboBoxChanged(const QString &tableName);
    void onAddRowClicked();
    void onDeleteRowClicked();
    void onSearchTextChanged(const QString &arg1);
    void onSuggestionClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    QStandardItemModel *adminModel;
    QSortFilterProxyModel *adminProxyModel;
    CsvDelegate *csvDelegate;
    QListWidget *suggestionList;
};
