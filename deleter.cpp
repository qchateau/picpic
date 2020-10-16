#include "deleter.hpp"

namespace picpic {
namespace {

constexpr int kBatchSize = 10;
}

Deleter::Deleter(PicModel* model, QVector<int> rows, QObject* parent)
    : QObject(parent), model_{model}, rows_{std::move(rows)}
{
    connect(this, &Deleter::next, this, &Deleter::onNext, Qt::QueuedConnection);
    std::sort(rows_.begin(), rows_.end(), std::greater<int>{});
    it_ = rows_.begin();
    next();
}

void Deleter::onNext()
{
    progress(std::distance(rows_.begin(), it_));
    for (int i = 0; i < kBatchSize; ++i) {
        if (it_ == rows_.end()) {
            done(success_);
            return;
        }

        success_ &= model_->removeRow(*it_++);
    }
    next();
}

} // picpic
