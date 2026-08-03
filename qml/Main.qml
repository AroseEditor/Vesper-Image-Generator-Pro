import QtQuick
import QtQuick.Window
import QtQuick.Effects
import Vesper

Window {
    id: window

    width: 1280
    height: 840
    minimumWidth: 1040
    minimumHeight: 680
    visible: true
    title: "Vesper Image Generator Pro"
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    readonly property int edgeMargin: 6
    readonly property var screenNames: ["Generate", "Models", "Gallery", "Composer", "Settings"]

    property int currentScreen: 0

    Component.onCompleted: WindowBackdrop.applyTo(window, true)

    Rectangle {
        id: shell
        anchors.fill: parent
        radius: window.visibility === Window.Maximized ? 0 : Theme.radiusLarge
        color: Theme.canvas
        border.width: 1
        border.color: Theme.glassEdgeSoft

        Item {
            id: auroraSource
            anchors.fill: parent
            visible: false
            layer.enabled: true

            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#191231" }
                    GradientStop { position: 0.55; color: "#120d1e" }
                    GradientStop { position: 1.0; color: "#0b0812" }
                }
            }

            Rectangle {
                width: parent.width * 0.62
                height: width
                radius: width / 2
                x: -width * 0.22
                y: -height * 0.3
                color: Theme.accentDeep
                opacity: 0.55

                SequentialAnimation on x {
                    loops: Animation.Infinite
                    NumberAnimation { to: -auroraSource.width * 0.05; duration: 14000; easing.type: Easing.InOutSine }
                    NumberAnimation { to: -auroraSource.width * 0.22; duration: 14000; easing.type: Easing.InOutSine }
                }
            }

            Rectangle {
                width: parent.width * 0.5
                height: width
                radius: width / 2
                x: parent.width * 0.62
                y: parent.height * 0.48
                color: "#8f3f7a"
                opacity: 0.42

                SequentialAnimation on y {
                    loops: Animation.Infinite
                    NumberAnimation { to: auroraSource.height * 0.62; duration: 17000; easing.type: Easing.InOutSine }
                    NumberAnimation { to: auroraSource.height * 0.4; duration: 17000; easing.type: Easing.InOutSine }
                }
            }

            Rectangle {
                width: parent.width * 0.36
                height: width
                radius: width / 2
                x: parent.width * 0.26
                y: parent.height * 0.66
                color: "#3f6ad6"
                opacity: 0.3
            }
        }

        MultiEffect {
            id: aurora
            anchors.fill: parent
            source: auroraSource
            blurEnabled: true
            blur: 1.0
            blurMax: 64
            saturation: 0.24
            brightness: -0.04
            layer.enabled: true
            layer.effect: MultiEffect {
                maskEnabled: true
                maskSource: shellMask
            }
        }

        Item {
            id: shellMask
            anchors.fill: parent
            visible: false
            layer.enabled: true

            Rectangle {
                anchors.fill: parent
                radius: shell.radius
                color: "black"
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: shell.radius
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.09)
        }

        TitleBar {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            title: "Vesper Image Generator Pro"
            subtitle: InferenceBridge.engineAvailable ? InferenceBridge.statusText
                                                      : "Engine missing"

            onMinimizeRequested: window.showMinimized()
            onMaximizeRequested: window.visibility === Window.Maximized ? window.showNormal()
                                                                       : window.showMaximized()
            onCloseRequested: window.close()

            DragHandler {
                target: null
                grabPermissions: PointerHandler.CanTakeOverFromAnything
                onActiveChanged: if (active) window.startSystemMove()
            }

            TapHandler {
                onDoubleTapped: window.visibility === Window.Maximized ? window.showNormal()
                                                                      : window.showMaximized()
            }
        }

        NavRail {
            id: nav
            anchors.left: parent.left
            anchors.leftMargin: Theme.gapSmall
            anchors.top: titleBar.bottom
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.gapSmall
            entries: window.screenNames
            currentIndex: window.currentScreen
            onSelected: function(index) { window.currentScreen = index }
        }

        StackLayoutHost {
            id: host
            anchors.left: nav.right
            anchors.right: parent.right
            anchors.top: titleBar.bottom
            anchors.bottom: parent.bottom
            anchors.rightMargin: Theme.gapMedium
            anchors.bottomMargin: Theme.gapMedium
            anchors.topMargin: 2
            currentIndex: window.currentScreen
        }
    }

    component StackLayoutHost: Item {
        property int currentIndex: 0

        GenerateScreen {
            anchors.fill: parent
            visible: parent.currentIndex === 0
            opacity: visible ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
        }

        ModelsScreen {
            anchors.fill: parent
            visible: parent.currentIndex === 1
            opacity: visible ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
        }

        GalleryScreen {
            anchors.fill: parent
            visible: parent.currentIndex === 2
            opacity: visible ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
            onEditRequested: function(path) {
                window.currentScreen = 3;
                composerBridge.requestBackground(path);
            }
        }

        ComposerScreen {
            id: composerScreen
            anchors.fill: parent
            visible: parent.currentIndex === 3
            opacity: visible ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
        }

        SettingsScreen {
            anchors.fill: parent
            visible: parent.currentIndex === 4
            opacity: visible ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
        }

        QtObject {
            id: composerBridge
            function requestBackground(path) { composerScreen.loadBackground(path) }
        }
    }

    Repeater {
        model: [Qt.LeftEdge, Qt.RightEdge, Qt.TopEdge, Qt.BottomEdge,
                Qt.LeftEdge | Qt.TopEdge, Qt.RightEdge | Qt.TopEdge,
                Qt.LeftEdge | Qt.BottomEdge, Qt.RightEdge | Qt.BottomEdge]

        Item {
            required property int modelData
            required property int index

            readonly property bool horizontal: modelData === Qt.LeftEdge || modelData === Qt.RightEdge
            readonly property bool vertical: modelData === Qt.TopEdge || modelData === Qt.BottomEdge
            readonly property bool corner: index >= 4

            width: horizontal ? window.edgeMargin : (corner ? 14 : window.width - window.edgeMargin * 2)
            height: vertical ? window.edgeMargin : (corner ? 14 : window.height - window.edgeMargin * 2)

            x: (modelData & Qt.RightEdge) ? window.width - width : 0
            y: (modelData & Qt.BottomEdge) ? window.height - height : 0

            visible: window.visibility !== Window.Maximized

            DragHandler {
                target: null
                grabPermissions: PointerHandler.CanTakeOverFromAnything
                onActiveChanged: if (active) window.startSystemResize(modelData)
            }

            HoverHandler {
                cursorShape: {
                    if (corner)
                        return ((modelData & Qt.LeftEdge) && (modelData & Qt.TopEdge)) ||
                               ((modelData & Qt.RightEdge) && (modelData & Qt.BottomEdge))
                               ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor;
                    return horizontal ? Qt.SizeHorCursor : Qt.SizeVerCursor;
                }
            }
        }
    }
}
