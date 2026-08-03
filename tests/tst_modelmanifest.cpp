#include "app/ByteFormat.h"
#include "models/ModelManifest.h"

#include <QTest>

using namespace vesper;

class TestModelManifest : public QObject {
    Q_OBJECT

private slots:
    void parsesMultiFileEntry();
    void derivesTotalSizeWhenAbsent();
    void rejectsShortChecksum();
    void rejectsUnknownRole();
    void rejectsMalformedJson();
    void mapsRolesToEngineFlags();
    void formatsSizesTheWayTheUiShowsThem();
};

void TestModelManifest::parsesMultiFileEntry() {
    const QByteArray json = R"({
        "models": [{
            "id": "flux",
            "name": "Flux",
            "family": "Flux",
            "total_size_bytes": 3000,
            "files": [
                {"role":"diffusion_model","filename":"a.gguf","url":"https://x/a","sha256":")"
                              + QByteArray(64, 'a') + R"(","size_bytes":1000},
                {"role":"t5xxl","filename":"b.safetensors","url":"https://x/b","sha256":")"
                              + QByteArray(64, 'b') + R"(","size_bytes":2000}
            ]
        }]
    })";

    const auto result = parseManifest(json);
    QVERIFY2(result.ok(), qPrintable(result.error));
    QCOMPARE(result.models.size(), 1);
    QCOMPARE(result.models.first().files.size(), 2);
    QCOMPARE(result.models.first().files.first().role, FileRole::DiffusionModel);
    QVERIFY(result.models.first().fileForRole(FileRole::T5xxl) != nullptr);
    QVERIFY(result.models.first().fileForRole(FileRole::Vae) == nullptr);
}

void TestModelManifest::derivesTotalSizeWhenAbsent() {
    const QByteArray json = R"({
        "models": [{
            "id":"m","name":"M","family":"F",
            "files":[{"role":"checkpoint","filename":"a","url":"https://x/a","sha256":")"
                              + QByteArray(64, 'c') + R"(","size_bytes":4265146304}]
        }]
    })";

    const auto result = parseManifest(json);
    QVERIFY(result.ok());
    QCOMPARE(result.models.first().totalSizeBytes, 4265146304LL);
}

void TestModelManifest::rejectsShortChecksum() {
    const QByteArray json = R"({
        "models":[{"id":"m","name":"M","family":"F",
        "files":[{"role":"checkpoint","filename":"a","url":"https://x/a","sha256":"abc","size_bytes":1}]}]
    })";

    QVERIFY(!parseManifest(json).ok());
}

void TestModelManifest::rejectsUnknownRole() {
    const QByteArray json = R"({
        "models":[{"id":"m","name":"M","family":"F",
        "files":[{"role":"mystery","filename":"a","url":"https://x/a","sha256":")"
                              + QByteArray(64, 'd') + R"(","size_bytes":1}]}]
    })";

    QVERIFY(!parseManifest(json).ok());
}

void TestModelManifest::rejectsMalformedJson() {
    QVERIFY(!parseManifest(QByteArrayLiteral("{ not json")).ok());
    QVERIFY(!parseManifest(QByteArrayLiteral("{\"models\":[]}")).ok());
}

void TestModelManifest::mapsRolesToEngineFlags() {
    QCOMPARE(fileRoleToCliFlag(FileRole::Checkpoint), QStringLiteral("--model"));
    QCOMPARE(fileRoleToCliFlag(FileRole::DiffusionModel), QStringLiteral("--diffusion-model"));
    QCOMPARE(fileRoleToCliFlag(FileRole::ClipL), QStringLiteral("--clip_l"));
    QCOMPARE(fileRoleToCliFlag(FileRole::T5xxl), QStringLiteral("--t5xxl"));
    QCOMPARE(fileRoleToCliFlag(FileRole::Vae), QStringLiteral("--vae"));
    QCOMPARE(fileRoleFromString(QStringLiteral("clip_g")), FileRole::ClipG);
}

void TestModelManifest::formatsSizesTheWayTheUiShowsThem() {
    QCOMPARE(ByteFormat::humanize(4265146304LL), QStringLiteral("4.27 GB"));
    QCOMPARE(ByteFormat::humanize(6938078334LL), QStringLiteral("6.94 GB"));
    QCOMPARE(ByteFormat::humanize(12259327156LL), QStringLiteral("12.26 GB"));
}

QTEST_MAIN(TestModelManifest)
#include "tst_modelmanifest.moc"
