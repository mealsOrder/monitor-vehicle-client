#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("RTP Stream Viewer");
    window.resize(800, 600);
    window.show();

    return app.exec();
}