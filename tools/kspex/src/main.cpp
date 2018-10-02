#include <QtGlobal>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>


static QtMessageHandler originalHandler;

static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
#if defined(Q_OS_WIN)
    OutputDebugStringA(message.toStdString().c_str());
    OutputDebugStringA("\n");
#endif
    originalHandler(type, context, message);
}


void installTestMessageHandler() {
    originalHandler = qInstallMessageHandler(messageHandler);
}

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
	installTestMessageHandler();
    QQuickView viewer;
	qDebug() << "Foo";
    viewer.setSource(QUrl("qrc:///qml/kspex/main.qml"));
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QQuickView::close);
    viewer.setMinimumSize(QSize(640, 360));
    viewer.show();
    return app.exec();
}
