import QtQuick
import Vesper.theme

Item {
    id: root

    property var entries: []
    property int currentIndex: 0

    signal selected(int index)

    implicitWidth: 168

    Column {
        anchors.fill: parent
        anchors.margins: Theme.gapSmall
        spacing: 4

        Repeater {
            model: root.entries

            Item {
                required property var modelData
                required property int index

                width: parent.width
                height: 42

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusMedium
                    color: index === root.currentIndex ? Qt.rgba(1, 1, 1, 0.11)
                                                       : (hover.hovered ? Qt.rgba(1, 1, 1, 0.05)
                                                                        : "transparent")
                    border.width: index === root.currentIndex ? 1 : 0
                    border.color: Theme.glassEdgeSoft

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                }

                Rectangle {
                    width: 3
                    height: index === root.currentIndex ? 20 : 0
                    radius: 2
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.accent

                    Behavior on height { NumberAnimation { duration: Theme.durationBase; easing.type: Easing.OutBack } }
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData
                    color: index === root.currentIndex ? Theme.text : Theme.textMuted
                    font.pixelSize: Theme.fontBody
                    font.weight: index === root.currentIndex ? Font.DemiBold : Font.Normal

                    Behavior on color { ColorAnimation { duration: Theme.durationFast } }
                }

                HoverHandler { id: hover }
                TapHandler { onTapped: root.selected(index) }
            }
        }
    }
}
