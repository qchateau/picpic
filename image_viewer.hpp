#pragma once

#include <QLabel>
#include <QPixmap>

namespace picpic {

class ImageViewer : public QLabel {
    Q_OBJECT
public:
    ImageViewer(QWidget* parent = nullptr);
    virtual QSize sizeHint() const override;
    virtual int heightForWidth(int width) const override;
    void rotate();

public slots:
    void setImagePath(const QString& path);
    void resizeEvent(QResizeEvent*) override;

private:
    QPixmap scaledPixmap() const;

    QPixmap pixmap_;
};

} // picpic
