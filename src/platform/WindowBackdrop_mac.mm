#include "platform/WindowBackdrop.h"

#import <AppKit/AppKit.h>

namespace vesper {

bool nativeBackdropSupported() {
    return true;
}

bool applyNativeBackdrop(QQuickWindow* window, bool darkTheme) {
    auto* nativeView = reinterpret_cast<NSView*>(window->winId());
    if (!nativeView) {
        return false;
    }

    NSWindow* nativeWindow = [nativeView window];
    if (!nativeWindow) {
        return false;
    }

    NSVisualEffectView* effect = [[NSVisualEffectView alloc] initWithFrame:nativeView.bounds];
    effect.material = NSVisualEffectMaterialUnderWindowBackground;
    effect.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    effect.state = NSVisualEffectStateActive;
    effect.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    NSView* contentView = [nativeWindow contentView];
    [contentView addSubview:effect positioned:NSWindowBelow relativeTo:nil];

    nativeWindow.titlebarAppearsTransparent = YES;
    nativeWindow.appearance = [NSAppearance appearanceNamed:darkTheme ? NSAppearanceNameDarkAqua
                                                                      : NSAppearanceNameAqua];
    return true;
}

}
