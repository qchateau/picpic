#include "image_viewer.hpp"

#include <QPixmap>
#include <QImage>
#include <QImageReader>

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

QString ImageViewer::path() const
{
    return path_;
}

void ImageViewer::setImagePath(const QString& path)
{
    path_ = path;

    QImageReader reader{path};
    reader.setAutoTransform(true);
    pixmap_ = QPixmap::fromImage(reader.read());

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
