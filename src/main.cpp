#include "UtilsIntr.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QFont font;
    font.setPixelSize(16);
    app.setFont(font);

    QCommandLineParser parser;
    parser.addPositionalArgument("inputs", "input file names", "[input file names]");
    parser.parse(app.arguments());
    auto inputs = parser.positionalArguments();

    const QUrl            url("qrc:/qml/Main.qml");
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    auto context = engine.rootContext();

    if (inputs.size() > 0) {
        context->setContextProperty("SELECTED_FILE", inputs[0]);
    } else {
#ifdef QT_DEBUG
        context->setContextProperty("SELECTED_FILE", R"(D:\Project\GHT_2B\GHT_2B\GHT_2B软件240505\60轨 V3.1.240412 源代码\超声数据60\Data\20201027\涂占宽-20201027 1246-13-60-2023110500100-0001.bmp)");
#endif
    }

    qmlRegisterSingletonInstance("GHT60", 1, 0, "QSUtils", QSUtils::Instance());
    qmlRegisterType<UtilsIntr>("GHT60", 1, 0, "Intr");

    engine.load(url);
    return app.exec();
}
