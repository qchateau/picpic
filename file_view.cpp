#include "file_view.hpp"
#include "pic_model.hpp"

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

void FileView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Delete: {
        qDebug() << "Delete";
        QVector<std::size_t> rows = selectedRows();
        if (rows.empty()) {
            break;
        }

        std::sort(rows.begin(), rows.end(), std::greater<int>{});

        struct Batch {
            std::size_t row, count;
        };
        QVector<Batch> batches;
        for (std::size_t row : rows) {
            if (!batches.empty()
                && row == batches.back().row + batches.back().count) {
                batches.back().count++;
            }
            else {
                batches.push_back(Batch{row, 1});
            }
        }

        for (const Batch& batch : batches) {
            model()->removeRows(batch.row, batch.count);
        }
        break;
    }
    }
    QTableView::keyPressEvent(event);
}

} // picpic
