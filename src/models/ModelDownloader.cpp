#include "models/ModelDownloader.h"

#include "models/DiskSpace.h"
#include "models/FileHasher.h"

#include <QDir>
#include <QFileInfo>
#include <QNetworkRequest>

namespace vesper {

namespace {
constexpr int kHttpPartialContent = 206;
constexpr int kHttpOk = 200;
}

ModelDownloader::ModelDownloader(QObject* parent) : QObject(parent) {
    m_hasher = new FileHasher(this);
}

ModelDownloader::~ModelDownloader() {
    cancel();
}

bool ModelDownloader::isBusy() const {
    return m_busy;
}

QString ModelDownloader::currentModelId() const {
    return m_busy ? m_entry.id : QString();
}

QString ModelDownloader::partialPathFor(const QString& finalPath) {
    return finalPath + QStringLiteral(".part");
}

void ModelDownloader::start(const ModelEntry& entry, const QString& destinationDirectory) {
    if (m_busy) {
        emit failed(entry.id, QStringLiteral("Another download is already running"));
        return;
    }

    if (!QDir().mkpath(destinationDirectory)) {
        emit failed(entry.id, QStringLiteral("Could not create the models directory"));
        return;
    }

    qint64 outstanding = 0;
    for (const ModelFile& file : entry.files) {
        const QString finalPath = QDir(destinationDirectory).filePath(file.filename);
        if (QFileInfo(finalPath).size() == file.sizeBytes && file.sizeBytes > 0) {
            continue;
        }
        outstanding += file.sizeBytes - QFileInfo(partialPathFor(finalPath)).size();
    }

    const DiskSpaceCheck space = checkDiskSpace(destinationDirectory, outstanding);
    if (!space.sufficient) {
        emit failed(entry.id, space.shortfallDescription());
        return;
    }

    m_entry = entry;
    m_destination = destinationDirectory;
    m_pending.clear();
    for (const ModelFile& file : entry.files) {
        m_pending.enqueue(file);
    }
    m_currentIndex = -1;
    m_busy = true;
    m_cancelling = false;

    emit started(entry.id);
    startNextFile();
}

qint64 ModelDownloader::completedBytesBefore() const {
    qint64 total = 0;
    for (int i = 0; i < m_currentIndex && i < m_entry.files.size(); ++i) {
        total += m_entry.files.at(i).sizeBytes;
    }
    return total;
}

void ModelDownloader::startNextFile() {
    if (m_pending.isEmpty()) {
        m_busy = false;
        emit finished(m_entry.id);
        return;
    }

    m_current = m_pending.dequeue();
    ++m_currentIndex;

    const QString finalPath = QDir(m_destination).filePath(m_current.filename);
    if (QFileInfo(finalPath).size() == m_current.sizeBytes && m_current.sizeBytes > 0) {
        startNextFile();
        return;
    }

    const QString partialPath = partialPathFor(finalPath);
    m_resumeOffset = QFileInfo(partialPath).size();
    if (m_resumeOffset >= m_current.sizeBytes && m_current.sizeBytes > 0) {
        m_resumeOffset = 0;
        QFile::remove(partialPath);
    }

    m_output.setFileName(partialPath);
    if (!m_output.open(m_resumeOffset > 0 ? QIODevice::Append : QIODevice::WriteOnly)) {
        failCurrent(QStringLiteral("Could not write to %1").arg(partialPath));
        return;
    }

    QNetworkRequest request{QUrl(m_current.url)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "VesperImageGenerator");
    if (m_resumeOffset > 0) {
        request.setRawHeader("Range",
                             QByteArrayLiteral("bytes=") + QByteArray::number(m_resumeOffset) + "-");
    }

    m_receivedThisFile = 0;
    m_rateTimer.start();

    m_reply = m_network.get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, &ModelDownloader::handleReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &ModelDownloader::handleReplyFinished);
}

void ModelDownloader::handleReadyRead() {
    if (!m_reply) {
        return;
    }

    if (m_resumeOffset > 0 && m_receivedThisFile == 0) {
        const int status =
            m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == kHttpOk) {
            m_output.close();
            m_output.open(QIODevice::WriteOnly | QIODevice::Truncate);
            m_resumeOffset = 0;
        } else if (status != kHttpPartialContent) {
            failCurrent(QStringLiteral("Server rejected the resume request (HTTP %1)").arg(status));
            return;
        }
    }

    const QByteArray chunk = m_reply->readAll();
    if (m_output.write(chunk) != chunk.size()) {
        failCurrent(QStringLiteral("Ran out of disk space while writing %1").arg(m_current.filename));
        return;
    }
    m_receivedThisFile += chunk.size();

    const qint64 fileDone = m_resumeOffset + m_receivedThisFile;
    const qint64 overallDone = completedBytesBefore() + fileDone;
    const qint64 elapsedMs = m_rateTimer.elapsed();
    const double rate =
        elapsedMs > 0 ? static_cast<double>(m_receivedThisFile) * 1000.0 / static_cast<double>(elapsedMs)
                      : 0.0;

    emit progress(m_entry.id, overallDone, m_entry.totalSizeBytes, rate, m_current.filename);
}

void ModelDownloader::handleReplyFinished() {
    if (!m_reply) {
        return;
    }

    const QNetworkReply::NetworkError error = m_reply->error();
    const QString errorText = m_reply->errorString();
    cleanupReply();
    m_output.flush();
    m_output.close();

    if (m_cancelling) {
        m_busy = false;
        emit cancelled(m_entry.id);
        return;
    }

    if (error != QNetworkReply::NoError) {
        failCurrent(errorText);
        return;
    }

    beginVerification();
}

void ModelDownloader::beginVerification() {
    const QString finalPath = QDir(m_destination).filePath(m_current.filename);
    const QString partialPath = partialPathFor(finalPath);

    emit verifying(m_entry.id, m_current.filename);

    disconnect(m_hasher, nullptr, this, nullptr);
    connect(m_hasher, &FileHasher::progress, this, [this](qint64 done, qint64 total) {
        emit verifyProgress(m_entry.id, done, total);
    });
    connect(m_hasher, &FileHasher::failed, this,
            [this](const QString& reason) { failCurrent(reason); });
    connect(m_hasher, &FileHasher::finished, this,
            [this, finalPath, partialPath](bool matched, const QString& actual) {
                if (!matched) {
                    QFile::remove(partialPath);
                    failCurrent(QStringLiteral("Checksum mismatch for %1. Expected %2, got %3.")
                                    .arg(m_current.filename, m_current.sha256, actual));
                    return;
                }
                QFile::remove(finalPath);
                if (!QFile::rename(partialPath, finalPath)) {
                    failCurrent(QStringLiteral("Could not move %1 into place").arg(m_current.filename));
                    return;
                }
                startNextFile();
            });

    m_hasher->verify(partialPath, m_current.sha256);
}

void ModelDownloader::failCurrent(const QString& reason) {
    cleanupReply();
    if (m_output.isOpen()) {
        m_output.close();
    }
    m_busy = false;
    emit failed(m_entry.id, reason);
}

void ModelDownloader::cleanupReply() {
    if (m_reply) {
        m_reply->disconnect(this);
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

void ModelDownloader::cancel() {
    if (!m_busy) {
        return;
    }
    m_cancelling = true;
    m_hasher->cancel();
    if (m_reply) {
        m_reply->abort();
    } else {
        m_busy = false;
        emit cancelled(m_entry.id);
    }
}

}
