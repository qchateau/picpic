#pragma once

#include <list>

#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
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
    bool keyEvent(QKeyEvent* event);

signals:
    void insertNextFile();
    void deleteNext(bool success = true);

private:
    void onNewFile(const QString& path);
    void onScanAction();
    void onNewAction();
    void onOpenAction();
    void onExportAction();
    void onDeleteSelection();

    void createActions();
    void createShortcuts();
    void createMainWidget();
    void createNewModel(const QString& path);

    void updateLabel();

    void onDeleteNext(bool success);

    void startNextScanner();
    void onInsertNextFile();
    void onScanDone();

    void updateExporters();
    void updateExportProgress(std::size_t nr_files);
    void onExportDone(std::size_t copied);

    QString db_path_;
    PicModel* model_{nullptr};
    ImageViewer* image_viewer_{nullptr};

    FileView* file_view_{nullptr};
    QLabel* file_view_label_{nullptr};
    QSpinBox* filter_spin_box_{nullptr};
    QProgressBar* export_pb_{nullptr};

    QAction* scan_action_{nullptr};
    QAction* export_action_{nullptr};

    std::list<Exporter> pending_exports_;

    QProgressDialog* delete_modal_{nullptr};
    QVector<std::size_t> pending_deletion_;

    QProgressDialog* scan_modal_{nullptr};
    FileScanner* file_scanner_{nullptr};
    QStringList pending_files_;
};

} // picpic
