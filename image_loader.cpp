#include "image_loader.hpp"

#include <QDebug>
#include <QImage>
#include <QImageReader>
#include <QPixmap>

namespace picpic {

ImageLoader::~ImageLoader()
{
    requestInterruption();
    cv_.notify_all();
    wait();
}

void ImageLoader::load(const QString& path, QSize size)
{
    std::unique_lock lock{mutex_};
    requests_.push_back(Request{path, size});
    cv_.notify_all();
}

void ImageLoader::run()
{
    while (!isInterruptionRequested()) {
        Request req;
        {
            std::unique_lock lock{mutex_};
            cv_.wait(lock, [this]() {
                return !requests_.empty() || isInterruptionRequested();
            });

            if (requests_.empty()) {
                continue;
            }

            req = requests_.front();
            requests_.pop_front();
        }

        qDebug() << "loading" << req.path;
        QImageReader reader{req.path};
        reader.setAutoTransform(true);
        QPixmap pixmap = QPixmap::fromImage(reader.read());
        if (req.size.isValid()) {
            pixmap = pixmap.scaled(
                req.size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        pixmapLoaded(req.path, pixmap);
        qDebug() << "loading" << req.path << "done";
    }
}

} // picpic
