#include "models/FileHasher.h"
#include "models/ModelDownloader.h"

#include <QCryptographicHash>
#include <QFile>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
#include <atomic>

using namespace vesper;

namespace {

QByteArray payload() {
    QByteArray data;
    data.reserve(512 * 1024);
    for (int i = 0; i < 512 * 1024; ++i) {
        data.append(static_cast<char>((i * 31 + 7) % 251));
    }
    return data;
}

QString sha256Of(const QByteArray& data) {
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

}

class RangeServer : public QTcpServer {
    Q_OBJECT

public:
    explicit RangeServer(QByteArray body, QObject* parent = nullptr)
        : QTcpServer(parent), m_body(std::move(body)) {}

    bool supportsRanges = true;
    int lastRequestedOffset = -1;
    QByteArray lastRequest;

protected:
    void incomingConnection(qintptr handle) override {
        auto* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(handle);

        auto buffer = std::make_shared<QByteArray>();
        auto answered = std::make_shared<bool>(false);

        connect(socket, &QTcpSocket::readyRead, this, [this, socket, buffer, answered] {
            if (*answered) {
                socket->readAll();
                return;
            }
            buffer->append(socket->readAll());
            const QByteArray request = *buffer;
            if (!request.contains("\r\n\r\n")) {
                return;
            }
            *answered = true;
            lastRequest = request;

            qint64 offset = 0;
            const QByteArray marker = "range: bytes=";
            const int rangeAt = request.toLower().indexOf(marker);
            if (rangeAt >= 0 && supportsRanges) {
                const int start = rangeAt + marker.size();
                const int dash = request.indexOf('-', start);
                offset = request.mid(start, dash - start).toLongLong();
            }
            lastRequestedOffset = static_cast<int>(offset);

            const QByteArray slice = m_body.mid(static_cast<int>(offset));
            QByteArray header;
            if (offset > 0 && supportsRanges) {
                header = "HTTP/1.1 206 Partial Content\r\n";
                header += "Content-Range: bytes " + QByteArray::number(offset) + "-" +
                          QByteArray::number(m_body.size() - 1) + "/" +
                          QByteArray::number(m_body.size()) + "\r\n";
            } else {
                header = "HTTP/1.1 200 OK\r\n";
            }
            header += "Accept-Ranges: bytes\r\n";
            header += "Content-Length: " + QByteArray::number(slice.size()) + "\r\n";
            header += "Connection: close\r\n\r\n";

            socket->write(header);
            socket->write(slice);
            socket->flush();
            socket->disconnectFromHost();
        });

        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }

private:
    QByteArray m_body;
};

class TestDownloader : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void downloadsAndVerifiesChecksum();
    void resumesFromPartialFile();
    void restartsWhenServerIgnoresRange();
    void rejectsChecksumMismatch();
    void hashesFileContentCorrectly();

private:
    ModelEntry entryFor(const RangeServer& server, const QString& sha) const;

    QByteArray m_body;
    QString m_sha;
};

void TestDownloader::initTestCase() {
    m_body = payload();
    m_sha = sha256Of(m_body);
}

ModelEntry TestDownloader::entryFor(const RangeServer& server, const QString& sha) const {
    ModelEntry entry;
    entry.id = QStringLiteral("test-model");
    entry.name = QStringLiteral("Test Model");
    entry.totalSizeBytes = m_body.size();

    ModelFile file;
    file.role = FileRole::Checkpoint;
    file.filename = QStringLiteral("payload.bin");
    file.url = QStringLiteral("http://127.0.0.1:%1/payload.bin").arg(server.serverPort());
    file.sha256 = sha;
    file.sizeBytes = m_body.size();
    entry.files.append(file);
    return entry;
}

void TestDownloader::downloadsAndVerifiesChecksum() {
    RangeServer server(m_body);
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QTemporaryDir dir;
    ModelDownloader downloader;
    QSignalSpy finished(&downloader, &ModelDownloader::finished);
    QSignalSpy failed(&downloader, &ModelDownloader::failed);

    downloader.start(entryFor(server, m_sha), dir.path());
    QVERIFY(finished.wait(30000));
    QCOMPARE(failed.count(), 0);

    QFile written(QDir(dir.path()).filePath(QStringLiteral("payload.bin")));
    QVERIFY(written.exists());
    QCOMPARE(written.size(), qint64(m_body.size()));
}

void TestDownloader::resumesFromPartialFile() {
    RangeServer server(m_body);
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QTemporaryDir dir;
    const QString finalPath = QDir(dir.path()).filePath(QStringLiteral("payload.bin"));
    const qint64 alreadyHave = 200000;

    QFile partial(ModelDownloader::partialPathFor(finalPath));
    QVERIFY(partial.open(QIODevice::WriteOnly));
    partial.write(m_body.left(static_cast<int>(alreadyHave)));
    partial.close();

    ModelDownloader downloader;
    QSignalSpy finished(&downloader, &ModelDownloader::finished);
    QSignalSpy failed(&downloader, &ModelDownloader::failed);

    downloader.start(entryFor(server, m_sha), dir.path());
    QVERIFY(finished.wait(30000));
    QCOMPARE(failed.count(), 0);
    QCOMPARE(server.lastRequestedOffset, int(alreadyHave));

    QFile written(finalPath);
    QVERIFY(written.open(QIODevice::ReadOnly));
    QCOMPARE(written.readAll(), m_body);
}

void TestDownloader::restartsWhenServerIgnoresRange() {
    RangeServer server(m_body);
    server.supportsRanges = false;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QTemporaryDir dir;
    const QString finalPath = QDir(dir.path()).filePath(QStringLiteral("payload.bin"));

    QFile partial(ModelDownloader::partialPathFor(finalPath));
    QVERIFY(partial.open(QIODevice::WriteOnly));
    partial.write(QByteArray(150000, 'x'));
    partial.close();

    ModelDownloader downloader;
    QSignalSpy finished(&downloader, &ModelDownloader::finished);

    downloader.start(entryFor(server, m_sha), dir.path());
    QVERIFY(finished.wait(30000));

    QFile written(finalPath);
    QVERIFY(written.open(QIODevice::ReadOnly));
    QCOMPARE(written.readAll(), m_body);
}

void TestDownloader::rejectsChecksumMismatch() {
    RangeServer server(m_body);
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QTemporaryDir dir;
    ModelDownloader downloader;
    QSignalSpy failed(&downloader, &ModelDownloader::failed);

    downloader.start(entryFor(server, QString(64, QLatin1Char('f'))), dir.path());
    QVERIFY(failed.wait(30000));

    QVERIFY(!QFile::exists(QDir(dir.path()).filePath(QStringLiteral("payload.bin"))));
    QVERIFY(failed.first().at(1).toString().contains(QStringLiteral("Checksum mismatch")));
}

void TestDownloader::hashesFileContentCorrectly() {
    QTemporaryDir dir;
    const QString path = QDir(dir.path()).filePath(QStringLiteral("blob.bin"));

    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(m_body);
    file.close();

    std::atomic_bool cancelled{false};
    QCOMPARE(FileHasher::hashFile(path, cancelled, nullptr), m_sha);
}

QTEST_MAIN(TestDownloader)
#include "tst_downloader.moc"
