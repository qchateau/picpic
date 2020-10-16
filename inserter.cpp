#include "inserter.hpp"

#include <QDebug>

namespace picpic {
namespace {

constexpr int kBatchSize = 10;

}

Inserter::Inserter(PicModel* model, const QString& path, QObject* parent)
    : QObject(parent), model_{model}, file_scanner_{path}
{
    connect(&file_scanner_, &FileScanner::newFile, this, [this](const QString& path) {
        pending_files_.push_back(path);
        if (pending_files_.size() == 1) {
            next();
        }
    });
    connect(&file_scanner_, &FileScanner::done, this, [this] { done_ = true; });
    connect(this, &Inserter::next, this, &Inserter::onNext, Qt::QueuedConnection);
    file_scanner_.start();
}

void Inserter::onNext()
{
    if (pending_files_.empty() && done_) {
        done();
        return;
    }

    auto begin = pending_files_.begin();
    auto end = std::min(begin + kBatchSize, pending_files_.end());

    for (auto it = begin; it != end; ++it) {
        if (!model_->insert(*it)) {
            qDebug() << "insertion failed:" << model_->lastError().driverText();
        }
    }

    model_->select();
    pending_files_.erase(begin, end);
    next();
}

} // picpic
