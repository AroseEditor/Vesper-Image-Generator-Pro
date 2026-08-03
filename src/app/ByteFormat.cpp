#include "app/ByteFormat.h"

namespace vesper {

ByteFormat::ByteFormat(QObject* parent) : QObject(parent) {}

QString ByteFormat::humanize(qint64 bytes) {
    if (bytes < 0) {
        return QStringLiteral("unknown");
    }
    constexpr double kilo = 1000.0;
    const double value = static_cast<double>(bytes);
    if (value < kilo) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (value < kilo * kilo) {
        return QStringLiteral("%1 KB").arg(value / kilo, 0, 'f', 0);
    }
    if (value < kilo * kilo * kilo) {
        return QStringLiteral("%1 MB").arg(value / (kilo * kilo), 0, 'f', 1);
    }
    return QStringLiteral("%1 GB").arg(value / (kilo * kilo * kilo), 0, 'f', 2);
}

QString ByteFormat::humanizeRate(double bytesPerSecond) {
    if (bytesPerSecond <= 0.0) {
        return QString();
    }
    return QStringLiteral("%1/s").arg(humanize(static_cast<qint64>(bytesPerSecond)));
}

QString ByteFormat::humanizeDuration(qint64 seconds) {
    if (seconds < 0) {
        return QString();
    }
    if (seconds < 60) {
        return QStringLiteral("%1s").arg(seconds);
    }
    if (seconds < 3600) {
        return QStringLiteral("%1m %2s").arg(seconds / 60).arg(seconds % 60);
    }
    return QStringLiteral("%1h %2m").arg(seconds / 3600).arg((seconds % 3600) / 60);
}

}
