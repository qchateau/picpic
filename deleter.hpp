#pragma once

#include <QObject>

#include "pic_model.hpp"

namespace picpic {

class Deleter : public QObject {
    Q_OBJECT
public:
    Deleter(PicModel* model, QVector<int> rows, QObject* parent = nullptr);
    void onNext();

signals:
    void done(bool success);
    void progress(int row);

    // private signal
    void next();

private:
    PicModel* model_;
    QVector<int> rows_;
    QVector<int>::iterator it_;
    bool success_{true};
};

} // picpic
