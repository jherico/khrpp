#include <QtGlobal>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>

void main(int argc, char** argv) {
    QGuiApplication app(argc, argv);

    QWindow* window = new QWindow();
    window->show();
    app.exec();
}



