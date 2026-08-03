#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <qqmlintegration.h>

namespace vesper {

class AppSettings : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString selectedModelId READ selectedModelId WRITE setSelectedModelId NOTIFY
                   selectedModelIdChanged)
    Q_PROPERTY(int threadCount READ threadCount WRITE setThreadCount NOTIFY threadCountChanged)
    Q_PROPERTY(QString backend READ backend WRITE setBackend NOTIFY backendChanged)
    Q_PROPERTY(bool offloadToCpu READ offloadToCpu WRITE setOffloadToCpu NOTIFY offloadToCpuChanged)
    Q_PROPERTY(bool flashAttention READ flashAttention WRITE setFlashAttention NOTIFY
                   flashAttentionChanged)
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    explicit AppSettings(QObject* parent = nullptr);

    QString selectedModelId() const;
    void setSelectedModelId(const QString& id);

    int threadCount() const;
    void setThreadCount(int count);

    QString backend() const;
    void setBackend(const QString& backend);

    bool offloadToCpu() const;
    void setOffloadToCpu(bool enabled);

    bool flashAttention() const;
    void setFlashAttention(bool enabled);

    QString version() const;

    Q_INVOKABLE bool licenseAccepted(const QString& modelId) const;
    Q_INVOKABLE void setLicenseAccepted(const QString& modelId, bool accepted);

signals:
    void selectedModelIdChanged();
    void threadCountChanged();
    void backendChanged();
    void offloadToCpuChanged();
    void flashAttentionChanged();
    void licenseAcceptanceChanged(const QString& modelId);

private:
    QSettings m_settings;
};

}
