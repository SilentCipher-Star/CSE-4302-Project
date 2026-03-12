#include "../include/custom_tables.hpp"
#include <QHeaderView>
#include <QVector>
#include <QResizeEvent>
#include <QShowEvent>

static void adjustWidthsImpl(QTableView *table)
{
    if (!table || !table->model())
        return;

    table->resizeColumnsToContents();

    int colCount = table->model()->columnCount();
    int totalWidth = 0;

    QVector<int> columnWidths(colCount);
    for (int i = 0; i < colCount; ++i)
    {
        columnWidths[i] = table->columnWidth(i);
        totalWidth += columnWidths[i];
    }

    if (totalWidth > 0)
    {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        int targetWidth = table->width() * 0.95;
        int usedWidth = 0;
        for (int i = 0; i < colCount; ++i)
        {
            int newWidth;
            if (i == colCount - 1)
                newWidth = targetWidth - usedWidth;
            else
                newWidth = int((double)columnWidths[i] / totalWidth * targetWidth);

            table->setColumnWidth(i, newWidth);
            usedWidth += newWidth;
        }
        table->horizontalHeader()->setStretchLastSection(true);
    }
}

void AcadenceTableView::adjustColumnWidths()
{
    if (isVisible() && width() > 50)
    {
        adjustWidthsImpl(this);
        m_pendingAdjust = false;
    }
    else
    {
        m_pendingAdjust = true;
    }
}

void AcadenceTableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    if (m_pendingAdjust)
        adjustColumnWidths();
}

void AcadenceTableView::showEvent(QShowEvent *event)
{
    QTableView::showEvent(event);
    if (m_pendingAdjust)
        adjustColumnWidths();
}

void AcadenceTableWidget::adjustColumnWidths()
{
    if (isVisible() && width() > 50)
    {
        adjustWidthsImpl(this);
        m_pendingAdjust = false;
    }
    else
    {
        m_pendingAdjust = true;
    }
}

void AcadenceTableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);
    if (m_pendingAdjust)
        adjustColumnWidths();
}

void AcadenceTableWidget::showEvent(QShowEvent *event)
{
    QTableWidget::showEvent(event);
    if (m_pendingAdjust)
        adjustColumnWidths();
}