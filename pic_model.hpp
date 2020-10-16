#pragma once

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStringList>

#include "image_loader.hpp"

namespace picpic {

QSqlDatabase openPicDatabase(const QString& path);

class PicModel : public QSqlTableModel {
    Q_OBJECT
public:
    enum Columns {
        kColId = 0,
        kColPath,
        kColRating,
    };

    PicModel(QSqlDatabase db, QObject* parent);

    bool insert(const QString& path, int rating = 0);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

protected:
    void queryChange() override;

private:
    mutable ImageLoader loader_;
    QMap<QString, QPixmap> thumbnails_;
    mutable QMap<QString, QModelIndex> loading_indices_;
};

} // picpic
