#pragma once

#include <QListView>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QProgressBar>

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
    void onExportAction();

private:
    void createActions();
    void createShortcuts();
    void createMainWidget();
    void createNewModel(const QString& path);

    void startScanning(const QString& dir);

    PicModel* model_{nullptr};
    FileView* file_view_{nullptr};
    QProgressBar* file_view_pg_{nullptr};
    QLabel* file_view_label_{nullptr};
    ImageViewer* image_viewer_{nullptr};
    QLabel* image_status_{nullptr};
    FileScanner* scanner_{nullptr};
    QVector<QString> dirs_to_scan_;
};

} // picpic
