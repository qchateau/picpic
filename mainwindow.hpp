#pragma once

#include <QListView>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>

#include "database.hpp"
#include "file_scanner.hpp"
#include "file_view.hpp"
#include "image_viewer.hpp"

namespace picpic {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onNewFile(QString path);
    void onScanDone();
    void onScanAction();
    void onNewAction();
    void onOpenAction();

private:
    void createActions();
    void createShortcuts();
    void createMainWidget();
    void createNewModel(const QString& path);

    void startScanning(const QString& dir);

    PicModel* model_{nullptr};
    QModelIndex selection_;
    FileView* file_view_{nullptr};
    QLabel* file_status_{nullptr};
    ImageViewer* image_viewer_{nullptr};
    FileScanner* scanner_{nullptr};
    QVector<QString> dirs_to_scan_;
};

} // picpic
