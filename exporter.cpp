#include "exporter.hpp"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace picpic {

Exporter::Exporter(QString dst_dir, QVector<QString> srcs, QObject* parent)
    : QThread(parent), dst_dir_{std::move(dst_dir)}, srcs_{std::move(srcs)}
{
}

void Exporter::run()
{
    int count = 0;
    int copied = 0;
    for (const auto& src : srcs_) {
        progress(count++);
        QFile from{src};
        QFile to{dst_dir_ + '/' + QFileInfo(src).fileName()};

        if (to.exists()) {
            qDebug() << to.fileName() << "already exists";
            continue;
        }

        from.copy(to.fileName());
        if (from.error()) {
            qDebug() << "failed to copy" << src << ":" << from.errorString();
        }
        else {
            ++copied;
        }
    }

    qDebug() << "copied" << copied << "/" << count;
    progress(count);
    done(copied);
}

} // picpic
