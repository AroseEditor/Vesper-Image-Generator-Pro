#include "models/ModelCatalog.h"

#include "app/AppPaths.h"
#include "app/ByteFormat.h"
#include "models/ModelDownloader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace vesper {

namespace {

QString stateLabel(ModelCatalog::State state) {
    switch (state) {
    case ModelCatalog::State::NotInstalled: return QStringLiteral("Not installed");
    case ModelCatalog::State::Downloading: return QStringLiteral("Downloading");
    case ModelCatalog::State::Verifying: return QStringLiteral("Verifying");
    case ModelCatalog::State::Installed: return QStringLiteral("Installed");
    case ModelCatalog::State::Failed: return QStringLiteral("Failed");
    }
    return QString();
}

}

ModelCatalog::ModelCatalog(QObject* parent) : QAbstractListModel(parent) {
    m_downloader = new ModelDownloader(this);

    connect(m_downloader, &ModelDownloader::started, this, [this](const QString& id) {
        m_activeDownloadId = id;
        m_runtime[id] = {State::Downloading, 0.0, QStringLiteral("Starting")};
        notifyRow(id);
        emit activeDownloadIdChanged();
    });

    connect(m_downloader, &ModelDownloader::progress, this,
            [this](const QString& id, qint64 received, qint64 total, double rate,
                   const QString& file) {
                RuntimeState& runtime = m_runtime[id];
                runtime.state = State::Downloading;
                runtime.progress = total > 0 ? static_cast<double>(received) / static_cast<double>(total)
                                             : 0.0;
                runtime.detail = QStringLiteral("%1 of %2 . %3 . %4")
                                     .arg(ByteFormat::humanize(received),
                                          ByteFormat::humanize(total),
                                          ByteFormat::humanizeRate(rate), file);
                notifyRow(id);
            });

    connect(m_downloader, &ModelDownloader::verifying, this,
            [this](const QString& id, const QString& file) {
                RuntimeState& runtime = m_runtime[id];
                runtime.state = State::Verifying;
                runtime.detail = QStringLiteral("Checking %1").arg(file);
                notifyRow(id);
            });

    connect(m_downloader, &ModelDownloader::verifyProgress, this,
            [this](const QString& id, qint64 done, qint64 total) {
                RuntimeState& runtime = m_runtime[id];
                runtime.progress = total > 0 ? static_cast<double>(done) / static_cast<double>(total)
                                             : 0.0;
                notifyRow(id);
            });

    connect(m_downloader, &ModelDownloader::finished, this, [this](const QString& id) {
        m_runtime[id] = {State::Installed, 1.0, QString()};
        m_activeDownloadId.clear();
        notifyRow(id);
        emit activeDownloadIdChanged();
        emit installedCountChanged();
        emit downloadFinished(id);
    });

    connect(m_downloader, &ModelDownloader::failed, this,
            [this](const QString& id, const QString& reason) {
                m_runtime[id] = {State::Failed, 0.0, reason};
                m_activeDownloadId.clear();
                notifyRow(id);
                emit activeDownloadIdChanged();
                emit downloadFailed(id, reason);
            });

    connect(m_downloader, &ModelDownloader::cancelled, this, [this](const QString& id) {
        m_runtime[id] = {State::NotInstalled, 0.0, QStringLiteral("Cancelled")};
        m_activeDownloadId.clear();
        notifyRow(id);
        emit activeDownloadIdChanged();
    });

    reload();
}

ModelCatalog::~ModelCatalog() = default;

QString ModelCatalog::modelsDirectory() {
    return AppPaths::modelsDirectory();
}

void ModelCatalog::reload() {
    QFile file(QStringLiteral(":/qt/qml/Vesper/resources/models.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = QStringLiteral("Bundled model manifest is missing");
        emit errorChanged();
        return;
    }

    const ManifestParseResult parsed = parseManifest(file.readAll());
    if (!parsed.ok()) {
        m_error = parsed.error;
        emit errorChanged();
        return;
    }

    beginResetModel();
    m_entries = parsed.models;
    m_runtime.clear();
    endResetModel();

    if (!m_error.isEmpty()) {
        m_error.clear();
        emit errorChanged();
    }

    refreshInstalledStates();
}

void ModelCatalog::refreshInstalledStates() {
    for (const ModelEntry& entry : m_entries) {
        if (m_runtime.value(entry.id).state == State::Downloading ||
            m_runtime.value(entry.id).state == State::Verifying) {
            continue;
        }
        m_runtime[entry.id] = {isInstalled(entry.id) ? State::Installed : State::NotInstalled,
                               isInstalled(entry.id) ? 1.0 : 0.0, QString()};
    }
    if (!m_entries.isEmpty()) {
        emit dataChanged(index(0), index(m_entries.size() - 1));
    }
    emit installedCountChanged();
}

int ModelCatalog::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant ModelCatalog::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_entries.size()) {
        return {};
    }

    const ModelEntry& entry = m_entries.at(index.row());
    const RuntimeState runtime = m_runtime.value(entry.id);

    switch (role) {
    case IdRole: return entry.id;
    case NameRole: return entry.name;
    case FamilyRole: return entry.family;
    case SizeBytesRole: return entry.totalSizeBytes;
    case SizeLabelRole: return ByteFormat::humanize(entry.totalSizeBytes);
    case LicenseNameRole: return entry.licenseName;
    case LicenseUrlRole: return entry.licenseUrl;
    case NotesRole: return entry.notes;
    case StateRole: return QVariant::fromValue(runtime.state);
    case StateLabelRole: return stateLabel(runtime.state);
    case ProgressRole: return runtime.progress;
    case ProgressLabelRole: return QStringLiteral("%1%").arg(runtime.progress * 100.0, 0, 'f', 0);
    case FileCountRole: return static_cast<int>(entry.files.size());
    case StatusDetailRole: return runtime.detail;
    default: return {};
    }
}

