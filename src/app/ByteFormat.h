#pragma once

#include <QString>
#include <QObject>
#include <qqmlintegration.h>

namespace vesper {

class ByteFormat : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit ByteFormat(QObject* parent = nullptr);

    Q_INVOKABLE static QString humanize(qint64 bytes);
    Q_INVOKABLE static QString humanizeRate(double bytesPerSecond);
    Q_INVOKABLE static QString humanizeDuration(qint64 seconds);
};

}
