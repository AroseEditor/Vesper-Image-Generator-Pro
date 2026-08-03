#include "inference/SdCliLocator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>

namespace vesper {

QString sdCliExecutableName() {
#ifdef Q_OS_WIN
    return QStringLiteral("sd-cli.exe");
#else
    return QStringLiteral("sd-cli");
#endif
}

QString locateSdCli() {
    const QString name = sdCliExecutableName();

    const QString override =
        QProcessEnvironment::systemEnvironment().value(QStringLiteral("VESPER_SD_CLI"));
    if (!override.isEmpty() && QFileInfo(override).isExecutable()) {
        return override;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates{
        QDir(appDir).filePath(name),
        QDir(appDir).filePath(QStringLiteral("bin/") + name),
        QDir(appDir).filePath(QStringLiteral("../Resources/") + name),
        QDir(appDir).filePath(QStringLiteral("third_party/stable-diffusion.cpp/bin/") + name),
        QDir(appDir).filePath(QStringLiteral("../third_party/stable-diffusion.cpp/bin/") + name),
    };

    for (const QString& candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

}
