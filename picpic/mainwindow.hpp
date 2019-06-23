#pragma once

#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>

#include "database.hpp"
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
    void onNewAction();
    void onOpenAction();

private:
    void createActions();
    void createMainWidget();
    void createNewModel(const QString& path);

    void startScanning(const QString& dir);

    PicModel* model_{nullptr};
    QTableView* table_view_{nullptr};
    FileScanner* scanner_{nullptr};
    QVector<QString> dirs_to_scan_;
};

} // picpic
