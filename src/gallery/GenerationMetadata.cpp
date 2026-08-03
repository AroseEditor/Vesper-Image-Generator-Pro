#include "gallery/GenerationMetadata.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

namespace vesper {

GenerationMetadata GenerationMetadata::fromRequest(const GenerationRequest& request) {
    GenerationMetadata metadata;
    metadata.imagePath = request.outputPath;
    metadata.modelId = request.modelId;
    metadata.prompt = request.prompt;
    metadata.negativePrompt = request.negativePrompt;
    metadata.sampler = request.sampler;
    metadata.scheduler = request.scheduler;
    metadata.mode = request.mode == GenerationMode::ImageToImage ? QStringLiteral("img2img")
                                                                 : QStringLiteral("txt2img");
    metadata.steps = request.steps;
    metadata.cfgScale = request.cfgScale;
    metadata.width = request.width;
    metadata.height = request.height;
    metadata.seed = request.seed;
    metadata.strength = request.strength;
    metadata.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata.appVersion = QStringLiteral(VESPER_VERSION);
    return metadata;
}

QString GenerationMetadata::sidecarPathFor(const QString& imagePath) {
    const QFileInfo info(imagePath);
    return info.dir().filePath(info.completeBaseName() + QStringLiteral(".json"));
}

QJsonObject GenerationMetadata::toJson() const {
    return QJsonObject{
        {QStringLiteral("model_id"), modelId},
        {QStringLiteral("prompt"), prompt},
        {QStringLiteral("negative_prompt"), negativePrompt},
        {QStringLiteral("sampler"), sampler},
        {QStringLiteral("scheduler"), scheduler},
        {QStringLiteral("mode"), mode},
        {QStringLiteral("steps"), steps},
        {QStringLiteral("cfg_scale"), cfgScale},
        {QStringLiteral("width"), width},
        {QStringLiteral("height"), height},
        {QStringLiteral("seed"), static_cast<double>(seed)},
        {QStringLiteral("strength"), strength},
        {QStringLiteral("elapsed_ms"), static_cast<double>(elapsedMs)},
        {QStringLiteral("created_at"), createdAt},
        {QStringLiteral("app_version"), appVersion},
    };
}

GenerationMetadata GenerationMetadata::fromJson(const QJsonObject& object) {
    GenerationMetadata metadata;
    metadata.modelId = object.value(QStringLiteral("model_id")).toString();
    metadata.prompt = object.value(QStringLiteral("prompt")).toString();
    metadata.negativePrompt = object.value(QStringLiteral("negative_prompt")).toString();
    metadata.sampler = object.value(QStringLiteral("sampler")).toString();
    metadata.scheduler = object.value(QStringLiteral("scheduler")).toString();
    metadata.mode = object.value(QStringLiteral("mode")).toString();
    metadata.steps = object.value(QStringLiteral("steps")).toInt();
    metadata.cfgScale = object.value(QStringLiteral("cfg_scale")).toDouble();
    metadata.width = object.value(QStringLiteral("width")).toInt();
    metadata.height = object.value(QStringLiteral("height")).toInt();
    metadata.seed = static_cast<qint64>(object.value(QStringLiteral("seed")).toDouble());
    metadata.strength = object.value(QStringLiteral("strength")).toDouble();
    metadata.elapsedMs = static_cast<qint64>(object.value(QStringLiteral("elapsed_ms")).toDouble());
    metadata.createdAt = object.value(QStringLiteral("created_at")).toString();
    metadata.appVersion = object.value(QStringLiteral("app_version")).toString();
    return metadata;
}

bool GenerationMetadata::save(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(QJsonDocument(toJson()).toJson(QJsonDocument::Indented)) > 0;
}

GenerationMetadata GenerationMetadata::load(const QString& path, bool* ok) {
    if (ok) {
        *ok = false;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return {};
    }
    if (ok) {
        *ok = true;
    }
    return fromJson(document.object());
}

}
