#include "inference/ArgumentBuilder.h"

#include <QTest>

using namespace vesper;

namespace {

ModelEntry singleFileModel() {
    ModelEntry entry;
    entry.id = QStringLiteral("sd15-emaonly");
    entry.name = QStringLiteral("Stable Diffusion 1.5");
    ModelFile file;
    file.role = FileRole::Checkpoint;
    file.filename = QStringLiteral("v1-5-pruned-emaonly.safetensors");
    file.url = QStringLiteral("https://example.invalid/model.safetensors");
    file.sha256 = QString(64, QLatin1Char('a'));
    file.sizeBytes = 4265146304LL;
    entry.files.append(file);
    return entry;
}

ModelEntry fluxModel() {
    ModelEntry entry;
    entry.id = QStringLiteral("flux1-schnell-q4ks");
    entry.name = QStringLiteral("FLUX.1 schnell");

    const QVector<QPair<FileRole, QString>> parts{
        {FileRole::DiffusionModel, QStringLiteral("flux1-schnell-Q4_K_S.gguf")},
        {FileRole::ClipL, QStringLiteral("clip_l.safetensors")},
        {FileRole::T5xxl, QStringLiteral("t5xxl_fp8_e4m3fn.safetensors")},
        {FileRole::Vae, QStringLiteral("ae.safetensors")},
    };

    for (const auto& part : parts) {
        ModelFile file;
        file.role = part.first;
        file.filename = part.second;
        file.url = QStringLiteral("https://example.invalid/") + part.second;
        file.sha256 = QString(64, QLatin1Char('b'));
        file.sizeBytes = 1000;
        entry.files.append(file);
    }
    return entry;
}

GenerationRequest baseRequest() {
    GenerationRequest request;
    request.prompt = QStringLiteral("a quiet harbour at dawn");
    request.outputPath = QStringLiteral("/out/image.png");
    request.sampler = QStringLiteral("euler_a");
    request.steps = 20;
    request.cfgScale = 7.0;
    request.width = 512;
    request.height = 512;
    request.seed = 42;
    return request;
}

}

class TestArgumentBuilder : public QObject {
    Q_OBJECT

private slots:
    void singleFileUsesModelFlag();
    void fluxMapsEveryRoleToItsOwnFlag();
    void imageToImageAddsInitAndStrength();
    void textToImageOmitsInitImage();
    void seedIsPassedThroughVerbatim();
    void emptyPromptIsRejected();
    void unknownSamplerIsRejected();
    void imageToImageWithoutReferenceIsRejected();
    void optionalFlagsAppearOnlyWhenEnabled();
};

void TestArgumentBuilder::singleFileUsesModelFlag() {
    const auto result = buildArguments(baseRequest(), singleFileModel(), QStringLiteral("/models"));
    QVERIFY(result.ok());
    QVERIFY(result.arguments.contains(QStringLiteral("--model")));
    QVERIFY(!result.arguments.contains(QStringLiteral("--diffusion-model")));

    const int index = result.arguments.indexOf(QStringLiteral("--model"));
    QVERIFY(result.arguments.at(index + 1).contains(QStringLiteral("v1-5-pruned-emaonly.safetensors")));
}

void TestArgumentBuilder::fluxMapsEveryRoleToItsOwnFlag() {
    const auto result = buildArguments(baseRequest(), fluxModel(), QStringLiteral("/models"));
    QVERIFY(result.ok());

    for (const QString& flag : {QStringLiteral("--diffusion-model"), QStringLiteral("--clip_l"),
                                QStringLiteral("--t5xxl"), QStringLiteral("--vae")}) {
        QVERIFY2(result.arguments.contains(flag), qPrintable(QStringLiteral("missing ") + flag));
    }
    QVERIFY(!result.arguments.contains(QStringLiteral("--model")));
}

void TestArgumentBuilder::imageToImageAddsInitAndStrength() {
    GenerationRequest request = baseRequest();
    request.mode = GenerationMode::ImageToImage;
    request.initImagePath = QStringLiteral("/in/reference.png");
    request.strength = 0.42;

    const auto result = buildArguments(request, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(result.ok());
    QVERIFY(result.arguments.contains(QStringLiteral("--init-img")));

    const int index = result.arguments.indexOf(QStringLiteral("--strength"));
    QVERIFY(index >= 0);
    QCOMPARE(result.arguments.at(index + 1), QStringLiteral("0.42"));

    const int modeIndex = result.arguments.indexOf(QStringLiteral("--mode"));
    QCOMPARE(result.arguments.at(modeIndex + 1), QStringLiteral("img2img"));
}

void TestArgumentBuilder::textToImageOmitsInitImage() {
    const auto result = buildArguments(baseRequest(), singleFileModel(), QStringLiteral("/models"));
    QVERIFY(result.ok());
    QVERIFY(!result.arguments.contains(QStringLiteral("--init-img")));
    QVERIFY(!result.arguments.contains(QStringLiteral("--strength")));

    const int modeIndex = result.arguments.indexOf(QStringLiteral("--mode"));
    QCOMPARE(result.arguments.at(modeIndex + 1), QStringLiteral("txt2img"));
}

void TestArgumentBuilder::seedIsPassedThroughVerbatim() {
    GenerationRequest request = baseRequest();
    request.seed = 1234567;

    const auto result = buildArguments(request, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(result.ok());

    const int index = result.arguments.indexOf(QStringLiteral("--seed"));
    QCOMPARE(result.arguments.at(index + 1), QStringLiteral("1234567"));
}

void TestArgumentBuilder::emptyPromptIsRejected() {
    GenerationRequest request = baseRequest();
    request.prompt = QStringLiteral("   ");

    const auto result = buildArguments(request, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(!result.ok());
}

void TestArgumentBuilder::unknownSamplerIsRejected() {
    GenerationRequest request = baseRequest();
    request.sampler = QStringLiteral("not_a_sampler");

    const auto result = buildArguments(request, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(!result.ok());
}

void TestArgumentBuilder::imageToImageWithoutReferenceIsRejected() {
    GenerationRequest request = baseRequest();
    request.mode = GenerationMode::ImageToImage;

    const auto result = buildArguments(request, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(!result.ok());
}

void TestArgumentBuilder::optionalFlagsAppearOnlyWhenEnabled() {
    GenerationRequest plain = baseRequest();
    const auto without = buildArguments(plain, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(!without.arguments.contains(QStringLiteral("--diffusion-fa")));
    QVERIFY(!without.arguments.contains(QStringLiteral("--offload-to-cpu")));
    QVERIFY(!without.arguments.contains(QStringLiteral("--threads")));

    GenerationRequest tuned = baseRequest();
    tuned.flashAttention = true;
    tuned.offloadToCpu = true;
    tuned.threads = 8;

    const auto with = buildArguments(tuned, singleFileModel(), QStringLiteral("/models"));
    QVERIFY(with.arguments.contains(QStringLiteral("--diffusion-fa")));
    QVERIFY(with.arguments.contains(QStringLiteral("--offload-to-cpu")));

    const int index = with.arguments.indexOf(QStringLiteral("--threads"));
    QCOMPARE(with.arguments.at(index + 1), QStringLiteral("8"));
}

QTEST_MAIN(TestArgumentBuilder)
#include "tst_argumentbuilder.moc"
