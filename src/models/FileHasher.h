#pragma once

#include <QObject>
#include <QString>
#include <atomic>
#include <memory>

namespace vesper {

class FileHasher : public QObject {
    Q_OBJECT

public:
    explicit FileHasher(QObject* parent = nullptr);
    ~FileHasher() override;

    void verify(const QString& filePath, const QString& expectedSha256);
    void cancel();
    bool isRunning() const;

    static QString hashFile(const QString& filePath, const std::atomic_bool& cancelled,
                            const std::function<void(qint64, qint64)>& onProgress);

signals:
    void progress(qint64 bytesHashed, qint64 bytesTotal);
    void finished(bool matched, const QString& actualSha256);
    void failed(const QString& reason);

private:
    std::shared_ptr<std::atomic_bool> m_cancelled;
    std::atomic_bool m_running{false};
};

}
