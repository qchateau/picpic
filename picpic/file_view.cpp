#include "file_view.hpp"
#include "database.hpp"

#include <QHeaderView>

namespace picpic {

FileView::FileView(QWidget* parent) : QTableView(parent)
{
    verticalHeader()->setVisible(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
}

void FileView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
    horizontalHeader()->setSectionResizeMode(
        PicModel::kColPath, QHeaderView::Stretch);
    setColumnHidden(PicModel::kColId, true);
    setColumnHidden(PicModel::kColHash, true);
    sortByColumn(PicModel::kColPath);
}

QModelIndex FileView::selected(int column) const
{
    if (selectedIndexes().empty()) {
        return QModelIndex();
    }
    return selectedIndexes().front().siblingAtColumn(column);
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