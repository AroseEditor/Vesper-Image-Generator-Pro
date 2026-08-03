#include "inference/InferenceBridge.h"

#include "app/AppPaths.h"
#include "gallery/GenerationMetadata.h"
#include "inference/ArgumentBuilder.h"
#include "inference/SamplerCatalog.h"
#include "inference/SdCliLocator.h"
#include "models/ModelCatalog.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QUrl>

namespace vesper {

namespace {

QString normalizeLocalPath(const QString& value) {
    if (value.startsWith(QLatin1String("file:"))) {
        return QUrl(value).toLocalFile();
    }
    return value;
}

qint64 resolveSeed(qint64 requested) {
    if (requested >= 0) {
        return requested;
    }
    return static_cast<qint64>(QRandomGenerator::global()->bounded(1, 2147483647));
}

}

InferenceBridge::InferenceBridge(QObject* parent) : QObject(parent) {
    m_enginePath = locateSdCli();
    if (m_enginePath.isEmpty()) {
        m_status = QStringLiteral("Engine not found. Rebuild so sd-cli sits next to the app.");
    } else {
        m_status = QStringLiteral("Ready");
    }
}

InferenceBridge::~InferenceBridge() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

bool InferenceBridge::isRunning() const {
    return m_process && m_process->state() != QProcess::NotRunning;
}

bool InferenceBridge::engineAvailable() const {
    return !m_enginePath.isEmpty();
}

QString InferenceBridge::enginePath() const {
    return m_enginePath;
}

int InferenceBridge::currentStep() const {
    return m_currentStep;
}

int InferenceBridge::totalSteps() const {
    return m_totalSteps;
}

double InferenceBridge::progress() const {
    return m_totalSteps > 0 ? static_cast<double>(m_currentStep) / static_cast<double>(m_totalSteps)
                            : 0.0;
}

QString InferenceBridge::statusText() const {
    return m_status;
}

QStringList InferenceBridge::log() const {
    return m_log;
}

QStringList InferenceBridge::samplers() {
    return availableSamplers();
}

QStringList InferenceBridge::schedulers() {
    return availableSchedulers();
}

void InferenceBridge::clearLog() {
    m_log.clear();
    emit logChanged();
}

void InferenceBridge::appendLog(const QString& line) {
    if (line.isEmpty()) {
        return;
    }
    m_log.append(line);
    while (m_log.size() > 400) {
        m_log.removeFirst();
    }
    emit logChanged();
}

void InferenceBridge::setStatus(const QString& text) {
    if (m_status == text) {
        return;
    }
    m_status = text;
    emit statusTextChanged();
}

void InferenceBridge::generate(const QVariantMap& options) {
    if (isRunning()) {
        emit failed(QStringLiteral("A generation is already running"));
        return;
    }
    if (m_enginePath.isEmpty()) {
        emit failed(QStringLiteral("The sd-cli engine was not found next to the application"));
        return;
    }

    auto* catalog = new ModelCatalog(this);
    const QString modelId = options.value(QStringLiteral("modelId")).toString();
    const ModelEntry* entry = catalog->entryById(modelId);
    if (!entry) {
        catalog->deleteLater();
        emit failed(QStringLiteral("Select an installed model first"));
        return;
    }
    if (!catalog->isInstalled(modelId)) {
        catalog->deleteLater();
        emit failed(QStringLiteral("Model '%1' is not installed").arg(entry->name));
        return;
    }
    const ModelEntry resolved = *entry;
    catalog->deleteLater();

    GenerationRequest request;
    request.mode = options.value(QStringLiteral("mode")).toString() == QLatin1String("img2img")
                       ? GenerationMode::ImageToImage
                       : GenerationMode::TextToImage;
    request.modelId = modelId;
    request.prompt = options.value(QStringLiteral("prompt")).toString();
    request.negativePrompt = options.value(QStringLiteral("negativePrompt")).toString();
    request.initImagePath = normalizeLocalPath(options.value(QStringLiteral("initImage")).toString());
    request.strength = options.value(QStringLiteral("strength"), 0.75).toDouble();
    request.steps = options.value(QStringLiteral("steps"), 20).toInt();
    request.cfgScale = options.value(QStringLiteral("cfgScale"), 7.0).toDouble();
    request.sampler = options.value(QStringLiteral("sampler"), QStringLiteral("euler_a")).toString();
    request.scheduler = options.value(QStringLiteral("scheduler")).toString();
    request.width = options.value(QStringLiteral("width"), 512).toInt();
    request.height = options.value(QStringLiteral("height"), 512).toInt();
    request.seed = resolveSeed(options.value(QStringLiteral("seed"), -1).toLongLong());
    request.batchCount = options.value(QStringLiteral("batchCount"), 1).toInt();
    request.threads = options.value(QStringLiteral("threads"), 0).toInt();
    request.backend = options.value(QStringLiteral("backend")).toString();
    request.offloadToCpu = options.value(QStringLiteral("offloadToCpu"), false).toBool();
    request.flashAttention = options.value(QStringLiteral("flashAttention"), false).toBool();

    m_outputDirectory = AppPaths::galleryDirectory();
    QDir().mkpath(m_outputDirectory);
    m_outputStem = QStringLiteral("vesper-%1-%2")
                       .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")))
                       .arg(request.seed);
    request.outputPath = QDir(m_outputDirectory).filePath(m_outputStem + QStringLiteral(".png"));

    const ArgumentBuildResult built = buildArguments(request, resolved, ModelCatalog::modelsDirectory());
    if (!built.ok()) {
        emit failed(built.error);
        return;
    }

    m_request = request;
    m_parser = ProgressParser();
    m_currentStep = 0;
    m_totalSteps = request.steps;
    m_cancelling = false;
    emit progressChanged();

    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, &QProcess::readyReadStandardOutput, this, &InferenceBridge::handleStdout);
        connect(m_process, &QProcess::readyReadStandardError, this, &InferenceBridge::handleStderr);
        connect(m_process, &QProcess::finished, this, &InferenceBridge::handleFinished);
    }

    appendLog(QStringLiteral("Running %1").arg(QFileInfo(m_enginePath).fileName()));
    setStatus(QStringLiteral("Loading model"));
    m_timer.start();
    m_process->setWorkingDirectory(m_outputDirectory);
    m_process->start(m_enginePath, built.arguments);
    emit runningChanged();
}

