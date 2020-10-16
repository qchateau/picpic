#include "image_viewer.hpp"

namespace picpic {
namespace {

constexpr int kCachedPictured = 5;

}

ImageViewer::ImageViewer(QWidget* parent)
    : QLabel(parent),
      pixmap_cache_(kCachedPictured),
      loader_(1),
      preloader_(kCachedPictured)
{
    setMinimumSize(1, 1);
    setScaledContents(false);

    connect(
        &loader_,
        &ImageLoader::pixmapLoaded,
        this,
        [this](const QString& path, const QPixmap& pixmap) {
            pixmap_cache_.insert(path, new QPixmap(pixmap));
            pixmap_ = pixmap;
            updatePixmap();
        });

    connect(
        &preloader_,
        &ImageLoader::pixmapLoaded,
        this,
        [this](const QString& path, const QPixmap& pixmap) {
            pixmap_cache_.insert(path, new QPixmap(pixmap));
        });

    loader_.start();
    preloader_.start();
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
    QPixmap* cached = pixmap_cache_[path];
    if (cached) {
        pixmap_ = *cached;
        updatePixmap();
        return;
    }

    setEnabled(false);
    loader_.load(path);
}

void ImageViewer::preload(const QString& path)
{
    if (pixmap_cache_[path]) {
        return;
    }

    preloader_.load(path);
}

void ImageViewer::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void ImageViewer::updatePixmap()
{
    setEnabled(true);
    if (pixmap_.isNull()) {
        clear();
    }
    else {
        QPixmap scaled = pixmap_.scaled(
            this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QLabel::setPixmap(scaled);
    }
}

} // picpic
