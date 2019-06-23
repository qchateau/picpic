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
    "size bigint, "
    "stars tinyint"
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
    PicModel(QSqlDatabase db, QObject* parent) : QSqlTableModel(parent, db)
    {
        setTable(kPicturesTable);
        setEditStrategy(QSqlTableModel::OnManualSubmit);
        select();
        setHeaderData(0, Qt::Horizontal, "ID");
        setHeaderData(1, Qt::Horizontal, "Hash");
        setHeaderData(2, Qt::Horizontal, "Size");
        setHeaderData(3, Qt::Horizontal, "Stars");
    }

    void insert(const QString& hash, qint64 size, int stars)
    {
        int row = rowCount();
        insertRows(row, 1);
        setData(index(row, 1), hash);
        setData(index(row, 2), size);
        setData(index(row, 3), stars);
    }

private:
};

} // picpic
