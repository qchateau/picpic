#include "image_viewer.hpp"

#include <QPixmap>

namespace picpic {

ImageViewer::ImageViewer(QWidget* parent) : QLabel(parent)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
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
    QMatrix m;
    m.rotate(90);
    pixmap_ = pixmap_.transformed(m);
    QLabel::setPixmap(scaledPixmap());
}

void ImageViewer::setImagePath(const QString& path)
{
    pixmap_ = QPixmap(path);
    QLabel::setPixmap(scaledPixmap());
}

void ImageViewer::resizeEvent(QResizeEvent*)
{
    if (!pixmap_.isNull()) {
        QLabel::setPixmap(scaledPixmap());
    }
}

QPixmap ImageViewer::scaledPixmap() const
{
    return pixmap_.scaled(
        this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

} // picpic
