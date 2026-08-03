#pragma once

#include <QString>

namespace vesper {

struct DiskSpaceCheck {
    bool sufficient = false;
    qint64 availableBytes = 0;
    qint64 requiredBytes = 0;

    QString shortfallDescription() const;
};

DiskSpaceCheck checkDiskSpace(const QString& directory, qint64 neededBytes);

qint64 directorySize(const QString& directory);

}
