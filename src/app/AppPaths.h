#pragma once

#include <QDir>
#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace vesper {

class AppPaths : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString modelsDirectory READ modelsDirectory CONSTANT)
    Q_PROPERTY(QString galleryDirectory READ galleryDirectory CONSTANT)
    Q_PROPERTY(QString compositionsDirectory READ compositionsDirectory CONSTANT)

public:
    explicit AppPaths(QObject* parent = nullptr);

    static QString dataRoot();
    static QString modelsDirectory();
    static QString galleryDirectory();
    static QString compositionsDirectory();
    static QString cacheDirectory();

    static bool ensureDirectories();
};

}
