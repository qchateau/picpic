#include <QApplication>

#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    picpic::MainWindow main_window;
    main_window.showMaximized();

    return app.exec();
}
