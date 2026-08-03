#include "gallery/GalleryModel.h"

#include "app/AppPaths.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

namespace vesper {

GalleryModel::GalleryModel(QObject* parent) : QAbstractListModel(parent) {
    QDir().mkpath(directory());
    m_watcher.addPath(directory());
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &GalleryModel::refresh);
    refresh();
}

QString GalleryModel::directory() {
    return AppPaths::galleryDirectory();
}

void GalleryModel::refresh() {
    QDir dir(directory());
    const QStringList files =
        dir.entryList({QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg")},
                      QDir::Files, QDir::Time);

    beginResetModel();
    m_items.clear();
    for (const QString& name : files) {
        Item item;
        item.imagePath = dir.filePath(name);
        const QString sidecar = GenerationMetadata::sidecarPathFor(item.imagePath);
        if (QFile::exists(sidecar)) {
            item.metadata = GenerationMetadata::load(sidecar);
        }
        m_items.append(item);
    }
    endResetModel();

    if (!m_watcher.directories().contains(directory())) {
        m_watcher.addPath(directory());
    }
    emit countChanged();
}

int GalleryModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_items.size());
}

QVariant GalleryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) {
        return {};
    }

    const Item& item = m_items.at(index.row());
    const GenerationMetadata& meta = item.metadata;

    switch (role) {
    case ImagePathRole: return item.imagePath;
    case ImageUrlRole: return QUrl::fromLocalFile(item.imagePath).toString();
    case FileNameRole: return QFileInfo(item.imagePath).fileName();
    case PromptRole: return meta.prompt;
    case NegativePromptRole: return meta.negativePrompt;
    case ModelIdRole: return meta.modelId;
    case SeedRole: return meta.seed;
    case SamplerRole: return meta.sampler;
    case StepsRole: return meta.steps;
    case CfgScaleRole: return meta.cfgScale;
    case DimensionsRole:
        return meta.width > 0 ? QStringLiteral("%1 x %2").arg(meta.width).arg(meta.height) : QString();
    case CreatedAtRole: return meta.createdAt;
    case ElapsedLabelRole:
        return meta.elapsedMs > 0 ? QStringLiteral("%1s").arg(meta.elapsedMs / 1000.0, 0, 'f', 1)
                                  : QString();
    default: return {};
    }
}

QHash<int, QByteArray> GalleryModel::roleNames() const {
    return {
        {ImagePathRole, "imagePath"},   {ImageUrlRole, "imageUrl"},
        {FileNameRole, "fileName"},     {PromptRole, "prompt"},
        {NegativePromptRole, "negativePrompt"}, {ModelIdRole, "modelId"},
        {SeedRole, "seed"},             {SamplerRole, "sampler"},
        {StepsRole, "steps"},           {CfgScaleRole, "cfgScale"},
        {DimensionsRole, "dimensions"}, {CreatedAtRole, "createdAt"},
        {ElapsedLabelRole, "elapsedLabel"},
    };
}

bool GalleryModel::removeAt(int row) {
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    const QString imagePath = m_items.at(row).imagePath;
    QFile::remove(GenerationMetadata::sidecarPathFor(imagePath));
    const bool removed = QFile::remove(imagePath);
    refresh();
    return removed;
}

bool GalleryModel::exportTo(int row, const QString& targetUrl) {
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    const QString target = QUrl(targetUrl).isLocalFile() ? QUrl(targetUrl).toLocalFile() : targetUrl;
    QFile::remove(target);
    return QFile::copy(m_items.at(row).imagePath, target);
}

QVariantMap GalleryModel::settingsAt(int row) const {
    QVariantMap map;
    if (row < 0 || row >= m_items.size()) {
        return map;
    }
    const GenerationMetadata& meta = m_items.at(row).metadata;
    map.insert(QStringLiteral("prompt"), meta.prompt);
    map.insert(QStringLiteral("negativePrompt"), meta.negativePrompt);
    map.insert(QStringLiteral("modelId"), meta.modelId);
    map.insert(QStringLiteral("sampler"), meta.sampler);
    map.insert(QStringLiteral("steps"), meta.steps);
    map.insert(QStringLiteral("cfgScale"), meta.cfgScale);
    map.insert(QStringLiteral("width"), meta.width);
    map.insert(QStringLiteral("height"), meta.height);
    map.insert(QStringLiteral("seed"), meta.seed);
    return map;
}

void GalleryModel::revealDirectory() const {
    QDesktopServices::openUrl(QUrl::fromLocalFile(directory()));
}

}
