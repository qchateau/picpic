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
    "hash varchar(128), "
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
        kColHash,
        kColPath,
        kColRating,
    };

    PicModel(QSqlDatabase db, QObject* parent) : QSqlTableModel(parent, db)
    {
        setTable(kPicturesTable);
        setEditStrategy(QSqlTableModel::OnFieldChange);
        setHeaderData(kColId, Qt::Horizontal, "ID");
        setHeaderData(kColHash, Qt::Horizontal, "Hash");
        setHeaderData(kColPath, Qt::Horizontal, "Path");
        setHeaderData(kColRating, Qt::Horizontal, "Rating");
    }

    void insert(const QString& path, const QString& hash, int rating)
    {
        int row = rowCount();
        insertRows(row, 1);
        setData(index(row, kColHash), hash);
        setData(index(row, kColPath), path);
        setData(index(row, kColRating), rating);
    }

private:
};

} // picpic
