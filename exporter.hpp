#pragma once

#include <QThread>
#include <QVector>

namespace picpic {

class Exporter : public QThread {
    Q_OBJECT
signals:
    void progress(int);
    void done(int);

public:
    Exporter(QString dst_dir, QVector<QString> srcs, QObject* parent = nullptr);
    std::size_t nrFiles() const { return srcs_.size(); }
    const QString& dst() const { return dst_dir_; }

protected:
    void run() override;

private:
    const QString dst_dir_;
    const QVector<QString> srcs_;
};

} // picpic
