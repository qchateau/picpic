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

void ImageLoader::load(const QString& path)
{
    std::unique_lock lock{mutex_};
    path_ = path;
    cv_.notify_all();
}

void ImageLoader::run()
{
    qDebug() << "waiting for image to load";
    while (!isInterruptionRequested()) {
        QString path;
        {
            std::unique_lock lock{mutex_};
            cv_.wait(lock, [this]() {
                return path_ || isInterruptionRequested();
            });

            if (!path_) {
                continue;
            }

            path = *path_;
            path_.reset();
        }

        qDebug() << "loading" << path;
        QImageReader reader{path};
        reader.setAutoTransform(true);
        pixmapLoaded(QPixmap::fromImage(reader.read()));
        qDebug() << "loading" << path << "done";
    }
}

} // picpic
