#include "main_window.hpp"

#include <cassert>

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>
#include <QSplitter>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyle>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>

#include "file_scanner.hpp"
#include "pic_model.hpp"

namespace picpic {

namespace {

constexpr int kMaxRating = 5;
constexpr int kInsertionBatchSize = 10;
constexpr int kDeleteBatchSize = 10;

class KeyListener : public QObject {
public:
    KeyListener(MainWindow* window) : QObject(window), window_{window} {}

protected:
    bool eventFilter(QObject*, QEvent* qevent) override
    {
        if (qevent->type() != QEvent::KeyPress) {
            return false;
        }
        QKeyEvent* event = static_cast<QKeyEvent*>(qevent);
        return window_->keyEvent(event);
    }

private:
    MainWindow* window_;
};

} // <anonymous>

MainWindow::MainWindow()
{
    // Listen keyboard events
    qApp->installEventFilter(new KeyListener(this));
    // UI
    createActions();
    createShortcuts();
    createMainWidget();
    // Connections
    connect(
        this,
        &MainWindow::insertNextFile,
        this,
        &MainWindow::onInsertNextFile,
        Qt::QueuedConnection);
    connect(
        this,
        &MainWindow::deleteNext,
        this,
        &MainWindow::onDeleteNext,
        Qt::QueuedConnection);
}

bool MainWindow::keyEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
        onDeleteSelection();
        return true;
    }
    return false;
}

void MainWindow::onNewFile(const QString& path)
{
    pending_files_.push_back(path);
    if (pending_files_.size() == 1) {
        insertNextFile();
    }
}

void MainWindow::onScanAction()
{
    if (!model_) {
        QMessageBox::warning(
            this, "No library", "Please create or open a library first");
        return;
    }

    QString path = QFileDialog::getExistingDirectory(this);
    if (path.isEmpty()) {
        return;
    }

    qDebug() << "scanning" << path;
    scan_modal_ =
        new QProgressDialog("Scanning files, please wait ...", "", 0, 0, this);
    scan_modal_->setCancelButton(nullptr);
    scan_modal_->open();

    file_scanner_ = new FileScanner(std::move(path), this);
    connect(file_scanner_, &FileScanner::newFile, this, &MainWindow::onNewFile);
    connect(file_scanner_, &FileScanner::done, this, [this] {
        delete file_scanner_;
        file_scanner_ = nullptr;
    });
    file_scanner_->start();
}

void MainWindow::onNewAction()
{
    QString path = QFileDialog::getSaveFileName(
        this, "New library", "db.sqlite", "SQLite (*.sqlite);;Any (*)");
    if (path.isEmpty()) {
        return;
    }
    qDebug() << "new" << path;
    QFile file(path);
    if (file.exists()) {
        file.remove();
    }
    createNewModel(path);
}

void MainWindow::onOpenAction()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open library", QString(), "SQLite (*.sqlite);;Any (*)");
    if (path.isEmpty()) {
        return;
    }
    qDebug() << "opening" << path;
    createNewModel(path);
}

void MainWindow::onExportAction()
{
    const auto selected_rows = file_view_->selectedRows();
    if (selected_rows.isEmpty()) {
        QMessageBox::warning(
            this, "No selection", "Please select pictures first");
        return;
    }

    QString dst_dir = QFileDialog::getExistingDirectory(this, "Destination");
    if (dst_dir.isEmpty()) {
        return;
    }

    auto srcs = [&]() {
        QVector<QString> srcs;
        srcs.reserve(selected_rows.size());
        for (int row : selected_rows) {
            srcs.push_back(
                model_->index(row, PicModel::kColPath).data().toString());
        };
        return srcs;
    }();
    qDebug() << "exporting" << srcs.size() << "files to" << dst_dir;

    pending_exports_.emplace_back(std::move(dst_dir), std::move(srcs), this);
    const auto& exporter = pending_exports_.back();
    connect(&exporter, &Exporter::progress, this, &MainWindow::updateExportProgress);
    connect(&exporter, &Exporter::done, this, &MainWindow::onExportDone);

    updateExporters();
}

