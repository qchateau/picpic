#include "image_viewer.hpp"

namespace picpic {

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
