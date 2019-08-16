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
    void newFile(QString path);
    void done();

protected:
    void run() override
    {
        QDirIterator it(root_, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            qDebug() << "Found" << path;
            newFile(path);
        }
        done();
    }

private:
    QString root_;
};

} // picpic
