#pragma once

#include <QString>

namespace vesper {

enum class GenerationMode {
    TextToImage,
    ImageToImage,
};

struct GenerationRequest {
    GenerationMode mode = GenerationMode::TextToImage;
    QString modelId;
    QString prompt;
    QString negativePrompt;
    QString initImagePath;
    double strength = 0.75;
    int steps = 20;
    double cfgScale = 7.0;
    QString sampler = QStringLiteral("euler_a");
    QString scheduler;
    int width = 512;
    int height = 512;
    qint64 seed = -1;
    int batchCount = 1;
    int threads = 0;
    QString backend;
    bool offloadToCpu = false;
    bool flashAttention = false;
    QString outputPath;
};

}
