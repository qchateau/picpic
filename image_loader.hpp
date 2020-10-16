#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>

#include <QLabel>
#include <QPixmap>
#include <QThread>

namespace picpic {

class ImageLoader : public QThread {
    Q_OBJECT
signals:
    void pixmapLoaded(QString path, QPixmap pixmap);

public:
    using QThread::QThread;
    ~ImageLoader() override;

    void cancel();
    void load(const QString& path, QSize size = {});

protected:
    void run() override;

private:
    struct Request {
        QString path;
        QSize size;
    };

    std::mutex mutex_;
    std::condition_variable cv_;
    QList<Request> requests_;
};

} // picpic
