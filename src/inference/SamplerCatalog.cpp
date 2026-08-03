#include "inference/SamplerCatalog.h"

namespace vesper {

QStringList availableSamplers() {
    return {
        QStringLiteral("euler"),      QStringLiteral("euler_a"),
        QStringLiteral("heun"),       QStringLiteral("dpm2"),
        QStringLiteral("dpm++2s_a"),  QStringLiteral("dpm++2m"),
        QStringLiteral("dpm++2mv2"),  QStringLiteral("ipndm"),
        QStringLiteral("ipndm_v"),    QStringLiteral("lcm"),
        QStringLiteral("ddim_trailing"), QStringLiteral("tcd"),
        QStringLiteral("res_multistep"), QStringLiteral("er_sde"),
        QStringLiteral("lms"),
    };
}

QStringList availableSchedulers() {
    return {
        QStringLiteral("default"), QStringLiteral("discrete"), QStringLiteral("karras"),
        QStringLiteral("exponential"), QStringLiteral("ays"), QStringLiteral("gits"),
        QStringLiteral("smoothstep"), QStringLiteral("sgm_uniform"),
    };
}

bool isKnownSampler(const QString& name) {
    return availableSamplers().contains(name);
}

}