QHash<int, QByteArray> ModelCatalog::roleNames() const {
    return {
        {IdRole, "modelId"},
        {NameRole, "name"},
        {FamilyRole, "family"},
        {SizeBytesRole, "sizeBytes"},
        {SizeLabelRole, "sizeLabel"},
        {LicenseNameRole, "licenseName"},
        {LicenseUrlRole, "licenseUrl"},
        {NotesRole, "notes"},
        {StateRole, "state"},
        {StateLabelRole, "stateLabel"},
        {ProgressRole, "progress"},
        {ProgressLabelRole, "progressLabel"},
        {FileCountRole, "fileCount"},
        {StatusDetailRole, "statusDetail"},
    };
}

QString ModelCatalog::error() const {
    return m_error;
}

QString ModelCatalog::activeDownloadId() const {
    return m_activeDownloadId;
}

int ModelCatalog::installedCount() const {
    int count = 0;
    for (const ModelEntry& entry : m_entries) {
        if (isInstalled(entry.id)) {
            ++count;
        }
    }
    return count;
}

const ModelEntry* ModelCatalog::entryById(const QString& id) const {
    for (const ModelEntry& entry : m_entries) {
        if (entry.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

int ModelCatalog::indexOf(const QString& modelId) const {
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).id == modelId) {
            return i;
        }
    }
    return -1;
}

void ModelCatalog::notifyRow(const QString& modelId) {
    const int row = indexOf(modelId);
    if (row >= 0) {
        emit dataChanged(index(row), index(row));
    }
}

bool ModelCatalog::isInstalled(const QString& id) const {
    const ModelEntry* entry = entryById(id);
    if (!entry) {
        return false;
    }
    const QDir dir(modelsDirectory());
    for (const ModelFile& file : entry->files) {
        const QFileInfo info(dir.filePath(file.filename));
        if (!info.exists() || (file.sizeBytes > 0 && info.size() != file.sizeBytes)) {
            return false;
        }
    }
    return true;
}

QStringList ModelCatalog::installedFilePaths(const QString& id) const {
    QStringList paths;
    const ModelEntry* entry = entryById(id);
    if (!entry) {
        return paths;
    }
    const QDir dir(modelsDirectory());
    for (const ModelFile& file : entry->files) {
        paths.append(dir.filePath(file.filename));
    }
    return paths;
}

void ModelCatalog::download(const QString& modelId) {
    const ModelEntry* entry = entryById(modelId);
    if (!entry) {
        emit downloadFailed(modelId, QStringLiteral("Unknown model"));
        return;
    }
    m_downloader->start(*entry, modelsDirectory());
}

void ModelCatalog::cancelDownload() {
    m_downloader->cancel();
}

bool ModelCatalog::remove(const QString& modelId) {
    const ModelEntry* entry = entryById(modelId);
    if (!entry) {
        return false;
    }

    const QDir dir(modelsDirectory());
    bool allRemoved = true;
    for (const ModelFile& file : entry->files) {
        const QString path = dir.filePath(file.filename);
        if (QFile::exists(path) && !QFile::remove(path)) {
            allRemoved = false;
        }
        QFile::remove(ModelDownloader::partialPathFor(path));
    }

    m_runtime[modelId] = {State::NotInstalled, 0.0, QString()};
    notifyRow(modelId);
    emit installedCountChanged();
    emit modelRemoved(modelId);
    return allRemoved;
}

qint64 ModelCatalog::installedSize(const QString& modelId) const {
    qint64 total = 0;
    for (const QString& path : installedFilePaths(modelId)) {
        const QFileInfo info(path);
        if (info.exists()) {
            total += info.size();
        }
        const QFileInfo partial(ModelDownloader::partialPathFor(path));
        if (partial.exists()) {
            total += partial.size();
        }
    }
    return total;
}

QString ModelCatalog::installedSizeLabel(const QString& modelId) const {
    return ByteFormat::humanize(installedSize(modelId));
}

QStringList ModelCatalog::installedModelIds() const {
    QStringList ids;
    for (const ModelEntry& entry : m_entries) {
        if (isInstalled(entry.id)) {
            ids.append(entry.id);
        }
    }
    return ids;
}

QVariantMap ModelCatalog::defaultsFor(const QString& modelId) const {
    QVariantMap defaults;
    const ModelEntry* entry = entryById(modelId);
    if (!entry) {
        return defaults;
    }
    defaults.insert(QStringLiteral("sampler"), entry->defaultSampler);
    defaults.insert(QStringLiteral("steps"), entry->defaultSteps);
    defaults.insert(QStringLiteral("cfgScale"), entry->defaultCfgScale);
    defaults.insert(QStringLiteral("width"), entry->defaultWidth);
    defaults.insert(QStringLiteral("height"), entry->defaultHeight);
    return defaults;
}

}
