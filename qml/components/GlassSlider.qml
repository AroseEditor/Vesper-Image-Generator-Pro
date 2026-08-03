import QtQuick
import QtQuick.Controls.Basic
import Vesper.theme

Item {
    id: root

    property string label: ""
    property real value: 0
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property int decimals: 0
    property string suffix: ""

    signal moved(real value)

    implicitHeight: 46

    Text {
        id: caption
        anchors.left: parent.left
        anchors.top: parent.top
        text: root.label
        color: Theme.textMuted
        font.pixelSize: Theme.fontSmall
    }

    Text {
        anchors.right: parent.right
        anchors.top: parent.top
        text: root.value.toFixed(root.decimals) + root.suffix
        color: Theme.text
        font.pixelSize: Theme.fontSmall
    }

    Slider {
        id: slider
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 22
        from: root.from
        to: root.to
        stepSize: root.stepSize
        value: root.value
        onMoved: root.moved(value)

        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: slider.availableWidth
            height: 4
            radius: 2
            color: Qt.rgba(1, 1, 1, 0.1)

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                radius: 2
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: Theme.accentDeep }
                    GradientStop { position: 1.0; color: Theme.accent }
                }
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 15
            height: 15
            radius: 8
            color: slider.pressed ? Theme.accent : "#efe8f6"
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.25)
            scale: slider.hovered ? 1.12 : 1.0

            Behavior on scale { NumberAnimation { duration: Theme.durationFast } }
            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
        }
    }
}
