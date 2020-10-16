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
    void load(const QString& path);

protected:
    void run() override;

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::optional<QString> path_;
};

} // picpic
