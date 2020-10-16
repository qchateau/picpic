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

#include "deleter.hpp"
#include "exporter.hpp"
#include "file_scanner.hpp"
#include "file_view.hpp"
#include "image_viewer.hpp"
#include "inserter.hpp"
#include "pic_model.hpp"

namespace picpic {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    bool keyEvent(QKeyEvent* event);

private:
    void onNewAction();
    void onOpenAction();
    void onScanAction();
    void onExportAction();
    void onDeleteSelection();

    void createActions();
    void createShortcuts();
    void createMainWidget();
    void createNewModel(const QString& path);

    void updateLabel();

    QString db_path_;
    PicModel* model_{nullptr};
    ImageViewer* image_viewer_{nullptr};

    FileView* file_view_{nullptr};
    QLabel* file_view_label_{nullptr};
    QSpinBox* filter_spin_box_{nullptr};

    QAction* scan_action_{nullptr};
    QAction* export_action_{nullptr};

    QProgressDialog* export_modal_{nullptr};
    Exporter* exporter_{nullptr};

    QProgressDialog* delete_modal_{nullptr};
    Deleter* deleter_{nullptr};

    QProgressDialog* scan_modal_{nullptr};
    Inserter* inserter_{nullptr};
};

} // picpic
