#include "file_view.hpp"
#include "pic_model.hpp"

#include <QHeaderView>
#include <QMessageBox>

namespace picpic {

FileView::FileView(QWidget* parent) : QTableView(parent)
{
    verticalHeader()->setVisible(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
}

void FileView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
    horizontalHeader()->setSectionResizeMode(
        PicModel::kColPath, QHeaderView::Stretch);
    setColumnHidden(PicModel::kColId, true);
    sortByColumn(PicModel::kColPath, Qt::AscendingOrder);
    setTextElideMode(Qt::ElideLeft);
    setWordWrap(false);
}

QVector<std::size_t> FileView::selectedRows() const
{
    auto selected_rows = selectionModel()->selectedRows();
    QVector<std::size_t> rows;
    rows.reserve(selected_rows.size());
    for (const auto& select : selected_rows) {
        rows.push_back(select.row());
    }
    return rows;
}

void FileView::selectionChanged(
    const QItemSelection& selected,
    const QItemSelection& deselected)
{
    QAbstractItemView::selectionChanged(selected, deselected);
    if (selected.indexes().size() == 0) {
        return;
    }

    rowSelected(selected.indexes()[0]);
}

} // picpic
