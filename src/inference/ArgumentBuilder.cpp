#include "inference/ArgumentBuilder.h"

#include "inference/SamplerCatalog.h"

#include <QDir>

namespace vesper {

ArgumentBuildResult buildArguments(const GenerationRequest& request, const ModelEntry& entry,
                                   const QString& modelsDirectory) {
    ArgumentBuildResult result;

    if (request.prompt.trimmed().isEmpty()) {
        result.error = QStringLiteral("Prompt is empty");
        return result;
    }
    if (request.outputPath.isEmpty()) {
        result.error = QStringLiteral("Output path is empty");
        return result;
    }
    if (entry.files.isEmpty()) {
        result.error = QStringLiteral("Model '%1' has no files").arg(entry.id);
        return result;
    }
    if (request.mode == GenerationMode::ImageToImage && request.initImagePath.isEmpty()) {
        result.error = QStringLiteral("Image to image needs a reference image");
        return result;
    }
    if (!isKnownSampler(request.sampler)) {
        result.error = QStringLiteral("Unknown sampler '%1'").arg(request.sampler);
        return result;
    }

    const QDir dir(modelsDirectory);
    QStringList args;

    args << QStringLiteral("--mode")
         << (request.mode == GenerationMode::ImageToImage ? QStringLiteral("img2img")
                                                          : QStringLiteral("txt2img"));

    for (const ModelFile& file : entry.files) {
        const QString flag = fileRoleToCliFlag(file.role);
        if (flag.isEmpty()) {
            result.error = QStringLiteral("File '%1' has an unusable role").arg(file.filename);
            return result;
        }
        args << flag << QDir::toNativeSeparators(dir.filePath(file.filename));
    }

    args << QStringLiteral("--prompt") << request.prompt;
    if (!request.negativePrompt.trimmed().isEmpty()) {
        args << QStringLiteral("--negative-prompt") << request.negativePrompt;
    }

    args << QStringLiteral("--steps") << QString::number(request.steps)
         << QStringLiteral("--cfg-scale") << QString::number(request.cfgScale, 'f', 2)
         << QStringLiteral("--sampling-method") << request.sampler
         << QStringLiteral("--width") << QString::number(request.width)
         << QStringLiteral("--height") << QString::number(request.height)
         << QStringLiteral("--seed") << QString::number(request.seed)
         << QStringLiteral("--batch-count") << QString::number(qMax(1, request.batchCount));

    if (!request.scheduler.isEmpty() && request.scheduler != QLatin1String("default")) {
        args << QStringLiteral("--scheduler") << request.scheduler;
    }

    if (request.mode == GenerationMode::ImageToImage) {
        args << QStringLiteral("--init-img") << QDir::toNativeSeparators(request.initImagePath)
             << QStringLiteral("--strength") << QString::number(request.strength, 'f', 2);
    }

    if (request.threads > 0) {
        args << QStringLiteral("--threads") << QString::number(request.threads);
    }
    if (!request.backend.isEmpty() && request.backend != QLatin1String("auto")) {
        args << QStringLiteral("--backend") << request.backend;
    }
    if (request.offloadToCpu) {
        args << QStringLiteral("--offload-to-cpu");
    }
    if (request.flashAttention) {
        args << QStringLiteral("--diffusion-fa");
    }

    args << QStringLiteral("--output") << QDir::toNativeSeparators(request.outputPath)
         << QStringLiteral("--verbose");

    result.arguments = args;
    return result;
}

}
