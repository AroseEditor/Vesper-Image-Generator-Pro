import Vesper
import QtQuick

Item {
    id: root

    property string title: ""
    property string subtitle: ""

    signal minimizeRequested()
    signal maximizeRequested()
    signal closeRequested()

    implicitHeight: 52

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.gapLarge
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        Rectangle {
            width: 22
            height: 22
            radius: 7
            anchors.verticalCenter: parent.verticalCenter
            gradient: Gradient {
                GradientStop { position: 0.0; color: Theme.accent }
                GradientStop { position: 1.0; color: Theme.accentDeep }
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 1

            Text {
                text: root.title
                color: Theme.text
                font.pixelSize: Theme.fontBody
                font.weight: Font.DemiBold
            }

            Text {
                text: root.subtitle
                color: Theme.textFaint
                font.pixelSize: Theme.fontMicro
                visible: text.length > 0
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        spacing: 6

        Repeater {
            model: [
                { glyph: "–", action: "minimize" },
                { glyph: "□", action: "maximize" },
                { glyph: "×", action: "close" }
            ]

            Rectangle {
                required property var modelData

                width: 30
                height: 26
                radius: Theme.radiusSmall - 2
                color: hover.hovered
                       ? (modelData.action === "close" ? Theme.danger : Qt.rgba(1, 1, 1, 0.12))
                       : "transparent"

                Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                Text {
                    anchors.centerIn: parent
                    text: modelData.glyph
                    color: Theme.text
                    font.pixelSize: modelData.action === "close" ? 15 : 12
                }

                HoverHandler { id: hover }

                TapHandler {
                    onTapped: {
                        if (modelData.action === "minimize")
                            root.minimizeRequested();
                        else if (modelData.action === "maximize")
                            root.maximizeRequested();
                        else
                            root.closeRequested();
                    }
                }
            }
        }
    }
}
