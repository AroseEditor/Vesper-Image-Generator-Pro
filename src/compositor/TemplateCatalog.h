#pragma once

#include "compositor/TextLayer.h"

#include <QAbstractListModel>
#include <qqmlintegration.h>

namespace vesper {

struct LayoutTemplate {
    QString id;
    QString name;
    QString description;
    QString promptPreset;
    QString negativePreset;
    int suggestedWidth = 768;
    int suggestedHeight = 512;
    QVector<TextLayer> layers;
};

class TemplateCatalog : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum Roles {
        TemplateIdRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        PromptPresetRole,
        NegativePresetRole,
        SuggestedWidthRole,
        SuggestedHeightRole,
    };

    explicit TemplateCatalog(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    static QVector<LayoutTemplate> loadAll();
    static const LayoutTemplate* find(const QVector<LayoutTemplate>& templates, const QString& id);

    const LayoutTemplate* byId(const QString& id) const;

    Q_INVOKABLE QVariantMap templateFor(const QString& id) const;

private:
    QVector<LayoutTemplate> m_templates;
};

}
