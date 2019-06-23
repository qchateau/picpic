#include "mainwindow.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyle>
#include <QTableView>

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
    createMainWidget();
}

void MainWindow::onNewFile(QString hash, qint64 size)
{
    model_->insert(hash, size, 0);
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
    QString path = QFileDialog::getExistingDirectory(this);
    qDebug() << "Scanning" << path;
    startScanning(path);
}

void MainWindow::onSaveAction()
{
    qDebug() << "Saving";
    if (!model_->submitAll()) {
        QMessageBox::warning(
            this, "Error", "Could not save: " + model_->lastError().text());
    }
}

void MainWindow::onNewAction()
{
    QString path = QFileDialog::getSaveFileName(this, "New library");
    qDebug() << "New" << path;
    createNewModel(path);
}

void MainWindow::onOpenAction()
{
    QString path = QFileDialog::getOpenFileName(this, "Open library");
    qDebug() << "Opening" << path;
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

    QIcon save_icon = style()->standardIcon(QStyle::SP_DriveFDIcon);
    QAction* save_act = new QAction(save_icon, "&Save", this);
    save_act->setShortcuts(QKeySequence::Save);
    save_act->setStatusTip("Save current library");
    connect(save_act, &QAction::triggered, this, &MainWindow::onSaveAction);

    QIcon open_icon = style()->standardIcon(QStyle::SP_DialogOpenButton);
    QAction* open_act = new QAction(open_icon, "&Open", this);
    open_act->setShortcuts(QKeySequence::Open);
    open_act->setStatusTip("Open a library");
    connect(open_act, &QAction::triggered, this, &MainWindow::onOpenAction);

    file_menu->addAction(new_act);
    file_menu->addAction(open_act);
    file_menu->addAction(save_act);
    file_menu->addAction(scan_act);
}

void MainWindow::createMainWidget()
{
    table_view_ = new QTableView(this);
    table_view_->setColumnHidden(0, true);
    table_view_->verticalHeader()->setVisible(false);
    for (int c = 0; c < table_view_->horizontalHeader()->count(); ++c) {
        table_view_->horizontalHeader()->setSectionResizeMode(
            c, QHeaderView::Stretch);
    }

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(table_view_, "Table");
    setCentralWidget(tabs);
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
    table_view_->setModel(model_);
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
