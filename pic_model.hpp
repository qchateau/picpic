#pragma once

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStringList>

namespace picpic {

QSqlDatabase openPicDatabase(const QString& path);

class PicModel : public QSqlTableModel {
    Q_OBJECT
signals:
    void rowsChanged();

public:
    enum Columns {
        kColId = 0,
        kColPath,
        kColRating,
    };

    PicModel(QSqlDatabase db, QObject* parent);
    void cachedInsert(const QString& path, int rating = 0);
    bool submitInserts();
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex())
        override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void selectAll();
};

} // picpic
