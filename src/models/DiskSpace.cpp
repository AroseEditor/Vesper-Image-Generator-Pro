#include "models/DiskSpace.h"

#include "app/ByteFormat.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QStorageInfo>

namespace vesper {

namespace {
constexpr double kHeadroomFactor = 1.10;
}

QString DiskSpaceCheck::shortfallDescription() const {
    if (sufficient) {
        return QString();
    }
    return QStringLiteral("Needs %1 free but only %2 is available")
        .arg(ByteFormat::humanize(requiredBytes), ByteFormat::humanize(availableBytes));
}

DiskSpaceCheck checkDiskSpace(const QString& directory, qint64 neededBytes) {
    DiskSpaceCheck check;
    check.requiredBytes = static_cast<qint64>(static_cast<double>(neededBytes) * kHeadroomFactor);

    QStorageInfo storage(directory);
    if (!storage.isValid() || !storage.isReady()) {
        storage = QStorageInfo::root();
    }
    check.availableBytes = storage.bytesAvailable();
    check.sufficient = check.availableBytes >= check.requiredBytes;
    return check;
}

qint64 directorySize(const QString& directory) {
    qint64 total = 0;
    QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

}
