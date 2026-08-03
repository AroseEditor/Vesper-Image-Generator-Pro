#pragma once

#include <QJsonObject>
#include <QString>
#include <QVector>

namespace vesper {

enum class FileRole {
    Checkpoint,
    DiffusionModel,
    ClipL,
    ClipG,
    T5xxl,
    Vae,
    Unknown,
};

FileRole fileRoleFromString(const QString& text);
QString fileRoleToString(FileRole role);
QString fileRoleToCliFlag(FileRole role);

struct ModelFile {
    FileRole role = FileRole::Unknown;
    QString filename;
    QString url;
    QString sha256;
    qint64 sizeBytes = 0;
};

struct ModelEntry {
    QString id;
    QString name;
    QString family;
    QString licenseName;
    QString licenseUrl;
    QString notes;
    QString defaultSampler;
    int defaultSteps = 20;
    double defaultCfgScale = 7.0;
    int defaultWidth = 512;
    int defaultHeight = 512;
    qint64 totalSizeBytes = 0;
    QVector<ModelFile> files;

    bool isValid() const;
    qint64 computedSizeBytes() const;
    const ModelFile* fileForRole(FileRole role) const;
};

struct ManifestParseResult {
    QVector<ModelEntry> models;
    QString error;

    bool ok() const { return error.isEmpty(); }
};

ManifestParseResult parseManifest(const QByteArray& json);

}
