#include "models/ModelManifest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace vesper {

FileRole fileRoleFromString(const QString& text) {
    if (text == QLatin1String("checkpoint")) return FileRole::Checkpoint;
    if (text == QLatin1String("diffusion_model")) return FileRole::DiffusionModel;
    if (text == QLatin1String("clip_l")) return FileRole::ClipL;
    if (text == QLatin1String("clip_g")) return FileRole::ClipG;
    if (text == QLatin1String("t5xxl")) return FileRole::T5xxl;
    if (text == QLatin1String("vae")) return FileRole::Vae;
    return FileRole::Unknown;
}

QString fileRoleToString(FileRole role) {
    switch (role) {
    case FileRole::Checkpoint: return QStringLiteral("checkpoint");
    case FileRole::DiffusionModel: return QStringLiteral("diffusion_model");
    case FileRole::ClipL: return QStringLiteral("clip_l");
    case FileRole::ClipG: return QStringLiteral("clip_g");
    case FileRole::T5xxl: return QStringLiteral("t5xxl");
    case FileRole::Vae: return QStringLiteral("vae");
    case FileRole::Unknown: break;
    }
    return QStringLiteral("unknown");
}

QString fileRoleToCliFlag(FileRole role) {
    switch (role) {
    case FileRole::Checkpoint: return QStringLiteral("--model");
    case FileRole::DiffusionModel: return QStringLiteral("--diffusion-model");
    case FileRole::ClipL: return QStringLiteral("--clip_l");
    case FileRole::ClipG: return QStringLiteral("--clip_g");
    case FileRole::T5xxl: return QStringLiteral("--t5xxl");
    case FileRole::Vae: return QStringLiteral("--vae");
    case FileRole::Unknown: break;
    }
    return QString();
}

bool ModelEntry::isValid() const {
    if (id.isEmpty() || name.isEmpty() || files.isEmpty()) {
        return false;
    }
    for (const ModelFile& file : files) {
        if (file.role == FileRole::Unknown || file.filename.isEmpty() || file.url.isEmpty()) {
            return false;
        }
        if (file.sha256.size() != 64) {
            return false;
        }
    }
    return true;
}

qint64 ModelEntry::computedSizeBytes() const {
    qint64 total = 0;
    for (const ModelFile& file : files) {
        total += file.sizeBytes;
    }
    return total;
}

const ModelFile* ModelEntry::fileForRole(FileRole role) const {
    for (const ModelFile& file : files) {
        if (file.role == role) {
            return &file;
        }
    }
    return nullptr;
}

ManifestParseResult parseManifest(const QByteArray& json) {
    ManifestParseResult result;

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        result.error = QStringLiteral("Manifest is not valid JSON: %1").arg(parseError.errorString());
        return result;
    }
    if (!document.isObject()) {
        result.error = QStringLiteral("Manifest root must be an object");
        return result;
    }

    const QJsonObject root = document.object();
    const QJsonArray models = root.value(QStringLiteral("models")).toArray();
    if (models.isEmpty()) {
        result.error = QStringLiteral("Manifest contains no models");
        return result;
    }

    for (const QJsonValue& value : models) {
        const QJsonObject object = value.toObject();

        ModelEntry entry;
        entry.id = object.value(QStringLiteral("id")).toString();
        entry.name = object.value(QStringLiteral("name")).toString();
        entry.family = object.value(QStringLiteral("family")).toString();
        entry.licenseName = object.value(QStringLiteral("license_name")).toString();
        entry.licenseUrl = object.value(QStringLiteral("license_url")).toString();
        entry.notes = object.value(QStringLiteral("notes")).toString();
        entry.defaultSampler =
            object.value(QStringLiteral("default_sampler")).toString(QStringLiteral("euler_a"));
        entry.defaultSteps = object.value(QStringLiteral("default_steps")).toInt(20);
        entry.defaultCfgScale = object.value(QStringLiteral("default_cfg_scale")).toDouble(7.0);
        entry.defaultWidth = object.value(QStringLiteral("default_width")).toInt(512);
        entry.defaultHeight = object.value(QStringLiteral("default_height")).toInt(512);

        const QJsonArray files = object.value(QStringLiteral("files")).toArray();
        for (const QJsonValue& fileValue : files) {
            const QJsonObject fileObject = fileValue.toObject();
            ModelFile file;
            file.role = fileRoleFromString(fileObject.value(QStringLiteral("role")).toString());
            file.filename = fileObject.value(QStringLiteral("filename")).toString();
            file.url = fileObject.value(QStringLiteral("url")).toString();
            file.sha256 = fileObject.value(QStringLiteral("sha256")).toString().toLower();
            file.sizeBytes = static_cast<qint64>(
                fileObject.value(QStringLiteral("size_bytes")).toDouble(0.0));
            entry.files.append(file);
        }

        entry.totalSizeBytes = static_cast<qint64>(
            object.value(QStringLiteral("total_size_bytes")).toDouble(0.0));
        if (entry.totalSizeBytes <= 0) {
            entry.totalSizeBytes = entry.computedSizeBytes();
        }

        if (!entry.isValid()) {
            result.error = QStringLiteral("Model entry '%1' is incomplete")
                               .arg(entry.id.isEmpty() ? QStringLiteral("<missing id>") : entry.id);
            result.models.clear();
            return result;
        }

        result.models.append(entry);
    }

    return result;
}

}
