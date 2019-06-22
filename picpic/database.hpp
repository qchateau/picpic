#pragma once

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStringList>

namespace picpic {

constexpr const char* kPicturesTable = "pictures";
constexpr const char* kPicturesTableCreationQuery =
    "create table pictures ("
    "id integer primary key, "
    "hash varchar(128), "
    "size bigint, "
    "stars tinyint"
    ")";

inline QSqlError initializeDatabase()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("db.sqlite");
    if (!db.open()) {
        return db.lastError();
    }

    if (!db.tables().contains(kPicturesTable)) {
        QSqlQuery query(db);
        if (!query.exec(kPicturesTableCreationQuery)) {
            return query.lastError();
        }
    }

    return {};
}

namespace pictures {

QSqlTableModel* createModel(const QString& table, QObject* parent)
{
    QSqlTableModel* model = new QSqlTableModel(parent);
    model->setTable(table);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Hash");
    model->setHeaderData(2, Qt::Horizontal, "Size");
    model->setHeaderData(3, Qt::Horizontal, "Stars");
    return model;
}

void insert(QSqlTableModel* model, const QString& hash, qint64 size, int stars)
{
    int row = model->rowCount();
    model->insertRows(row, 1);
    model->setData(model->index(row, 1), hash);
    model->setData(model->index(row, 2), size);
    model->setData(model->index(row, 3), stars);
}

} // pictures

} // picpic
