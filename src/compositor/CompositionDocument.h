#pragma once

#include "compositor/TextLayer.h"

#include <QAbstractListModel>
#include <QSize>
#include <qqmlintegration.h>

namespace vesper {

class CompositionDocument : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString backgroundPath READ backgroundPath WRITE setBackgroundPath NOTIFY
                   backgroundPathChanged)
    Q_PROPERTY(QString backgroundUrl READ backgroundUrl NOTIFY backgroundPathChanged)
    Q_PROPERTY(QString templateId READ templateId WRITE setTemplateId NOTIFY templateIdChanged)
    Q_PROPERTY(int backgroundWidth READ backgroundWidth NOTIFY backgroundPathChanged)
    Q_PROPERTY(int backgroundHeight READ backgroundHeight NOTIFY backgroundPathChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)

public:
    enum Roles {
        LayerIdRole = Qt::UserRole + 1,
        TextRole,
        XRole,
        YRole,
        WidthRole,
        FontSizeRole,
        FontFamilyRole,
        AlignmentRole,
        ColorRole,
        LineHeightRole,
        LetterSpacingRole,
        RotationRole,
        OpacityRole,
        BoldRole,
        ItalicRole,
    };

    explicit CompositionDocument(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    QString backgroundPath() const;
    void setBackgroundPath(const QString& path);
    QString backgroundUrl() const;
    QString templateId() const;
    void setTemplateId(const QString& id);
    int backgroundWidth() const;
    int backgroundHeight() const;
    int selectedIndex() const;
    void setSelectedIndex(int index);
    bool isDirty() const;

    const QVector<TextLayer>& layers() const;

    Q_INVOKABLE void applyTemplate(const QString& templateId, const QString& title,
                                   const QString& body);
    Q_INVOKABLE int addLayer(const QString& text);
    Q_INVOKABLE void removeLayer(int row);
    Q_INVOKABLE void moveLayer(int row, double x, double y);
    Q_INVOKABLE void resizeLayer(int row, double width);
    Q_INVOKABLE void setLayerProperty(int row, const QString& property, const QVariant& value);
    Q_INVOKABLE QVariantMap layerAt(int row) const;

    Q_INVOKABLE bool saveProject(const QString& targetUrl);
    Q_INVOKABLE bool loadProject(const QString& sourceUrl);
    Q_INVOKABLE bool exportImage(const QString& targetUrl, int width, int height, int quality);
    Q_INVOKABLE void clear();

signals:
    void backgroundPathChanged();
    void templateIdChanged();
    void selectedIndexChanged();
    void countChanged();
    void dirtyChanged();
    void exported(const QString& path);
    void errorOccurred(const QString& reason);

private:
    void markDirty(bool dirty = true);

    QVector<TextLayer> m_layers;
    QString m_backgroundPath;
    QString m_templateId;
    QSize m_backgroundSize;
    int m_selectedIndex = -1;
    int m_nextLayerNumber = 1;
    bool m_dirty = false;
};

}
