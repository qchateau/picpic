#include "inserter.hpp"

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
    for (int i = 0; i < kBatchSize; ++i) {
        if (pending_files_.empty()) {
            break;
        }

        const QString& path = pending_files_.front();
        model_->cachedInsert(path, 0);
        pending_files_.pop_front();
    }

    if (!pending_files_.empty()) {
        next();
        return;
    }

    model_->submitInserts();
    model_->select();

    if (done_) {
        done();
    }
}

} // picpic
