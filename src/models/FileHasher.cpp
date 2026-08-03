#include "models/FileHasher.h"

#include <QCryptographicHash>
#include <QFile>
#include <QtConcurrent/QtConcurrent>

namespace vesper {

namespace {
constexpr qint64 kChunkBytes = 8 * 1024 * 1024;
}

FileHasher::FileHasher(QObject* parent)
    : QObject(parent), m_cancelled(std::make_shared<std::atomic_bool>(false)) {}

FileHasher::~FileHasher() {
    cancel();
}

bool FileHasher::isRunning() const {
    return m_running.load();
}

void FileHasher::cancel() {
    m_cancelled->store(true);
}

QString FileHasher::hashFile(const QString& filePath, const std::atomic_bool& cancelled,
                             const std::function<void(qint64, qint64)>& onProgress) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    const qint64 total = file.size();
    qint64 done = 0;
    QByteArray buffer;
    buffer.resize(kChunkBytes);

    while (!file.atEnd()) {
        if (cancelled.load()) {
            return QString();
        }
        const qint64 read = file.read(buffer.data(), kChunkBytes);
        if (read < 0) {
            return QString();
        }
        hash.addData(QByteArrayView(buffer.constData(), static_cast<qsizetype>(read)));
        done += read;
        if (onProgress) {
            onProgress(done, total);
        }
    }

    return QString::fromLatin1(hash.result().toHex());
}

void FileHasher::verify(const QString& filePath, const QString& expectedSha256) {
    if (m_running.load()) {
        emit failed(QStringLiteral("A verification is already running"));
        return;
    }

    m_cancelled = std::make_shared<std::atomic_bool>(false);
    m_running.store(true);

    auto cancelled = m_cancelled;
    const QString expected = expectedSha256.toLower();

    auto* watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, expected] {
        const QString actual = watcher->result();
        m_running.store(false);
        watcher->deleteLater();
        if (actual.isEmpty()) {
            emit failed(QStringLiteral("Could not read the file for verification"));
            return;
        }
        emit finished(actual == expected, actual);
    });

    watcher->setFuture(QtConcurrent::run([this, filePath, cancelled] {
        return hashFile(filePath, *cancelled, [this](qint64 done, qint64 total) {
            QMetaObject::invokeMethod(this, [this, done, total] { emit progress(done, total); },
                                      Qt::QueuedConnection);
        });
    }));
}

}
