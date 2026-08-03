import Vesper
import QtQuick

Item {
    id: root

    property real radius: Theme.radiusLarge
    property color tint: Theme.glass
    property color edge: Theme.glassEdge
    property bool elevated: false
    default property alias content: contentHolder.data

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.tint
        border.width: 1
        border.color: root.edge

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius - 1
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, root.elevated ? 0.09 : 0.05) }
                GradientStop { position: 0.45; color: Qt.rgba(1, 1, 1, 0.012) }
                GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.05) }
            }
        }

        Rectangle {
            width: parent.width - root.radius * 1.6
            height: 1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 1
            color: Qt.rgba(1, 1, 1, 0.22)
        }
    }

    Item {
        id: contentHolder
        anchors.fill: parent
    }
}
