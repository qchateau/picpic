#include "file_view.hpp"
#include "database.hpp"

#include <QHeaderView>
#include <QKeyEvent>

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
    setTextElideMode(Qt::ElideLeft);
    sortByColumn(PicModel::kColPath, Qt::AscendingOrder);
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

void FileView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Delete: {
        qDebug() << "Delete";
        QVector<int> rows;
        for (const auto& select : this->selectedIndexes()) {
            rows.push_back(select.row());
        }
        std::sort(rows.begin(), rows.end(), std::greater<int>{});
        for (int row : rows) {
            model()->removeRow(row);
        }
        break;
    }
    }
    QTableView::keyPressEvent(event);
}

} // picpic