#pragma once

#include "gallery/GenerationMetadata.h"

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <qqmlintegration.h>

namespace vesper {

class GalleryModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString directory READ directory CONSTANT)

public:
    enum Roles {
        ImagePathRole = Qt::UserRole + 1,
        ImageUrlRole,
        FileNameRole,
        PromptRole,
        NegativePromptRole,
        ModelIdRole,
        SeedRole,
        SamplerRole,
        StepsRole,
        CfgScaleRole,
        DimensionsRole,
        CreatedAtRole,
        ElapsedLabelRole,
    };

    explicit GalleryModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    static QString directory();

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool removeAt(int row);
    Q_INVOKABLE bool exportTo(int row, const QString& targetUrl);
    Q_INVOKABLE QVariantMap settingsAt(int row) const;
    Q_INVOKABLE void revealDirectory() const;

signals:
    void countChanged();

private:
    struct Item {
        QString imagePath;
        GenerationMetadata metadata;
    };

    QVector<Item> m_items;
    QFileSystemWatcher m_watcher;
};

}
