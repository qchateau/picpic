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

private:
    void onNewFile(QString path);
    void onScanAction();
    void onNewAction();
    void onOpenAction();
    void onExportAction();

    void createActions();
    void createShortcuts();
    void createMainWidget();
    void createNewModel(const QString& path);

    void updateLabel();
    void updateScanners();
    void onScanDone();
    void updateExporters();
    void updateExportProgress(std::size_t nr_files);
    void onCopyDone(std::size_t copied);

    QString db_path_;
    PicModel* model_{nullptr};
    FileView* file_view_{nullptr};
    QProgressBar* file_view_pg_{nullptr};
    QLabel* file_view_label_{nullptr};
    QSpinBox* filter_spin_box_{nullptr};
    QAction* scan_action_{nullptr};
    QAction* export_action_{nullptr};
    ImageViewer* image_viewer_{nullptr};
    std::list<Exporter> pending_exports_;
    std::list<FileScanner> pending_scans_;
};

} // picpic
