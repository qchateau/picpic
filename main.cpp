#include <QApplication>
#include <QStyleFactory>

#include "main_window.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
#ifdef _WIN32
    app.setStyle(QStyleFactory::create("fusion"));
#endif

    picpic::MainWindow main_window;
    main_window.showMaximized();

    return app.exec();
}
