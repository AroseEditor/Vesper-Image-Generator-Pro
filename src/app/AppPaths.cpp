#include "app/AppPaths.h"

#include <QStandardPaths>

namespace vesper {

AppPaths::AppPaths(QObject* parent) : QObject(parent) {}

QString AppPaths::dataRoot() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString AppPaths::modelsDirectory() {
    return QDir(dataRoot()).filePath(QStringLiteral("models"));
}

QString AppPaths::galleryDirectory() {
    return QDir(dataRoot()).filePath(QStringLiteral("gallery"));
}

QString AppPaths::compositionsDirectory() {
    return QDir(dataRoot()).filePath(QStringLiteral("compositions"));
}

QString AppPaths::cacheDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

bool AppPaths::ensureDirectories() {
    const QStringList required{modelsDirectory(), galleryDirectory(), compositionsDirectory(),
                               cacheDirectory()};
    for (const QString& path : required) {
        if (!QDir().mkpath(path)) {
            return false;
        }
    }
    return true;
}

}
