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
    static constexpr QCryptographicHash::Algorithm kAlgo{
        QCryptographicHash::Sha512};

    FileScanner(QObject* parent = nullptr) : QThread(parent) {}
    void setDir(const QString& dir) { root_ = dir; }

signals:
    void newFile(QString path, QString hash);
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
            QCryptographicHash hasher(kAlgo);
            hasher.addData(&file);
            QString hash = hasher.result().toHex();
            newFile(it.filePath(), hash);
        }
        done();
    }

private:
    QString root_;
};

} // picpic
