#include "compositor/CompositionDocument.h"

#include "compositor/CompositionRenderer.h"
#include "compositor/TemplateCatalog.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace vesper {

namespace {

QString toLocalPath(const QString& value) {
    if (value.startsWith(QLatin1String("file:"))) {
        return QUrl(value).toLocalFile();
    }
    return value;
}

QString projectPathFor(const QString& imagePath) {
    const QFileInfo info(imagePath);
    return info.dir().filePath(info.completeBaseName() + QStringLiteral(".composition.json"));
}

}

CompositionDocument::CompositionDocument(QObject* parent) : QAbstractListModel(parent) {}

int CompositionDocument::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_layers.size());
}

QVariant CompositionDocument::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_layers.size()) {
        return {};
    }
    const TextLayer& layer = m_layers.at(index.row());
    switch (role) {
    case LayerIdRole: return layer.id;
    case TextRole: return layer.text;
    case XRole: return layer.x;
    case YRole: return layer.y;
    case WidthRole: return layer.width;
    case FontSizeRole: return layer.fontSize;
    case FontFamilyRole: return layer.fontFamily;
    case AlignmentRole: return layer.alignment;
    case ColorRole: return layer.color;
    case LineHeightRole: return layer.lineHeight;
    case LetterSpacingRole: return layer.letterSpacing;
    case RotationRole: return layer.rotation;
    case OpacityRole: return layer.opacity;
    case BoldRole: return layer.bold;
    case ItalicRole: return layer.italic;
    default: return {};
    }
}

bool CompositionDocument::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= m_layers.size()) {
        return false;
    }
    TextLayer& layer = m_layers[index.row()];
    switch (role) {
    case TextRole: layer.text = value.toString(); break;
    case XRole: layer.x = value.toDouble(); break;
    case YRole: layer.y = value.toDouble(); break;
    case WidthRole: layer.width = qMax(0.02, value.toDouble()); break;
    case FontSizeRole: layer.fontSize = qMax(0.005, value.toDouble()); break;
    case FontFamilyRole: layer.fontFamily = value.toString(); break;
    case AlignmentRole: layer.alignment = value.toString(); break;
    case ColorRole: layer.color = value.value<QColor>(); break;
    case LineHeightRole: layer.lineHeight = qMax(0.5, value.toDouble()); break;
    case LetterSpacingRole: layer.letterSpacing = value.toDouble(); break;
    case RotationRole: layer.rotation = value.toDouble(); break;
    case OpacityRole: layer.opacity = qBound(0.0, value.toDouble(), 1.0); break;
    case BoldRole: layer.bold = value.toBool(); break;
    case ItalicRole: layer.italic = value.toBool(); break;
    default: return false;
    }
    emit dataChanged(index, index, {role});
    markDirty();
    return true;
}

QHash<int, QByteArray> CompositionDocument::roleNames() const {
    return {
        {LayerIdRole, "layerId"},   {TextRole, "text"},
        {XRole, "x"},               {YRole, "y"},
        {WidthRole, "width"},       {FontSizeRole, "fontSize"},
        {FontFamilyRole, "fontFamily"}, {AlignmentRole, "alignment"},
        {ColorRole, "color"},       {LineHeightRole, "lineHeight"},
        {LetterSpacingRole, "letterSpacing"}, {RotationRole, "rotation"},
        {OpacityRole, "opacity"},   {BoldRole, "bold"},
        {ItalicRole, "italic"},
    };
}

QString CompositionDocument::backgroundPath() const {
    return m_backgroundPath;
}

QString CompositionDocument::backgroundUrl() const {
    return m_backgroundPath.isEmpty() ? QString() : QUrl::fromLocalFile(m_backgroundPath).toString();
}

void CompositionDocument::setBackgroundPath(const QString& path) {
    const QString local = toLocalPath(path);
    if (m_backgroundPath == local) {
        return;
    }
    m_backgroundPath = local;

    QImageReader reader(local);
    m_backgroundSize = reader.size();

    emit backgroundPathChanged();
    markDirty();
}

int CompositionDocument::backgroundWidth() const {
    return m_backgroundSize.isValid() ? m_backgroundSize.width() : 0;
}

int CompositionDocument::backgroundHeight() const {
    return m_backgroundSize.isValid() ? m_backgroundSize.height() : 0;
}

QString CompositionDocument::templateId() const {
    return m_templateId;
}

void CompositionDocument::setTemplateId(const QString& id) {
    if (m_templateId == id) {
        return;
    }
    m_templateId = id;
    emit templateIdChanged();
}

