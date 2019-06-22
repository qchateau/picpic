#pragma once

#include <future>

#include <QDir>
#include <QMainWindow>
#include <QSqlTableModel>

#include "file_scanner.hpp"

namespace picpic {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onNewFile(QString hash, qint64 size);
    void onScanDone();
    void onScanAction();
    void onSaveAction();
    void onSaveAsAction();
    void onNewAction();
    void onOpenAction();

private:
    void createActions();
    void createMainWidget();

    void startScanning(const QString& dir);

    QSqlTableModel* model_{nullptr};
    FileScanner* scanner_{nullptr};
    QVector<QString> dirs_to_scan_;
};

} // picpic
