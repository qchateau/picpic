#pragma once

#include <QThread>

namespace picpic {

class FileScanner : public QThread {
    Q_OBJECT
public:
    FileScanner(QString dir, QObject* parent = nullptr);

signals:
    void newFile(QString path);
    void done();

protected:
    void run() override;

private:
    QString root_;
};

} // picpic
