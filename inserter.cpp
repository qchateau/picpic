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
    connect(&file_scanner_, &FileScanner::done, this, [this] {
        done_ = true;

        if (pending_files_.empty()) {
            // next won't be called
            done(success_);
        }
    });
    connect(this, &Inserter::next, this, &Inserter::onNext, Qt::QueuedConnection);
    file_scanner_.start();
}

void Inserter::onNext()
{
    auto begin = pending_files_.begin();
    auto end = std::min(begin + kBatchSize, pending_files_.end());

    for (auto it = begin; it != end; ++it) {
        bool success = model_->insert(*it);
        if (!success) {
            qDebug() << "inserting" << *it
                     << "failed:" << model_->lastError().text();
        }
        success_ &= success;
    }

    model_->select();
    pending_files_.erase(begin, end);

    if (!pending_files_.empty()) {
        next();
    }
    else if (done_) {
        done(success_);
    }
}

} // picpic
