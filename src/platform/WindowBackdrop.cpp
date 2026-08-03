#include "platform/WindowBackdrop.h"

#include <QGuiApplication>

namespace vesper {

WindowBackdrop::WindowBackdrop(QObject* parent) : QObject(parent) {
    m_available = nativeBackdropSupported();
}

bool WindowBackdrop::nativeBlurAvailable() const {
    return m_available;
}

QString WindowBackdrop::platformName() {
    return QGuiApplication::platformName();
}

bool WindowBackdrop::applyTo(QQuickWindow* window, bool darkTheme) {
    if (!window) {
        return false;
    }
    return applyNativeBackdrop(window, darkTheme);
}

}
