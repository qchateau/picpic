#pragma once

#include <list>

#include <QListView>
#include <QMainWindow>
#include <QProgressBar>
#include <QSpinBox>
#include <QSqlTableModel>
#include <QTableView>

#include "exporter.hpp"
#include "file_scanner.hpp"
#include "file_view.hpp"
#include "image_viewer.hpp"
#include "pic_model.hpp"

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
    void updateExporters();
    void updateExportProgress(std::size_t nr_files);
    void onCopyDone(std::size_t copied);

    PicModel* model_{nullptr};
    FileView* file_view_{nullptr};
    QProgressBar* file_view_pg_{nullptr};
    QLabel* file_view_label_{nullptr};
    QSpinBox* filter_spin_box_{nullptr};
    ImageViewer* image_viewer_{nullptr};
    FileScanner* scanner_{nullptr};
    QVector<QString> dirs_to_scan_;
    std::list<Exporter> pending_exports_;
};

} // picpic
