#include "compositor/TemplateCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace vesper {

namespace {
const QStringList& templateResources() {
    static const QStringList paths{
        QStringLiteral(":/qt/qml/Vesper/resources/templates/letter.json"),
        QStringLiteral(":/qt/qml/Vesper/resources/templates/card.json"),
        QStringLiteral(":/qt/qml/Vesper/resources/templates/poster.json"),
    };
    return paths;
}
}

TemplateCatalog::TemplateCatalog(QObject* parent) : QAbstractListModel(parent) {
    m_templates = loadAll();
}

QVector<LayoutTemplate> TemplateCatalog::loadAll() {
    QVector<LayoutTemplate> templates;

    for (const QString& path : templateResources()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (!document.isObject()) {
            continue;
        }
        const QJsonObject object = document.object();

        LayoutTemplate layout;
        layout.id = object.value(QStringLiteral("id")).toString();
        layout.name = object.value(QStringLiteral("name")).toString();
        layout.description = object.value(QStringLiteral("description")).toString();
        layout.promptPreset = object.value(QStringLiteral("prompt_preset")).toString();
        layout.negativePreset = object.value(QStringLiteral("negative_preset")).toString();
        layout.suggestedWidth = object.value(QStringLiteral("suggested_width")).toInt(768);
        layout.suggestedHeight = object.value(QStringLiteral("suggested_height")).toInt(512);

        const QJsonArray layers = object.value(QStringLiteral("layers")).toArray();
        for (const QJsonValue& value : layers) {
            layout.layers.append(TextLayer::fromJson(value.toObject()));
        }

        if (!layout.id.isEmpty()) {
            templates.append(layout);
        }
    }

    return templates;
}

const LayoutTemplate* TemplateCatalog::find(const QVector<LayoutTemplate>& templates,
                                            const QString& id) {
    for (const LayoutTemplate& layout : templates) {
        if (layout.id == id) {
            return &layout;
        }
    }
    return nullptr;
}

const LayoutTemplate* TemplateCatalog::byId(const QString& id) const {
    return find(m_templates, id);
}

int TemplateCatalog::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_templates.size());
}

QVariant TemplateCatalog::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_templates.size()) {
        return {};
    }
    const LayoutTemplate& layout = m_templates.at(index.row());
    switch (role) {
    case TemplateIdRole: return layout.id;
    case NameRole: return layout.name;
    case DescriptionRole: return layout.description;
    case PromptPresetRole: return layout.promptPreset;
    case NegativePresetRole: return layout.negativePreset;
    case SuggestedWidthRole: return layout.suggestedWidth;
    case SuggestedHeightRole: return layout.suggestedHeight;
    default: return {};
    }
}

QHash<int, QByteArray> TemplateCatalog::roleNames() const {
    return {
        {TemplateIdRole, "templateId"},
        {NameRole, "name"},
        {DescriptionRole, "description"},
        {PromptPresetRole, "promptPreset"},
        {NegativePresetRole, "negativePreset"},
        {SuggestedWidthRole, "suggestedWidth"},
        {SuggestedHeightRole, "suggestedHeight"},
    };
}

QVariantMap TemplateCatalog::templateFor(const QString& id) const {
    QVariantMap map;
    const LayoutTemplate* layout = byId(id);
    if (!layout) {
        return map;
    }
    map.insert(QStringLiteral("id"), layout->id);
    map.insert(QStringLiteral("name"), layout->name);
    map.insert(QStringLiteral("description"), layout->description);
    map.insert(QStringLiteral("promptPreset"), layout->promptPreset);
    map.insert(QStringLiteral("negativePreset"), layout->negativePreset);
    map.insert(QStringLiteral("suggestedWidth"), layout->suggestedWidth);
    map.insert(QStringLiteral("suggestedHeight"), layout->suggestedHeight);
    return map;
}

}