void MainWindow::onDeleteSelection()
{
    auto rows = file_view_->selectedRows();
    if (rows.empty()) {
        return;
    }

    std::sort(rows.begin(), rows.end(), std::greater<int>{});
    pending_deletion_.clear();
    std::copy(
        std::begin(rows), std::end(rows), std::back_inserter(pending_deletion_));

    delete_modal_ = new QProgressDialog(
        "Deleting entries, please wait ...", "", 0, pending_deletion_.size(), this);
    delete_modal_->setCancelButton(nullptr);
    delete_modal_->open();
    deleteNext();
}

void MainWindow::onDeleteNext(bool success)
{
    delete_modal_->setValue(delete_modal_->maximum() - pending_deletion_.size());

    for (int i = 0; i < kDeleteBatchSize; ++i) {
        if (pending_deletion_.empty()) {
            break;
        }

        int row = pending_deletion_.front();
        success &= model_->removeRow(row);
        pending_deletion_.pop_front();
    }

    if (!pending_deletion_.empty()) {
        deleteNext(success);
    }
    else {
        model_->select();

        if (delete_modal_) {
            delete delete_modal_;
            delete_modal_ = nullptr;
        }

        if (!success) {
            QMessageBox::warning(
                this,
                "Delete error",
                QString("Error while deleting entries: %1")
                    .arg(model_->lastError().driverText()));
        }
    }
}

void MainWindow::createActions()
{
    QToolBar* toolbar = addToolBar("main toolbar");
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);

    QIcon new_icon = style()->standardIcon(QStyle::SP_FileIcon);
    QAction* new_act = new QAction(new_icon, "&New", this);
    new_act->setShortcuts(QKeySequence::New);
    new_act->setStatusTip("New library");
    connect(new_act, &QAction::triggered, this, &MainWindow::onNewAction);

    QIcon open_icon = style()->standardIcon(QStyle::SP_DialogOpenButton);
    QAction* open_act = new QAction(open_icon, "&Open", this);
    open_act->setShortcuts(QKeySequence::Open);
    open_act->setStatusTip("Open a library");
    connect(open_act, &QAction::triggered, this, &MainWindow::onOpenAction);

    QIcon scan_icon = style()->standardIcon(QStyle::SP_DriveHDIcon);
    QAction* scan_act = new QAction(scan_icon, "S&can directory", this);
    scan_act->setShortcut(QKeySequence("Ctrl+K"));
    scan_act->setStatusTip(
        "Scan a directory and add its content to the library");
    connect(scan_act, &QAction::triggered, this, &MainWindow::onScanAction);
    scan_act->setEnabled(false);
    scan_action_ = scan_act;

    QIcon save_icon = style()->standardIcon(QStyle::SP_ComputerIcon);
    QAction* export_act = new QAction(save_icon, "&Export selection", this);
    export_act->setShortcut(QKeySequence("Ctrl+E"));
    export_act->setStatusTip("Export pictures");
    connect(export_act, &QAction::triggered, this, &MainWindow::onExportAction);
    export_act->setEnabled(false);
    export_action_ = export_act;

    toolbar->addAction(new_act);
    toolbar->addAction(open_act);
    toolbar->addAction(scan_act);
    toolbar->addAction(export_act);
}

void MainWindow::createShortcuts()
{
    for (int i = 0; i < kMaxRating + 1; ++i) {
        QShortcut* shortcut = new QShortcut(QKeySequence('0' + i), this);
        connect(shortcut, &QShortcut::activated, [this, i] {
            for (int row : file_view_->selectedRows()) {
                model_->setData(model_->index(row, PicModel::kColRating), i);
            }
        });
    }

    QShortcut* rotate = new QShortcut(Qt::Key_R, this);
    connect(rotate, &QShortcut::activated, [this] { image_viewer_->rotate(); });
}

