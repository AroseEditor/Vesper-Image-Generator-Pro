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
            model: ["minimize", "maximize", "close"]

            Rectangle {
                id: windowButton
                required property string modelData

                width: 30
                height: 26
                radius: Theme.radiusSmall - 2
                color: hover.hovered
                       ? (modelData === "close" ? Theme.danger : Qt.rgba(1, 1, 1, 0.12))
                       : "transparent"

                Behavior on color { ColorAnimation { duration: Theme.durationFast } }

                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 1
                    color: Theme.text
                    visible: windowButton.modelData === "minimize"
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 9
                    height: 9
                    color: "transparent"
                    border.width: 1
                    border.color: Theme.text
                    visible: windowButton.modelData === "maximize"
                }

                Item {
                    anchors.centerIn: parent
                    width: 11
                    height: 11
                    visible: windowButton.modelData === "close"

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: 1
                        color: Theme.text
                        rotation: 45
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: 1
                        color: Theme.text
                        rotation: -45
                    }
                }

                HoverHandler { id: hover }

                TapHandler {
                    onTapped: {
                        if (windowButton.modelData === "minimize")
                            root.minimizeRequested();
                        else if (windowButton.modelData === "maximize")
                            root.maximizeRequested();
                        else
                            root.closeRequested();
                    }
                }
            }
        }
    }
}
