#pragma once

#include <QCache>
#include <QLabel>
#include <QPixmap>

#include "image_loader.hpp"

namespace picpic {

class ImageViewer : public QLabel {
    Q_OBJECT
public:
    ImageViewer(QWidget* parent = nullptr);
    virtual QSize sizeHint() const override;
    virtual int heightForWidth(int width) const override;
    void rotate();
    void setImagePath(const QString& path);
    void preload(const QString& path);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void updatePixmap();

    QPixmap pixmap_;
    QCache<QString, QPixmap> pixmap_cache_;
    ImageLoader loader_;
    ImageLoader preloader_;
};

} // picpic
