#pragma once

#include <QObject>

#include "file_scanner.hpp"
#include "pic_model.hpp"

namespace picpic {

class Inserter : public QObject {
    Q_OBJECT
public:
    Inserter(PicModel* model, const QString& path, QObject* parent = nullptr);
    void onNext();

signals:
    void done();

    // private signal
    void next();

private:
    PicModel* model_;
    FileScanner file_scanner_;
    QStringList pending_files_;
    bool done_{false};
};

} // picpic
