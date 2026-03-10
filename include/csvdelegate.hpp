#pragma once

#include <QStyledItemDelegate>
#include <QString>

class CsvDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CsvDelegate(QObject *parent = nullptr);

    class ValidationError
    {
    public:
        ValidationError(const QString &msg) : message(msg) {}
        QString message;
    };

    QString currentTable;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};