void MainWindow::createMainWidget()
{
    export_pb_ = new QProgressBar(this);
    export_pb_->setHidden(true);
    file_view_label_ = new QLabel(this);
    filter_spin_box_ = new QSpinBox(this);
    filter_spin_box_->setMinimum(0);
    filter_spin_box_->setMaximum(kMaxRating);
    file_view_ = new FileView(this);

    image_viewer_ = new ImageViewer(this);
    image_viewer_->setMinimumSize(800, 600);
    image_viewer_->setAlignment(Qt::AlignCenter);

    connect(file_view_, &FileView::rowSelected, this, &MainWindow::updateLabel);

    connect(file_view_, &FileView::rowSelected, [&](const QModelIndex& index) {
        QString path =
            model_->data(index.sibling(index.row(), PicModel::kColPath)).toString();
        qDebug() << "displaying" << path;
        image_viewer_->setImagePath(path);
        QFileInfo fileinfo(path);
    });

    connect(file_view_, &FileView::activated, [](const QModelIndex& index) {
        QString path =
            index.sibling(index.row(), PicModel::kColPath).data().toString();
        qDebug() << "opening" << path;
        QDesktopServices::openUrl(QUrl("file:///" + path));
    });

    connect(
        filter_spin_box_,
        static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
        [&](int value) { model_->setFilter(QString("rating>=%1").arg(value)); });

    QSplitter* central = new QSplitter(Qt::Horizontal);

    QVBoxLayout* llayout = new QVBoxLayout();
    QHBoxLayout* top_llayout = new QHBoxLayout();
    top_llayout->addWidget(new QLabel("Min rating:"));
    top_llayout->addWidget(filter_spin_box_);

    llayout->addWidget(file_view_label_);
    llayout->addLayout(top_llayout);
    llayout->addWidget(file_view_);
    llayout->addWidget(export_pb_);

    QWidget* lwid = new QWidget(this);
    lwid->setLayout(llayout);
    central->addWidget(lwid);

    QVBoxLayout* rlayout = new QVBoxLayout();
    rlayout->addWidget(image_viewer_);

    QWidget* rwid = new QWidget(this);
    rwid->setLayout(rlayout);
    central->addWidget(rwid);

    setCentralWidget(central);
}

void MainWindow::createNewModel(const QString& path)
{
    // Database
    if (model_) {
        delete model_;
        model_ = nullptr;
    }
    auto db = openPicDatabase(path);
    model_ = new PicModel(db, this);
    db_path_ = path;

    // Connect model
    connect(model_, &PicModel::modelReset, this, &MainWindow::updateLabel);

    // Update widgets that use the model
    file_view_->setModel(model_);
    model_->select();

    // Enable buttons
    scan_action_->setEnabled(true);
    export_action_->setEnabled(true);
}

void MainWindow::updateLabel()
{
    if (!model_ || !file_view_) {
        file_view_label_->clear();
        return;
    }

    file_view_label_->setText( //
        QString("Library: %1 - %2 files.\nSelected: %3")
            .arg(db_path_)
            .arg(QString::number(model_->rowCount()))
            .arg(file_view_->selectedRows().size()));
}

void MainWindow::onInsertNextFile()
{
    if (!model_) {
        pending_files_.clear();
        return;
    }

    for (int i = 0; i < kInsertionBatchSize; ++i) {
        if (pending_files_.empty()) {
            break;
        }

        const QString& path = pending_files_.front();
        model_->cachedInsert(path, 0);
        pending_files_.pop_front();
    }

    if (!pending_files_.empty()) {
        insertNextFile();
    }
    else {
        model_->submitInserts();
        model_->select();

        if (!file_scanner_ && scan_modal_) {
            delete scan_modal_;
            scan_modal_ = nullptr;
        }
    }
}

void MainWindow::updateExporters()
{
    if (pending_exports_.empty()) {
        export_pb_->setHidden(true);
    }
    else {
        export_pb_->setHidden(false);
        export_pb_->setMinimum(0);
        export_pb_->setMaximum(pending_exports_.front().nrFiles());
        export_pb_->setValue(0);
        pending_exports_.front().start();
    }
}

void MainWindow::updateExportProgress(int nr_files)
{
    export_pb_->setValue(nr_files);
}

void MainWindow::onExportDone(int copied)
{
    assert(!pending_exports_.empty());

    const auto& exporter = pending_exports_.front();
    if (copied != exporter.nrFiles()) {
        QMessageBox::warning(
            this,
            "Export error",
            QString("%1/%2 have been copied to %3")
                .arg(copied)
                .arg(exporter.nrFiles())
                .arg(exporter.dst()));
    }

    pending_exports_.pop_front();
    updateExporters();
}

} // picpic
