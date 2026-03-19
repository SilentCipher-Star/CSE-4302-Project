#pragma once
#include <QTableView>
#include <QTableWidget>

class QResizeEvent;
class QShowEvent;

class AcadenceTableView : public QTableView
{
    Q_OBJECT
public:
    using QTableView::QTableView;
    void adjustColumnWidths();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    bool m_pendingAdjust = false;
};

class AcadenceTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    using QTableWidget::QTableWidget;
    void adjustColumnWidths();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    bool m_pendingAdjust = false;
};