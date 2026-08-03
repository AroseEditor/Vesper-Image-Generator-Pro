#include "platform/WindowBackdrop.h"

#include <QOperatingSystemVersion>
#include <dwmapi.h>
#include <windows.h>

namespace vesper {

namespace {

constexpr DWORD kUseImmersiveDarkMode = 20;
constexpr DWORD kSystemBackdropType = 38;
constexpr DWORD kMicaAlt = 4;

bool supportsSystemBackdrop() {
    const QOperatingSystemVersion current = QOperatingSystemVersion::current();
    if (current.majorVersion() < 10) {
        return false;
    }
    return current.microVersion() >= 22621;
}

}

bool nativeBackdropSupported() {
    return supportsSystemBackdrop();
}

bool applyNativeBackdrop(QQuickWindow* window, bool darkTheme) {
    if (!supportsSystemBackdrop()) {
        return false;
    }

    auto handle = reinterpret_cast<HWND>(window->winId());
    if (!handle) {
        return false;
    }

    BOOL dark = darkTheme ? TRUE : FALSE;
    DwmSetWindowAttribute(handle, kUseImmersiveDarkMode, &dark, sizeof(dark));

    DWORD backdrop = kMicaAlt;
    return SUCCEEDED(DwmSetWindowAttribute(handle, kSystemBackdropType, &backdrop, sizeof(backdrop)));
}

}
