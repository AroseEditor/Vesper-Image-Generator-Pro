#include "platform/WindowBackdrop.h"

namespace vesper {

bool nativeBackdropSupported() {
    return false;
}

bool applyNativeBackdrop(QQuickWindow* window, bool darkTheme) {
    Q_UNUSED(window);
    Q_UNUSED(darkTheme);
    return false;
}

}
