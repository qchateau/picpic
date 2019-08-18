#include "mainwindow.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QRegularExpression>
#include <QShortcut>
#include <QSplitter>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyle>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDesktopServices>

#include "database.hpp"
#include "file_scanner.hpp"

namespace picpic {

namespace {

constexpr int kMaxRating = 5;

} // <anonymous>

MainWindow::MainWindow()
{
    // File scanner
    scanner_ = new FileScanner(this);
    connect(
        scanner_,
        &FileScanner::newFile,
        this,
        &MainWindow::onNewFile,
        Qt::QueuedConnection);
    connect(
        scanner_,
        &FileScanner::done,
        this,
        &MainWindow::onScanDone,
        Qt::QueuedConnection);

    // UI
    createActions();
    createShortcuts();
    createMainWidget();
}

void MainWindow::onNewFile(QString path)
{
    if (!model_) {
        return;
    }

    QRegularExpression regex{".*\\.(jpg|jpeg|png|bmp|gif)"};
    if (regex.match(path).hasMatch()) {
        qDebug() << "new file:" << path;
        model_->insert(path, 0);
    }
}

void MainWindow::onScanDone()
{
    if (dirs_to_scan_.size() > 0) {
        scanner_->setDir(dirs_to_scan_.front());
        dirs_to_scan_.pop_front();
        scanner_->start();
        file_view_pg_->setHidden(false);
        file_view_pg_->setMaximum(0);
        file_view_pg_->setMinimum(0);
    }
    else {
        file_view_pg_->setHidden(true);
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
    if (!path.isEmpty()) {
        qDebug() << "scanning" << path;
        startScanning(path);
    }
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
    if (file_view_->selectedIndexes().isEmpty()) {
        QMessageBox::warning(
            this, "No selection", "Please select pictures first");
        return;
    }

    QString dst_dir = QFileDialog::getExistingDirectory(this, "Destination");
    if (dst_dir.isEmpty()) {
        return;
    }

    // TODO: async this to allow graphical feedback
    qDebug() << "exporting to" << dst_dir;

    file_view_pg_->setMinimum(0);
    file_view_pg_->setMaximum(file_view_->selectedIndexes().size());
    file_view_pg_->setHidden(false);

    int cnt{0};
    int copied{0};
    for (const QModelIndex& index : file_view_->selectedIndexes()) {
        file_view_pg_->setValue(cnt++);
        QString src = index.sibling(index.row(), PicModel::kColPath).data().toString();
        if (QFile::copy(src, dst_dir + '/' + QFileInfo(src).fileName())) {
            ++copied;
            qDebug() << "copied" << src;
        }
        else {
            qDebug() << "failed to copy" << src;
        }
    }

    qDebug() << "copied" << copied << "/" << cnt;
    file_view_pg_->setHidden(true);
}

void MainWindow::createActions()
{
    QToolBar* toolbar = addToolBar("main toolbar");
    toolbar->setMovable(false);
    toolbar->setFloatable(false);

    QIcon scan_icon = style()->standardIcon(QStyle::SP_DriveHDIcon);
    QAction* scan_act = new QAction(scan_icon, "S&can directory", this);
    scan_act->setShortcut(QKeySequence("Ctrl+K"));
    scan_act->setStatusTip(
        "Scan a directory and add its content to the library");
    connect(scan_act, &QAction::triggered, this, &MainWindow::onScanAction);

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

    QIcon save_icon = style()->standardIcon(QStyle::SP_ComputerIcon);
    QAction* export_act = new QAction(save_icon, "&Export selection", this);
    export_act->setShortcut(QKeySequence("Ctrl+E"));
    export_act->setStatusTip("Export pictures");
    connect(export_act, &QAction::triggered, this, &MainWindow::onExportAction);

    toolbar->addAction(new_act);
    toolbar->addAction(open_act);
    toolbar->addAction(scan_act);
    toolbar->addAction(export_act);
}

void MainWindow::createShortcuts()
{
    for (int i = 0; i < kMaxRating + 1; ++i) {
        QShortcut* shortcut = new QShortcut(QKeySequence('0' + i), this);
        connect(shortcut, &QShortcut::activated, [=] {
            for (const QModelIndex& index : file_view_->selectedIndexes()) {
                model_->setData(index.sibling(index.row(), PicModel::kColRating), i);
            }
        });
    }

    QShortcut* rotate = new QShortcut(Qt::Key_R, this);
    connect(rotate, &QShortcut::activated, [this] { image_viewer_->rotate(); });
}

void MainWindow::createMainWidget()
{
    file_view_pg_ = new QProgressBar(this);
    file_view_pg_->setHidden(true);
    file_view_label_ = new QLabel(this);
    filter_spin_box_ = new QSpinBox(this);
    filter_spin_box_->setMinimum(0);
    filter_spin_box_->setMaximum(kMaxRating);
    file_view_ = new FileView(this);

    image_viewer_ = new ImageViewer(this);
    image_viewer_->setMinimumSize(800, 600);
    image_viewer_->setAlignment(Qt::AlignCenter);

    connect(file_view_, &FileView::rowSelected, [&](const QModelIndex& index) {
        QString path =
            model_->data(index.sibling(index.row(), PicModel::kColPath)).toString();
        qDebug() << "displaying" << path;
        image_viewer_->setImagePath(path);
        QFileInfo fileinfo(path);
    });

    connect(file_view_, &FileView::activated, [this] (const QModelIndex& index) {
        QString path = index.sibling(index.row(), PicModel::kColPath).data().toString();
        qDebug() << "opening" << path;
        QDesktopServices::openUrl(QUrl("file:///"+path));
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
    llayout->addWidget(file_view_pg_);

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
    auto db = openPicDatabase(path);
    if (model_) {
        delete model_;
        model_ = nullptr;
    }
    model_ = new PicModel(db, this);
    file_view_label_->setText(
        QString("Library: %1 - %2 files")
            .arg(QFileInfo(path).fileName(), QString::number(model_->rowCount())));

    // Update widgets that use the model
    file_view_->setModel(model_);
}

void MainWindow::startScanning(const QString& dir)
{
    dirs_to_scan_.push_back(dir);
    onScanDone(); // restart a new scan
}

} // picpic
