#pragma once

#include "inference/GenerationRequest.h"
#include "models/ModelManifest.h"

#include <QStringList>

namespace vesper {

struct ArgumentBuildResult {
    QStringList arguments;
    QString error;

    bool ok() const { return error.isEmpty(); }
};

ArgumentBuildResult buildArguments(const GenerationRequest& request, const ModelEntry& entry,
                                   const QString& modelsDirectory);

}