void InferenceBridge::handleStdout() {
    m_parser.append(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
    for (const QString& line : m_parser.takeLines()) {
        if (const auto progress = parseProgressLine(line)) {
            m_currentStep = progress->step;
            m_totalSteps = progress->totalSteps;
            emit progressChanged();
            setStatus(QStringLiteral("Sampling %1 of %2 at %3 s/step")
                          .arg(progress->step)
                          .arg(progress->totalSteps)
                          .arg(progress->secondsPerStep, 0, 'f', 2));
        } else {
            appendLog(line);
        }
    }
}

void InferenceBridge::handleStderr() {
    ProgressParser parser;
    parser.append(QString::fromLocal8Bit(m_process->readAllStandardError()));
    for (const QString& line : parser.takeLines()) {
        appendLog(line);
    }
}

QStringList InferenceBridge::collectOutputs() const {
    const QDir dir(m_outputDirectory);
    QStringList produced;

    const QString primary = dir.filePath(m_outputStem + QStringLiteral(".png"));
    if (QFileInfo::exists(primary)) {
        produced.append(primary);
    }

    const QStringList extras =
        dir.entryList({m_outputStem + QStringLiteral("_*.png")}, QDir::Files, QDir::Name);
    for (const QString& name : extras) {
        produced.append(dir.filePath(name));
    }
    return produced;
}

void InferenceBridge::handleFinished(int exitCode, QProcess::ExitStatus status) {
    emit runningChanged();

    if (m_cancelling) {
        setStatus(QStringLiteral("Cancelled"));
        emit cancelled();
        return;
    }

    if (status == QProcess::CrashExit) {
        setStatus(QStringLiteral("Engine crashed"));
        emit failed(QStringLiteral("The engine stopped unexpectedly"));
        return;
    }
    if (exitCode != 0) {
        setStatus(QStringLiteral("Failed"));
        emit failed(QStringLiteral("Engine exited with code %1. Check the log for details.").arg(exitCode));
        return;
    }

    const QStringList produced = collectOutputs();
    if (produced.isEmpty()) {
        setStatus(QStringLiteral("Failed"));
        emit failed(QStringLiteral("The engine reported success but wrote no image"));
        return;
    }

    const qint64 elapsedMs = m_timer.elapsed();
    QString firstMetadata;
    for (const QString& imagePath : produced) {
        GenerationMetadata metadata = GenerationMetadata::fromRequest(m_request);
        metadata.imagePath = imagePath;
        metadata.elapsedMs = elapsedMs;
        const QString metadataPath = GenerationMetadata::sidecarPathFor(imagePath);
        metadata.save(metadataPath);
        if (firstMetadata.isEmpty()) {
            firstMetadata = metadataPath;
        }
    }

    setStatus(QStringLiteral("Done in %1s").arg(elapsedMs / 1000.0, 0, 'f', 1));
    m_currentStep = m_totalSteps;
    emit progressChanged();
    emit succeeded(produced.first(), firstMetadata);
}

void InferenceBridge::cancel() {
    if (!isRunning()) {
        return;
    }
    m_cancelling = true;
    setStatus(QStringLiteral("Cancelling"));
    m_process->kill();
}

}
