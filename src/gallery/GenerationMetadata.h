#pragma once

#include "inference/GenerationRequest.h"

#include <QJsonObject>
#include <QString>

namespace vesper {

struct GenerationMetadata {
    QString imagePath;
    QString modelId;
    QString prompt;
    QString negativePrompt;
    QString sampler;
    QString scheduler;
    QString mode;
    int steps = 0;
    double cfgScale = 0.0;
    int width = 0;
    int height = 0;
    qint64 seed = 0;
    double strength = 0.0;
    qint64 elapsedMs = 0;
    QString createdAt;
    QString appVersion;

    static GenerationMetadata fromRequest(const GenerationRequest& request);
    static QString sidecarPathFor(const QString& imagePath);
    static GenerationMetadata load(const QString& path, bool* ok = nullptr);

    QJsonObject toJson() const;
    static GenerationMetadata fromJson(const QJsonObject& object);
    bool save(const QString& path) const;
};

}
