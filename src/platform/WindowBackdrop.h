#pragma once

#include <QObject>
#include <QQuickWindow>
#include <qqmlintegration.h>

namespace vesper {

class WindowBackdrop : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool nativeBlurAvailable READ nativeBlurAvailable CONSTANT)
    Q_PROPERTY(QString platformName READ platformName CONSTANT)

public:
    explicit WindowBackdrop(QObject* parent = nullptr);

    bool nativeBlurAvailable() const;
    static QString platformName();

    Q_INVOKABLE bool applyTo(QQuickWindow* window, bool darkTheme);

private:
    bool m_available = false;
};

bool applyNativeBackdrop(QQuickWindow* window, bool darkTheme);
bool nativeBackdropSupported();

}
