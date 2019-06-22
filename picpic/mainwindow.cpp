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

MainWindow::MainWindow() : model_(new QSqlTableModel(this))
{
    // Database
    // TODO properly
    qDebug() << "Initalizing database";
    auto res = initializeDatabase();
    if (res.type() != QSqlError::NoError) {
        qDebug() << "failed: " << res.text();
    }

    model_ = pictures::createModel(kPicturesTable, this);

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
    pictures::insert(model_, hash, size, 0);
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

void MainWindow::onSaveAsAction()
{
    QString path = QFileDialog::getSaveFileName(this, "Save library as");
    qDebug() << "Saving as" << path;
    // TODO
}

void MainWindow::onNewAction()
{
    // TODO
}

void MainWindow::onOpenAction()
{
    QString path = QFileDialog::getOpenFileName(this, "Open library");
    qDebug() << "Opening" << path;
    // TODO
    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(path);
    if (!db.open()) {
        QMessageBox::warning(
            this, "Error", "Failed to open library: " + db.lastError().text());
    }
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

    QAction* save_as_act = new QAction(save_icon, "Save as", this);
    save_as_act->setShortcuts(QKeySequence::SaveAs);
    save_as_act->setStatusTip("Save current library as");
    connect(save_as_act, &QAction::triggered, this, &MainWindow::onSaveAsAction);

    QIcon open_icon = style()->standardIcon(QStyle::SP_DialogOpenButton);
    QAction* open_act = new QAction(open_icon, "&Open", this);
    open_act->setShortcuts(QKeySequence::Open);
    open_act->setStatusTip("Open a library");
    connect(open_act, &QAction::triggered, this, &MainWindow::onOpenAction);

    file_menu->addAction(new_act);
    file_menu->addAction(open_act);
    file_menu->addAction(save_act);
    file_menu->addAction(save_as_act);
    file_menu->addAction(scan_act);
}

void MainWindow::createMainWidget()
{
    QTableView* table_view = new QTableView(this);
    table_view->setModel(model_);
    table_view->setColumnHidden(0, true);
    table_view->verticalHeader()->setVisible(false);
    for (int c = 0; c < table_view->horizontalHeader()->count(); ++c) {
        table_view->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
    }

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(table_view, "Table");
    setCentralWidget(tabs);
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