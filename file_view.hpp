#pragma once

#include <QTableView>

namespace picpic {

class FileView : public QTableView {
    Q_OBJECT
public:
    using QTableView::selectedIndexes;

    FileView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;

signals:
    void rowSelected(const QModelIndex& index);

protected:
    void selectionChanged(
        const QItemSelection& selected,
        const QItemSelection& deselected) override;
};

} // picpic
