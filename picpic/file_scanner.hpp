#pragma once

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QThread>

namespace picpic {

class FileScanner : public QThread {
    Q_OBJECT
public:
    FileScanner(QObject* parent = nullptr) : QThread(parent) {}
    void setDir(const QString& dir) { root_ = dir; }

signals:
    void newFile(QString hash, qint64 size);
    void done();

protected:
    void run() override
    {
        QDirIterator it(root_, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFile file(it.next());
            if (!file.open(QIODevice::ReadOnly)) {
                continue;
            }
            QFileInfo info = it.fileInfo();
            QCryptographicHash hasher(QCryptographicHash::Sha512);
            hasher.addData(&file);
            QString hash = hasher.result().toHex();
            qint64 size = info.size();
            newFile(hash, size);
        }
        done();
    }

private:
    QString root_;
};

} // picpic