int CompositionDocument::selectedIndex() const {
    return m_selectedIndex;
}

void CompositionDocument::setSelectedIndex(int index) {
    const int clamped = (index >= 0 && index < m_layers.size()) ? index : -1;
    if (m_selectedIndex == clamped) {
        return;
    }
    m_selectedIndex = clamped;
    emit selectedIndexChanged();
}

bool CompositionDocument::isDirty() const {
    return m_dirty;
}

void CompositionDocument::markDirty(bool dirty) {
    if (m_dirty == dirty) {
        return;
    }
    m_dirty = dirty;
    emit dirtyChanged();
}

const QVector<TextLayer>& CompositionDocument::layers() const {
    return m_layers;
}

void CompositionDocument::applyTemplate(const QString& templateId, const QString& title,
                                        const QString& body) {
    const QVector<LayoutTemplate> templates = TemplateCatalog::loadAll();
    const LayoutTemplate* layout = TemplateCatalog::find(templates, templateId);
    if (!layout) {
        emit errorOccurred(QStringLiteral("Unknown template '%1'").arg(templateId));
        return;
    }

    beginResetModel();
    m_layers = layout->layers;
    for (TextLayer& layer : m_layers) {
        if (layer.id == QLatin1String("title") && !title.isEmpty()) {
            layer.text = title;
        } else if (layer.id == QLatin1String("body") && !body.isEmpty()) {
            layer.text = body;
        }
    }
    m_nextLayerNumber = static_cast<int>(m_layers.size()) + 1;
    endResetModel();

    setTemplateId(templateId);
    setSelectedIndex(m_layers.isEmpty() ? -1 : 0);
    emit countChanged();
    markDirty();
}

int CompositionDocument::addLayer(const QString& text) {
    TextLayer layer;
    layer.id = QStringLiteral("layer%1").arg(m_nextLayerNumber++);
    layer.text = text.isEmpty() ? QStringLiteral("New text") : text;
    layer.x = 0.1;
    layer.y = 0.1;
    layer.width = 0.4;

    const int row = static_cast<int>(m_layers.size());
    beginInsertRows(QModelIndex(), row, row);
    m_layers.append(layer);
    endInsertRows();

    emit countChanged();
    setSelectedIndex(row);
    markDirty();
    return row;
}

