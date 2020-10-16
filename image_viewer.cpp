#include "image_viewer.hpp"

namespace picpic {
namespace {

constexpr int kCachedPictured = 5;

}

ImageViewer::ImageViewer(QWidget* parent)
    : QLabel(parent), pixmap_cache_(kCachedPictured)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
    loader_.start();
    connect(
        &loader_,
        &ImageLoader::pixmapLoaded,
        this,
        [this](const QString& path, const QPixmap& pixmap) {
            pixmap_cache_.insert(path, new QPixmap(pixmap));
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
    QPixmap* cached = pixmap_cache_[path];
    if (cached) {
        pixmap_ = *cached;
        updatePixmap();
        return;
    }

    loader_.load(path);
}

void ImageViewer::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void ImageViewer::updatePixmap()
{
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
