#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>

#include <QLabel>
#include <QPixmap>
#include <QThread>

namespace picpic {

class ImageLoader : public QThread {
    Q_OBJECT
signals:
    void pixmapLoaded(QPixmap);

public:
    using QThread::QThread;
    ~ImageLoader() override;
    void load(const QString& path);

protected:
    void run() override;

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::optional<QString> path_;
};

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
    void updatePixmap();

    QPixmap pixmap_;
    ImageLoader loader_;
};

} // picpic
