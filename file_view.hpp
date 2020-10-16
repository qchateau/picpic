#pragma once

#include <QTableView>

namespace picpic {

class FileView : public QTableView {
    Q_OBJECT
public:
    FileView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;
    QVector<int> selectedRows() const;
};

} // picpic
