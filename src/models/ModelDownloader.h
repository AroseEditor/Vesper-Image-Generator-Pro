#pragma once

#include "models/ModelManifest.h"

#include <QElapsedTimer>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QQueue>

namespace vesper {

class FileHasher;

class ModelDownloader : public QObject {
    Q_OBJECT

public:
    explicit ModelDownloader(QObject* parent = nullptr);
    ~ModelDownloader() override;

    void start(const ModelEntry& entry, const QString& destinationDirectory);
    void cancel();
    bool isBusy() const;
    QString currentModelId() const;

    static QString partialPathFor(const QString& finalPath);

signals:
    void started(const QString& modelId);
    void progress(const QString& modelId, qint64 receivedBytes, qint64 totalBytes,
                  double bytesPerSecond, const QString& currentFile);
    void verifying(const QString& modelId, const QString& filename);
    void verifyProgress(const QString& modelId, qint64 hashed, qint64 total);
    void finished(const QString& modelId);
    void failed(const QString& modelId, const QString& reason);
    void cancelled(const QString& modelId);

private:
    void startNextFile();
    void handleReadyRead();
    void handleReplyFinished();
    void beginVerification();
    void failCurrent(const QString& reason);
    void cleanupReply();
    qint64 completedBytesBefore() const;

    QNetworkAccessManager m_network;
    QPointer<QNetworkReply> m_reply;
    FileHasher* m_hasher = nullptr;

    ModelEntry m_entry;
    QString m_destination;
    QQueue<ModelFile> m_pending;
    ModelFile m_current;
    int m_currentIndex = -1;

    QFile m_output;
    qint64 m_resumeOffset = 0;
    qint64 m_receivedThisFile = 0;
    QElapsedTimer m_rateTimer;
    bool m_busy = false;
    bool m_cancelling = false;
};

}
