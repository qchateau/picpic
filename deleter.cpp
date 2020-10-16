#include "deleter.hpp"

namespace picpic {
namespace {

constexpr int kBatchSize = 1000;

}

Deleter::Deleter(PicModel* model, const QVector<int>& rows, QObject* parent)
    : QObject(parent), model_{model}
{
    connect(this, &Deleter::next, this, &Deleter::onNext, Qt::QueuedConnection);

    std::transform(
        rows.begin(), rows.end(), std::back_inserter(ids_), [this](int row) {
            return QString::number(
                model_->data(model_->index(row, PicModel::kColId)).toInt());
        });

    it_ = ids_.begin();
    next();
}

void Deleter::onNext()
{
    progress(std::distance(ids_.begin(), it_));

    if (it_ == ids_.end()) {
        done(success_);
        return;
    }

    QStringList ids;
    auto end = std::min(it_ + kBatchSize, ids_.end());
    ids.reserve(end - it_);
    std::move(it_, end, std::back_inserter(ids));
    it_ = end;

    QSqlQuery query(model_->database());
    success_ &= query.exec(QString("DELETE FROM %1 WHERE id IN (%2)")
                               .arg(model_->tableName(), ids.join(',')));

    next();
}

} // picpic
