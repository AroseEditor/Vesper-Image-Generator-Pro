#include "app/AppSettings.h"

#include <QThread>

namespace vesper {

namespace {
constexpr auto kSelectedModel = "generation/selectedModelId";
constexpr auto kThreadCount = "generation/threadCount";
constexpr auto kBackend = "generation/backend";
constexpr auto kOffloadToCpu = "generation/offloadToCpu";
constexpr auto kFlashAttention = "generation/flashAttention";
constexpr auto kLicensePrefix = "licenses/";
}

AppSettings::AppSettings(QObject* parent) : QObject(parent) {}

QString AppSettings::selectedModelId() const {
    return m_settings.value(kSelectedModel).toString();
}

void AppSettings::setSelectedModelId(const QString& id) {
    if (selectedModelId() == id) {
        return;
    }
    m_settings.setValue(kSelectedModel, id);
    emit selectedModelIdChanged();
}

int AppSettings::threadCount() const {
    const int fallback = qMax(1, QThread::idealThreadCount() / 2);
    return m_settings.value(kThreadCount, fallback).toInt();
}

void AppSettings::setThreadCount(int count) {
    const int clamped = qBound(1, count, 64);
    if (threadCount() == clamped) {
        return;
    }
    m_settings.setValue(kThreadCount, clamped);
    emit threadCountChanged();
}

QString AppSettings::backend() const {
    return m_settings.value(kBackend, QStringLiteral("auto")).toString();
}

void AppSettings::setBackend(const QString& backend) {
    if (this->backend() == backend) {
        return;
    }
    m_settings.setValue(kBackend, backend);
    emit backendChanged();
}

bool AppSettings::offloadToCpu() const {
    return m_settings.value(kOffloadToCpu, false).toBool();
}

void AppSettings::setOffloadToCpu(bool enabled) {
    if (offloadToCpu() == enabled) {
        return;
    }
    m_settings.setValue(kOffloadToCpu, enabled);
    emit offloadToCpuChanged();
}

bool AppSettings::flashAttention() const {
    return m_settings.value(kFlashAttention, false).toBool();
}

void AppSettings::setFlashAttention(bool enabled) {
    if (flashAttention() == enabled) {
        return;
    }
    m_settings.setValue(kFlashAttention, enabled);
    emit flashAttentionChanged();
}

QString AppSettings::version() const {
    return QStringLiteral(VESPER_VERSION);
}

bool AppSettings::licenseAccepted(const QString& modelId) const {
    return m_settings.value(QString::fromLatin1(kLicensePrefix) + modelId, false).toBool();
}

void AppSettings::setLicenseAccepted(const QString& modelId, bool accepted) {
    m_settings.setValue(QString::fromLatin1(kLicensePrefix) + modelId, accepted);
    emit licenseAcceptanceChanged(modelId);
}

}
