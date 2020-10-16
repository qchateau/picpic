#include "file_scanner.hpp"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>

namespace picpic {

FileScanner::FileScanner(QString dir, QObject* parent)
    : QThread(parent), root_{std::move(dir)}
{
}

void FileScanner::run()
{
    QRegularExpression regex{".*\\.(jpg|jpeg|png|bmp|gif)"};
    QDirIterator it(root_, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (!regex.match(path).hasMatch()) {
            continue;
        }
        qDebug() << "new file:" << path;
        newFile(path);
    }
    done();
}

} // picpic