void CompositionDocument::removeLayer(int row) {
    if (row < 0 || row >= m_layers.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_layers.remove(row);
    endRemoveRows();

    emit countChanged();
    setSelectedIndex(qMin(row, static_cast<int>(m_layers.size()) - 1));
    markDirty();
}

void CompositionDocument::moveLayer(int row, double x, double y) {
    if (row < 0 || row >= m_layers.size()) {
        return;
    }
    m_layers[row].x = x;
    m_layers[row].y = y;
    emit dataChanged(index(row), index(row), {XRole, YRole});
    markDirty();
}

void CompositionDocument::resizeLayer(int row, double width) {
    setData(index(row), width, WidthRole);
}

void CompositionDocument::setLayerProperty(int row, const QString& property, const QVariant& value) {
    static const QHash<QString, int> mapping{
        {QStringLiteral("text"), TextRole},
        {QStringLiteral("x"), XRole},
        {QStringLiteral("y"), YRole},
        {QStringLiteral("width"), WidthRole},
        {QStringLiteral("fontSize"), FontSizeRole},
        {QStringLiteral("fontFamily"), FontFamilyRole},
        {QStringLiteral("alignment"), AlignmentRole},
        {QStringLiteral("color"), ColorRole},
        {QStringLiteral("lineHeight"), LineHeightRole},
        {QStringLiteral("letterSpacing"), LetterSpacingRole},
        {QStringLiteral("rotation"), RotationRole},
        {QStringLiteral("opacity"), OpacityRole},
        {QStringLiteral("bold"), BoldRole},
        {QStringLiteral("italic"), ItalicRole},
    };
    const int role = mapping.value(property, -1);
    if (role >= 0) {
        setData(index(row), value, role);
    }
}

QVariantMap CompositionDocument::layerAt(int row) const {
    QVariantMap map;
    if (row < 0 || row >= m_layers.size()) {
        return map;
    }
    const TextLayer& layer = m_layers.at(row);
    map.insert(QStringLiteral("layerId"), layer.id);
    map.insert(QStringLiteral("text"), layer.text);
    map.insert(QStringLiteral("x"), layer.x);
    map.insert(QStringLiteral("y"), layer.y);
    map.insert(QStringLiteral("width"), layer.width);
    map.insert(QStringLiteral("fontSize"), layer.fontSize);
    map.insert(QStringLiteral("fontFamily"), layer.fontFamily);
    map.insert(QStringLiteral("alignment"), layer.alignment);
    map.insert(QStringLiteral("color"), layer.color);
    map.insert(QStringLiteral("lineHeight"), layer.lineHeight);
    map.insert(QStringLiteral("letterSpacing"), layer.letterSpacing);
    map.insert(QStringLiteral("rotation"), layer.rotation);
    map.insert(QStringLiteral("opacity"), layer.opacity);
    map.insert(QStringLiteral("bold"), layer.bold);
    map.insert(QStringLiteral("italic"), layer.italic);
    return map;
}

bool CompositionDocument::saveProject(const QString& targetUrl) {
    const QString target = toLocalPath(targetUrl);
    if (target.isEmpty()) {
        emit errorOccurred(QStringLiteral("No save location given"));
        return false;
    }

    const QFileInfo info(target);
    const QString backgroundCopy = info.dir().filePath(info.completeBaseName() + QStringLiteral(".png"));
    if (!m_backgroundPath.isEmpty() && m_backgroundPath != backgroundCopy) {
        QFile::remove(backgroundCopy);
        if (!QFile::copy(m_backgroundPath, backgroundCopy)) {
            emit errorOccurred(QStringLiteral("Could not copy the background image"));
            return false;
        }
    }

    QJsonArray layerArray;
    for (const TextLayer& layer : m_layers) {
        layerArray.append(layer.toJson());
    }

    const QJsonObject root{
        {QStringLiteral("schema_version"), 1},
        {QStringLiteral("template_id"), m_templateId},
        {QStringLiteral("background"), QFileInfo(backgroundCopy).fileName()},
        {QStringLiteral("background_width"), backgroundWidth()},
        {QStringLiteral("background_height"), backgroundHeight()},
        {QStringLiteral("layers"), layerArray},
    };

    const QString projectPath = projectPathFor(backgroundCopy);
    QFile file(projectPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit errorOccurred(QStringLiteral("Could not write the composition file"));
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    markDirty(false);
    return true;
}

bool CompositionDocument::loadProject(const QString& sourceUrl) {
    const QString source = toLocalPath(sourceUrl);
    QFile file(source);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QStringLiteral("Could not open the composition file"));
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        emit errorOccurred(QStringLiteral("The composition file is not valid JSON"));
        return false;
    }

    const QJsonObject root = document.object();
    const QFileInfo info(source);
    const QString background = root.value(QStringLiteral("background")).toString();

    beginResetModel();
    m_layers.clear();
    const QJsonArray layers = root.value(QStringLiteral("layers")).toArray();
    for (const QJsonValue& value : layers) {
        m_layers.append(TextLayer::fromJson(value.toObject()));
    }
    m_nextLayerNumber = static_cast<int>(m_layers.size()) + 1;
    endResetModel();

    setTemplateId(root.value(QStringLiteral("template_id")).toString());
    if (!background.isEmpty()) {
        setBackgroundPath(info.dir().filePath(background));
    }

    emit countChanged();
    setSelectedIndex(m_layers.isEmpty() ? -1 : 0);
    markDirty(false);
    return true;
}

bool CompositionDocument::exportImage(const QString& targetUrl, int width, int height, int quality) {
    const QString target = toLocalPath(targetUrl);
    if (target.isEmpty()) {
        emit errorOccurred(QStringLiteral("No export location given"));
        return false;
    }

    QSize size(width, height);
    if (!size.isValid() || size.isEmpty()) {
        size = m_backgroundSize.isValid() ? m_backgroundSize : QSize(1024, 1024);
    }

    const RenderResult result = renderComposition(m_backgroundPath, m_layers, size);
    if (!result.ok()) {
        emit errorOccurred(result.error);
        return false;
    }

    if (!result.image.save(target, nullptr, quality)) {
        emit errorOccurred(QStringLiteral("Could not write %1").arg(target));
        return false;
    }

    emit exported(target);
    return true;
}

void CompositionDocument::clear() {
    beginResetModel();
    m_layers.clear();
    m_nextLayerNumber = 1;
    endResetModel();

    m_backgroundPath.clear();
    m_backgroundSize = QSize();
    emit backgroundPathChanged();
    emit countChanged();
    setSelectedIndex(-1);
    markDirty(false);
}

}
