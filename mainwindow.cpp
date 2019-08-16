#include "mainwindow.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>
#include <QSplitter>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyle>
#include <QTableView>
#include <QVBoxLayout>

#include "database.hpp"
#include "file_scanner.hpp"

namespace picpic {

MainWindow::MainWindow()
{
    // File scanner
    scanner_ = new FileScanner(this);
    connect(scanner_, &FileScanner::newFile, this, &MainWindow::onNewFile);
    connect(scanner_, &FileScanner::done, this, &MainWindow::onScanDone);

    // UI
    createActions();
    createShortcuts();
    createMainWidget();
}

void MainWindow::onNewFile(QString path)
{
    if (model_) {
        qDebug() << "New file:" << path;
        model_->insert(path, 0);
    }
}

void MainWindow::onScanDone()
{
    dirs_to_scan_.pop_front();
    if (dirs_to_scan_.size() > 0) {
        scanner_->setDir(dirs_to_scan_.front());
        scanner_->start();
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
    qDebug() << "scanning" << path;
    startScanning(path);
}

void MainWindow::onNewAction()
{
    QString path = QFileDialog::getSaveFileName(this, "New library");
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
    QString path = QFileDialog::getOpenFileName(this, "Open library");
    if (path.isEmpty()) {
        return;
    }
    qDebug() << "opening" << path;
    createNewModel(path);
}

void MainWindow::createActions()
{
    QMenu* file_menu = menuBar()->addMenu("&File");

    QIcon scan_icon = style()->standardIcon(QStyle::SP_DriveHDIcon);
    QAction* scan_act = new QAction(scan_icon, "S&can directory", this);
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

    file_menu->addAction(new_act);
    file_menu->addAction(open_act);
    file_menu->addAction(scan_act);
}

void MainWindow::createShortcuts()
{
    auto rate = [&](int rating) {
        QModelIndex index = file_view_->selected(PicModel::kColRating);
        if (index.model()) {
            model_->setData(index, rating);
        }
        else {
            qDebug() << "No model selected";
        }
    };

    for (int i = 0; i < 6; ++i) {
        QShortcut* shortcut = new QShortcut(QKeySequence('0' + i), this);
        connect(shortcut, &QShortcut::activated, [=] { rate(i); });
    }

    QShortcut* rotate = new QShortcut(Qt::Key_R, this);
    connect(rotate, &QShortcut::activated, [this] { image_viewer_->rotate(); });
}

void MainWindow::createMainWidget()
{
    file_view_ = new FileView(this);
    image_viewer_ = new ImageViewer(this);
    image_viewer_->setMinimumSize(800, 600);
    file_status_ = new QLabel(this);

    connect(file_view_, &FileView::rowSelected, [&](const QModelIndex& index) {
        QString path =
            model_->data(index.siblingAtColumn(PicModel::kColPath)).toString();
        qDebug() << "displaying" << path;
        image_viewer_->setImagePath(path);
        QFileInfo fileinfo(path);
        file_status_->setText(fileinfo.fileName());
    });

    QSplitter* central = new QSplitter(Qt::Horizontal);
    central->addWidget(file_view_);

    QVBoxLayout* rlayout = new QVBoxLayout();
    rlayout->addWidget(image_viewer_);
    rlayout->addWidget(file_status_);

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

    // Update widgets that use the model
    file_view_->setModel(model_);
}

void MainWindow::startScanning(const QString& dir)
{
    dirs_to_scan_.push_back(dir);
    if (dirs_to_scan_.size() == 1) {
        scanner_->setDir(dir);
        scanner_->start();
    }
}

} // picpic
