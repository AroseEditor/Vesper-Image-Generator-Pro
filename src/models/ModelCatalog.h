#pragma once

#include "models/ModelManifest.h"

#include <QAbstractListModel>
#include <QHash>
#include <qqmlintegration.h>

namespace vesper {

class ModelDownloader;

class ModelCatalog : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString activeDownloadId READ activeDownloadId NOTIFY activeDownloadIdChanged)
    Q_PROPERTY(int installedCount READ installedCount NOTIFY installedCountChanged)
    Q_PROPERTY(QString modelsDirectory READ modelsDirectory CONSTANT)

public:
    enum class State {
        NotInstalled,
        Downloading,
        Verifying,
        Installed,
        Failed,
    };
    Q_ENUM(State)

    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        FamilyRole,
        SizeBytesRole,
        SizeLabelRole,
        LicenseNameRole,
        LicenseUrlRole,
        NotesRole,
        StateRole,
        StateLabelRole,
        ProgressRole,
        ProgressLabelRole,
        FileCountRole,
        StatusDetailRole,
    };

    explicit ModelCatalog(QObject* parent = nullptr);
    ~ModelCatalog() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString error() const;
    QString activeDownloadId() const;
    int installedCount() const;
    static QString modelsDirectory();

    const ModelEntry* entryById(const QString& id) const;
    bool isInstalled(const QString& id) const;
    QStringList installedFilePaths(const QString& id) const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void download(const QString& modelId);
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE bool remove(const QString& modelId);
    Q_INVOKABLE qint64 installedSize(const QString& modelId) const;
    Q_INVOKABLE QString installedSizeLabel(const QString& modelId) const;
    Q_INVOKABLE QStringList installedModelIds() const;
    Q_INVOKABLE QVariantMap defaultsFor(const QString& modelId) const;

signals:
    void errorChanged();
    void activeDownloadIdChanged();
    void installedCountChanged();
    void downloadFailed(const QString& modelId, const QString& reason);
    void downloadFinished(const QString& modelId);
    void modelRemoved(const QString& modelId);

private:
    struct RuntimeState {
        State state = State::NotInstalled;
        double progress = 0.0;
        QString detail;
    };

    void refreshInstalledStates();
    void notifyRow(const QString& modelId);
    int indexOf(const QString& modelId) const;

    QVector<ModelEntry> m_entries;
    QHash<QString, RuntimeState> m_runtime;
    ModelDownloader* m_downloader = nullptr;
    QString m_error;
    QString m_activeDownloadId;
};

}
