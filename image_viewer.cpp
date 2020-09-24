#include "image_viewer.hpp"

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

ImageViewer::ImageViewer(QWidget* parent) : QLabel(parent)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
    loader_.start();
    connect(&loader_, &ImageLoader::pixmapLoaded, this, [this](QPixmap pixmap) {
        pixmap_ = pixmap;
        updatePixmap();
    });
}

QSize ImageViewer::sizeHint() const
{
    int w = this->width();
    return QSize(w, heightForWidth(w));
}

int ImageViewer::heightForWidth(int width) const
{
    return pixmap_.isNull()
               ? this->height()
               : ((qreal)pixmap_.height() * width) / pixmap_.width();
}

void ImageViewer::rotate()
{
    if (!pixmap_.isNull()) {
        QMatrix m;
        m.rotate(90);
        pixmap_ = pixmap_.transformed(m);
    }
    updatePixmap();
}

void ImageViewer::setImagePath(const QString& path)
{
    loader_.load(path);
}

void ImageViewer::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void ImageViewer::updatePixmap()
{
    if (!pixmap_.isNull()) {
        QPixmap scaled = pixmap_.scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QLabel::setPixmap(scaled);
    }
}

} // picpic
