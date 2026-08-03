#include "app/AppPaths.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSurfaceFormat>

int main(int argc, char* argv[]) {
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("VesperImageGenerator"));
    app.setApplicationDisplayName(QStringLiteral(VESPER_DISPLAY_NAME));
    app.setOrganizationName(QStringLiteral(VESPER_ORGANIZATION));
    app.setApplicationVersion(QStringLiteral(VESPER_VERSION));

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    if (!vesper::AppPaths::ensureDirectories()) {
        qWarning("Could not create the application data directories");
    }

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);

    engine.loadFromModule("Vesper", "Main");
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    return app.exec();
}
