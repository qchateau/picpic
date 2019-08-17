#pragma once

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStringList>

namespace picpic {

constexpr const char* kPicturesConnectionName = "pictures";
constexpr const char* kPicturesTable = "pictures";
constexpr const char* kPicturesTableCreationQuery =
    "create table pictures ("
    "id integer primary key, "
    "path varchar(4096), "
    "rating tinyint"
    ")";

inline QSqlDatabase openPicDatabase(const QString& path)
{
    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE", kPicturesConnectionName);
    db.setDatabaseName(path);
    if (db.open()) {
        if (!db.tables().contains(kPicturesTable)) {
            QSqlQuery query(db);
            if (!query.exec(kPicturesTableCreationQuery)) {
                qDebug() << "failed to create pictures table:"
                         << query.lastError().text();
            }
            else {
                qDebug() << "created pictures table";
            }
        }
    }
    return db;
}

class PicModel : public QSqlTableModel {
    Q_OBJECT
public:
    enum Columns {
        kColId = 0,
        kColPath,
        kColRating,
    };

    PicModel(QSqlDatabase db, QObject* parent) : QSqlTableModel(parent, db)
    {
        setTable(kPicturesTable);
        setEditStrategy(QSqlTableModel::OnFieldChange);
        select();
        setHeaderData(kColId, Qt::Horizontal, "ID");
        setHeaderData(kColPath, Qt::Horizontal, "Path");
        setHeaderData(kColRating, Qt::Horizontal, "Rating");
    }

    void insert(const QString& path, int rating = 0)
    {
        int row = rowCount();
        insertRows(row, 1);
        setData(index(row, kColPath), path);
        setData(index(row, kColRating), rating);
        submitAll();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
    {
        if (index.column() == kColRating && role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        }
        else {
            return QSqlTableModel::data(index, role);
        }
    }
};

} // picpic
