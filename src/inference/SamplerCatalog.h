#pragma once

#include <QStringList>

namespace vesper {

QStringList availableSamplers();
QStringList availableSchedulers();
bool isKnownSampler(const QString& name);

}
